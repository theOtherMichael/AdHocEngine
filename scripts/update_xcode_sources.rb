# frozen_string_literal: true

# This script can be used to manually update the /src and /include groups in the Xcode projects.
# Use it habitually before commiting changes that involve adding, removing, or moving files and folders.

require 'xcodeproj'
require 'fileutils'

def audit_file_groups(project)
  puts '  Auditing file groups...'

  should_recheck_for_empty_groups = false

  project.main_group.recursive_children_groups.each do |group|
    next if group == project.main_group['Frameworks'] || group == project.main_group['Products']

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

    puts "    Sorted group #{group.path}"
    group.sort({ groups_position: :above })
  end

  while should_recheck_for_empty_groups
    should_recheck_for_empty_groups = false
    project.main_group.recursive_children_groups.each do |group|
      next unless group.empty?

      puts "    Removed empty group #{group.hierarchy_path}"
      group.remove_from_project
      should_recheck_for_empty_groups = true
      next
    end
  end
end

def add_file_reference(project, file)
  puts "    #{file}..."
  destination_group = project.main_group.find_subpath(File.dirname(file), true)

  destination_group.source_tree = 'SOURCE_ROOT'
  destination_group.parents.each { |group| group.source_tree = 'SOURCE_ROOT' }

  file_reference = destination_group.new_reference(file, 'SOURCE_ROOT')

  project.targets.each do |target|
    target.source_build_phase.add_file_reference(file_reference)
    puts("      Added #{file} to #{target}")
  end
end

def add_missing_file_references(project, project_files)
  puts '  Adding missing file references...'

  project_file_references = project.files.select do |path|
    path.path.start_with?('src/', 'include/') && path.path.end_with?('.c', '.cpp', '.cc', '.cxx', '.h', '.hpp', '.hxx', '.m', '.mm')
  end

  project_files.each do |project_file|
    if project_file_references.none? { |project_file_reference| project_file_reference.path == project_file }
      add_file_reference(project, project_file)
    end
  end
end

def remove_broken_file_references(project, project_files)
  puts '  Removing broken file references...'

  project_file_references = project.files.select do |path|
    path.path.end_with?('.c', '.cpp', '.cc', '.cxx', '.h', '.hpp', '.hxx', '.m', '.mm')
  end

  project_file_references.each do |project_file_reference|
    if project_file_reference.source_tree != 'SOURCE_ROOT'
      puts("    Removing reference to #{project_file_reference.path} as it is not project-relative")
      project_file_reference.remove_from_project
      next
    end

    next if project_files.include?(project_file_reference.path)
    next unless project_file_reference.path.start_with?('src/', 'include/')

    puts("    Removing reference to now-deleted #{project_file_reference.path}")
    project_file_reference.remove_from_project
    next
  end
end

def glob_files_in_dir(dir)
  allowed_extensions = ['.c', '.cpp', '.cc', '.cxx', '.h', '.hpp', '.hxx', '.m', '.mm']
  Dir.glob("#{dir}/**/*").select do |file|
    File.file?(file) && allowed_extensions.include?(File.extname(file))
  end
end

def update_project(project_name)
  puts "Updating #{project_name}..."

  project_folder_relative_path = File.join('..', "#{project_name}/")

  source_files = glob_files_in_dir(File.join(project_folder_relative_path, 'src'))
  header_files = glob_files_in_dir(File.join(project_folder_relative_path, 'include'))
  project_files = source_files + header_files

  project_files_relative = project_files.map do |project_file|
    project_file.delete_prefix(project_folder_relative_path)
  end

  project_relative_path = File.join(project_folder_relative_path, "#{project_name}.xcodeproj")
  project = Xcodeproj::Project.open(project_relative_path)

  remove_broken_file_references(project, project_files_relative)
  add_missing_file_references(project, project_files_relative)
  audit_file_groups(project)

  project.save
end

Dir.chdir(__dir__)
puts 'Updating Ad Hoc Engine Xcode projects...'
update_project('Engine')
update_project('EngineTests')
update_project('Editor')
update_project('EditorTests')
update_project('Launcher')
