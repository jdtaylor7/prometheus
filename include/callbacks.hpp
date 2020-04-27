#ifndef CALLBACKS_HPP
#define CALLBACKS_HPP

/*
 * FramebufferCallback.
 */
template <typename T>
struct FramebufferCallback;

template <typename Ret, typename... Params>
struct FramebufferCallback<Ret(Params...)>
{
    template <typename... Args>
    static Ret callback(Args... args) { return func(args...); }

    static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> FramebufferCallback<Ret(Params...)>::func;

/*
 * CursorCallback.
 */
template <typename T>
struct CursorCallback;

template <typename Ret, typename... Params>
struct CursorCallback<Ret(Params...)>
{
    template <typename... Args>
    static Ret callback(Args... args) { return func(args...); }

    static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> CursorCallback<Ret(Params...)>::func;

/*
 * ScrollCallback.
 */
template <typename T>
struct ScrollCallback;

template <typename Ret, typename... Params>
struct ScrollCallback<Ret(Params...)>
{
    template <typename... Args>
    static Ret callback(Args... args) { return func(args...); }

    static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> ScrollCallback<Ret(Params...)>::func;

#endif /* CALLBACKS_HPP */
