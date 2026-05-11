#pragma once

#include <Engine/Common/Singleton.h>
#include <Engine/Core/SymbolExportMacros.h>

#if ADHOC_EDITOR

template <class Derived>
Derived& Engine::SingletonBase<Derived>::InternalInstance()
{
    static auto instance = Derived{};
    return instance;
}

#define ADHOC_IMPLEMENT_SINGLETON(ClassName) template class ENGINE_API Engine::SingletonBase<ClassName>

#else

#define ADHOC_IMPLEMENT_SINGLETON(ClassName)

#endif
