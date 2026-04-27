#include <Engine/Core/DynamicLibrary.h>

#include <Engine/Core/PlatformAbstraction.h>
#include PLATFORM_HEADER(DynamicLibrary.h)

#include <memory>

namespace Engine
{

struct DynamicLibrary::Implementation
{
    std::unique_ptr<Platform::DynamicLibraryImplementation> platformImplementation;

    Implementation() : platformImplementation(std::make_unique<Platform::DynamicLibraryImplementation>()) {}
};

// TODO: Assert implementation signatures match generic

DynamicLibrary::DynamicLibrary() : implementation(std::make_unique<Implementation>())
{}

DynamicLibrary::~DynamicLibrary() = default;

DynamicLibrary::DynamicLibrary(const std::filesystem::path& path) : implementation(std::make_unique<Implementation>())
{
    Load(path);
}

bool DynamicLibrary::IsValid() const
{
    return implementation->platformImplementation->IsValidImpl();
}

void DynamicLibrary::Load(const std::filesystem::path& path)
{
    implementation->platformImplementation->Load(path);
}

void DynamicLibrary::Unload()
{
    implementation->platformImplementation->Unload();
}

void* DynamicLibrary::GetRawFunctionPtr(const std::string& functionName) const
{
    return implementation->platformImplementation->GetRawFunctionPtr(functionName);
}

} // namespace Engine
