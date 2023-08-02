/* Copyright (C) 2023 Trevor Last
 * See LICENSE file for copyright and license details.
 */

#ifndef UTIL_SIGNAL_HPP
#define UTIL_SIGNAL_HPP

#include <functional>
#include <type_traits>
#include <vector>


template<class Callable>
class Signal
{
    using Callback = std::function<Callable>;
    std::vector<Callback> funcs{};

public:
    void connect(Callback cb)
    {
        funcs.push_back(cb);
    }

    template<typename... Args>
    auto emit(Args... args)
    {
        typename std::invoke_result<Callable, Args...>::type v{};
        for (auto func : funcs)
            v = func(args...);
        return v;
    }
};

template<typename... Args>
class Signal<void(Args...)>
{
    using Callable = void(Args...);
    using Callback = std::function<Callable>;
    std::vector<Callback> funcs{};

public:
    void connect(Callback cb)
    {
        funcs.push_back(cb);
    }

    void emit(Args... args)
    {
        for (auto func : funcs)
            func(args...);
    }
};


#endif
