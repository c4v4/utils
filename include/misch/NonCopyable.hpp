#ifndef CAV_NONCOPYABLE_HPP
#define CAV_NONCOPYABLE_HPP

namespace cav {
    /**
     * @brief Non-copyable Mixin.
     * Modified from: https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Non-copyable_Mixin
     * @tparam T
     */
    template <class T>
    class NonCopyable {
    public:
        NonCopyable(const NonCopyable &) = delete;
        NonCopyable(NonCopyable &&) noexcept = default;  // only movable objects
        NonCopyable &operator=(const NonCopyable &) = delete;
        NonCopyable &operator=(NonCopyable &&) noexcept = delete;
        T &operator=(const T &) = delete;
        T &operator=(T &&) noexcept = delete;

    protected:
        NonCopyable() = default;
        ~NonCopyable() = default;  /// Protected non-virtual destructor
    };
}  // namespace cav

#endif
