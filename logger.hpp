#pragma once

#include <string>
#include <format>
#include <iostream>

#include "config.hpp"

using std::string;
using std::format;

class Logger
{
    void print(std::ostream& os, string&& info_type, string&& message) const
    {
        os << std::format("{}[{}]: {}\n", info_type, module_name, message);
        os.flush();
    }

    template <typename TrueType, typename FalseType, bool>
    struct conditional_type;

    template <typename TrueType, typename FalseType>
    struct conditional_type<TrueType, FalseType, true>
    {
        using type = TrueType;
    };

    template <typename TrueType, typename FalseType>
    struct conditional_type<TrueType, FalseType, false>
    {
        using type = FalseType;
    };

    template <typename T>
    static constexpr bool is_wchar_str =
        std::is_same_v<std::remove_cvref_t<T>, wchar_t*>
            || std::is_same_v<std::remove_cvref_t<T>, const wchar_t*>;

    template <typename T>
    using format_arg_t = typename conditional_type<string, T, is_wchar_str<T>>::type;

    template <typename T>
    static constexpr decltype(auto) forward_format_arg(T t)
    {
        if constexpr(is_wchar_str<T>)
        {
            string tmp;
            char buf[sizeof(wchar_t) + 1]{};
            for (int i = 0; auto wc = t[i]; i++)
            {
                wctomb(buf, wc);
                tmp.append(buf);
            }
            return tmp;
        }
        else
        {
            return t;
        }
    }
public:
    inline static const char* locale = setlocale(LC_ALL, "");
    inline static const bool stdio_sync = std::ios::sync_with_stdio(false);
    const bool trace_mode;
    const string module_name;

    Logger(const string& set_name, bool set_trace_mode = config::trace_mode):
        module_name(set_name),
        trace_mode(set_trace_mode)
    {}

    template <typename ... Args_t>
    void error(std::format_string<format_arg_t<Args_t>...> fmt_str, Args_t && ... args) const
    {
        return print(std::cerr, "\033[1m\033[31m[ERROR]\033[0m", format(fmt_str, forward_format_arg<Args_t>(args)...));
    }

    template <typename ... Args_t>
    void warn(std::format_string<format_arg_t<Args_t>...> fmt_str, Args_t && ... args) const
    {
        return print(std::cout, "\033[33m[WARN]\033[0m", format(fmt_str, forward_format_arg<Args_t>(args)...));
    }

    template <typename ... Args_t>
    void info(std::format_string<format_arg_t<Args_t>...> fmt_str, Args_t && ... args) const
    {
        return print(std::cout, "[INFO]", format(fmt_str, forward_format_arg<Args_t>(args)...));
    }

    template <typename ... Args_t>
    void trace(std::format_string<format_arg_t<Args_t>...> fmt_str, Args_t && ... args) const
    {
        if (trace_mode)
        {
            return print(std::cout, "\033[32m[TRACE]\033[0m", format(fmt_str, forward_format_arg<Args_t>(args)...));
        }
    }
};