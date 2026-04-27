#pragma once

namespace Engine
{

template <class Derived>
class SingletonBase
{
public:
    SingletonBase(const SingletonBase&)            = delete;
    SingletonBase& operator=(const SingletonBase&) = delete;
    SingletonBase(SingletonBase&&)                 = delete;
    SingletonBase& operator=(SingletonBase&&)      = delete;

protected:
    static Derived& InternalInstance()
    {
        static auto instance = Derived{};
        return instance;
    }

    SingletonBase()  = default;
    ~SingletonBase() = default;
};

template <class Derived>
class ImmutableSingleton : public SingletonBase<Derived>
{
public:
    static const Derived& Instance() { return SingletonBase<Derived>::InternalInstance(); }

#if ADHOC_INTERNAL
    static Derived& MutableInstance() { return SingletonBase<Derived>::InternalInstance(); }
#endif
};

template <class Derived>
class MutableSingleton : public SingletonBase<Derived>
{
public:
    static Derived& Instance() { return SingletonBase<Derived>::InternalInstance(); }
};

} // namespace Engine
