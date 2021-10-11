#ifndef CAV_NOEXCEPTION_HPP
#define CAV_NOEXCEPTION_HPP

#if __cpp_exceptions
    #define _try try
    #define _catch(X) catch (X)
    #define _throw(E) throw E
#else
    #include <fmt/core.h>

    #define _try if (true)
    #define _catch(X) if (false)

template <typename Exception>
void _throw(Exception e) {
    fmt::print(stderr, "An exception has occurred:\n");
    if constexpr (std::is_fundamental_v<Exception>) {
        fmt::print(stderr, "Exception value: {}\n", e);
    } else {
        fmt::print(stderr, "{}\n", e.what());
    }
    abort();
}
#endif

#endif