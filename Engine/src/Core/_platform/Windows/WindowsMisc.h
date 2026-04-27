#pragma once

#include <filesystem>
#include <string>

namespace Engine::Platform
{

void* StackAllocImpl(size_t size);

std::string GetBacktraceImpl();

std::filesystem::path GetExecutablePathImpl();

} // namespace Engine::Platform
