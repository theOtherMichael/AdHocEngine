# frozen_string_literal: true

# This script updates the Xcode projects to build all relevant source code.
# When developing on Windows, run it before committing to ensure added/moved/removed 
# files and folders still build correctly on macOS.
# When adding vendor packages as source code, update the script to include them.

begin
  require 'xcodeproj'
rescue LoadError
  abort <<~MSG
    Error: the 'xcodeproj' gem is not installed.

    Install it by running:
      gem install xcodeproj

    Then re-run this script.
  MSG
end
require 'fileutils'
require 'set'

SOURCE_EXTENSIONS = %w[.c .cpp .cc .cxx .m .mm].freeze
HEADER_EXTENSIONS = %w[.h .hpp .hxx].freeze
CODE_EXTENSIONS = (SOURCE_EXTENSIONS + HEADER_EXTENSIONS).freeze

SKIPPED_GROUP_NAMES = %w[Frameworks Products].freeze

# Per-project allowlist of vendor files to manage. Each entry under a project is
# a vendor "package" with:
#   :root            - path under the project folder (used as the managed prefix
#                      and for grouping)
#
#   File selection (one or more may be combined; results are unioned then filtered):
#   :files           - explicit list of paths relative to :root
#   :glob            - single glob pattern resolved beneath :root
#   :globs           - array of glob patterns resolved beneath :root (union)
#   :exclude_glob    - single glob pattern (or array) of paths to exclude after
#                      collection; applied to :glob / :globs results only, not :files
#
#   :compiler_flags  - optional map of relative-to-:root path to a string passed
#                      as PBXBuildFile COMPILER_FLAGS. The script is the source
#                      of truth: drift is overwritten and removed entries are
#                      cleared on the next run.
VENDOR_PACKAGES = {
  'Engine' => [
    { root: 'vendor/metal-cpp/include', glob: '**/*.hpp' },
  ],
  'Editor' => [
    {
      root: 'vendor/imgui',
      globs: [
        '*.{h,cpp}',
        'backends/imgui_impl_{glfw,metal,osx}.{h,cpp,mm}',
      ],
      compiler_flags: {
        'backends/imgui_impl_glfw.cpp' => '-Wno-documentation',
      },
    },
  ],
}.freeze

PROJECTS_TO_UPDATE = %w[Engine EngineTests Editor EditorTests Launcher].freeze

def header?(path)
  HEADER_EXTENSIONS.include?(File.extname(path.to_s))
end

def source?(path)
  SOURCE_EXTENSIONS.include?(File.extname(path.to_s))
end

def code_file?(path)
  return false if path.nil?

  CODE_EXTENSIONS.include?(File.extname(path))
end

def code_file_ref?(ref)
  code_file?(ref.path)
end

def in_managed_prefix?(path, prefixes)
  return false if path.nil?

  prefixes.any? { |prefix| path.start_with?(prefix) }
end

def glob_code_files(dir)
  Dir.glob("#{dir}/**/*").select { |file| File.file?(file) && code_file?(file) }
end

def collect_vendor_files(project_root, packages)
  files = []
  flags = {}

  packages.each do |package|
    package_root = File.join(project_root, package[:root])

    glob_patterns = Array(package[:globs]) + Array(package[:glob])
    exclude_set = Array(package[:exclude_glob]).flat_map do |pattern|
      Dir.glob(File.join(package_root, pattern))
    end.to_set

    glob_patterns.each do |pattern|
      Dir.glob(File.join(package_root, pattern)).each do |path|
        next unless File.file?(path) && code_file?(path)
        next if exclude_set.include?(path)

        files << path
      end
    end

    Array(package[:files]).each do |relative|
      files << File.join(package_root, relative)
    end

    (package[:compiler_flags] || {}).each do |relative, flag|
      flags[File.join(package_root, relative)] = flag
    end
  end

  [files, flags]
end

# Locate an existing PBXHeadersBuildPhase on `target` without creating one.
# `target.headers_build_phase` auto-instantiates an empty phase as a side
# effect, which we never want.
def existing_headers_phase(target)
  target.build_phases.find { |phase| phase.is_a?(Xcodeproj::Project::Object::PBXHeadersBuildPhase) }
end

def headers_phase_for(project, target)
  existing_headers_phase(target) || begin
    phase = project.new(Xcodeproj::Project::Object::PBXHeadersBuildPhase)
    target.build_phases << phase
    phase
  end
end

def apply_compiler_flags(build_file, flags)
  if flags && !flags.empty?
    build_file.settings ||= {}
    build_file.settings['COMPILER_FLAGS'] = flags
  elsif build_file.settings
    build_file.settings.delete('COMPILER_FLAGS')
    build_file.settings = nil if build_file.settings.empty?
  end
end

def add_file_reference(project, file, compiler_flags)
  puts "    #{file}..."
  destination_group = project.main_group.find_subpath(File.dirname(file), true)

  # Every group in this project is anchored to SOURCE_ROOT (its `path` holds the
  # full project-relative path, not a parent-relative one). Walk parents up to
  # but not including the main group to keep that invariant intact.
  destination_group.source_tree = 'SOURCE_ROOT'
  group = destination_group.parent
  while group && group != project.main_group
    group.source_tree = 'SOURCE_ROOT'
    group = group.parent
  end

  file_reference = destination_group.new_reference(file, 'SOURCE_ROOT')
  is_header = header?(file)

  project.targets.each do |target|
    next unless target.is_a?(Xcodeproj::Project::Object::PBXNativeTarget)

    if is_header
      headers_phase_for(project, target).add_file_reference(file_reference)
    else
      build_file = target.source_build_phase.add_file_reference(file_reference)
      apply_compiler_flags(build_file, compiler_flags)
    end
    puts("      Added #{file} to #{target}")
  end
end

def add_missing_file_references(project, expected_files, compiler_flags)
  puts '  Adding missing file references...'

  existing_paths = project.files.select { |ref| code_file_ref?(ref) }.map(&:path).to_set

  expected_files.each do |file|
    next if existing_paths.include?(file)

    add_file_reference(project, file, compiler_flags[file])
  end
end

def remove_broken_file_references(project, expected_files, managed_prefixes)
  puts '  Removing broken file references...'

  expected_set = expected_files.to_set

  project.files.select { |ref| code_file_ref?(ref) }.each do |ref|
    next unless in_managed_prefix?(ref.path, managed_prefixes)

    if ref.source_tree != 'SOURCE_ROOT'
      puts("    Removing reference to #{ref.path} as it is not project-relative")
      ref.remove_from_project
      next
    end

    next if expected_set.include?(ref.path)

    puts("    Removing reference to now-deleted #{ref.path}")
    ref.remove_from_project
  end
end

def reconcile_compiler_flags(project, expected_flags)
  project.targets.each do |target|
    next unless target.is_a?(Xcodeproj::Project::Object::PBXNativeTarget)

    target.source_build_phase.files.each do |build_file|
      ref = build_file.file_ref
      next unless ref&.path

      desired = expected_flags[ref.path]
      current = build_file.settings && build_file.settings['COMPILER_FLAGS']
      next if current == desired

      apply_compiler_flags(build_file, desired)
      if desired
        puts "    Set COMPILER_FLAGS=#{desired} on #{ref.path} in #{target}"
      else
        puts "    Cleared COMPILER_FLAGS on #{ref.path} in #{target}"
      end
    end
  end
end

def fix_build_phase_membership(project)
  puts '  Fixing build phase membership...'

  project.targets.each do |target|
    next unless target.is_a?(Xcodeproj::Project::Object::PBXNativeTarget)

    misplaced_headers = target.source_build_phase.files.select do |build_file|
      build_file.file_ref && header?(build_file.file_ref.path)
    end

    misplaced_headers.each do |build_file|
      ref = build_file.file_ref
      puts "    Moving #{ref.path} to Headers phase in #{target}"
      target.source_build_phase.files.delete(build_file)
      headers_phase_for(project, target).add_file_reference(ref)
    end

    headers_phase = existing_headers_phase(target)
    next if headers_phase.nil?

    misplaced_sources = headers_phase.files.select do |build_file|
      build_file.file_ref && source?(build_file.file_ref.path)
    end

    misplaced_sources.each do |build_file|
      ref = build_file.file_ref
      puts "    Moving #{ref.path} to Sources phase in #{target}"
      headers_phase.files.delete(build_file)
      target.source_build_phase.add_file_reference(ref)
    end
  end
end

def audit_file_groups(project)
  puts '  Auditing file groups...'

  should_recheck_for_empty_groups = false

  project.main_group.recursive_children_groups.each do |group|
    next if SKIPPED_GROUP_NAMES.include?(group.display_name)

    if group.empty?
      puts "    Removed empty group #{group.hierarchy_path}"
      group.remove_from_project
      should_recheck_for_empty_groups = true
      next
    end

    if group.path != group.hierarchy_path[1..]
      group.path = group.hierarchy_path[1..]
      puts "    Fixed non-matching path for group #{group.path}"
    end

    group_folder_name = File.basename(group.path)
    if group.name != group_folder_name && group_folder_name != group.path
      puts("    Renamed non-matching group name \"#{group.name}\" to \"#{group_folder_name}\"")
      group.name = group_folder_name
    end

    group.sort({ groups_position: :above })
  end

  while should_recheck_for_empty_groups
    should_recheck_for_empty_groups = false
    project.main_group.recursive_children_groups.each do |group|
      next unless group.empty?

      puts "    Removed empty group #{group.hierarchy_path}"
      group.remove_from_project
      should_recheck_for_empty_groups = true
    end
  end
end

def update_project(project_name)
  puts "Updating #{project_name}..."

  project_root = File.join('..', project_name)
  packages = VENDOR_PACKAGES[project_name] || []

  first_party_files = glob_code_files(File.join(project_root, 'src')) +
                      glob_code_files(File.join(project_root, 'include'))
  vendor_files, vendor_flags = collect_vendor_files(project_root, packages)

  prefix_to_strip = "#{project_root}/"
  expected_files = (first_party_files + vendor_files).map { |path| path.delete_prefix(prefix_to_strip) }
  expected_flags = vendor_flags.transform_keys { |path| path.delete_prefix(prefix_to_strip) }

  managed_prefixes = ['src/', 'include/'] + packages.map { |pkg| "#{pkg[:root]}/" }

  project = Xcodeproj::Project.open(File.join(project_root, "#{project_name}.xcodeproj"))

  remove_broken_file_references(project, expected_files, managed_prefixes)
  add_missing_file_references(project, expected_files, expected_flags)
  reconcile_compiler_flags(project, expected_flags)
  fix_build_phase_membership(project)
  audit_file_groups(project)

  project.save
end

Dir.chdir(__dir__)
puts 'Updating Ad Hoc Engine Xcode projects...'

failed = []
PROJECTS_TO_UPDATE.each do |project_name|
  update_project(project_name)
rescue StandardError => e
  failed << project_name
  warn "Error updating #{project_name}: #{e.class}: #{e.message}"
  warn e.backtrace.first(5).join("\n")
end

unless failed.empty?
  warn "Failed to update: #{failed.join(', ')}"
  exit 1
end
