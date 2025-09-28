#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

namespace bookdb {

struct TransparentStringLess {
    using is_transparent = void;

    template <class L, class R>
        requires(std::is_convertible_v<L, std::string_view> && std::is_convertible_v<R, std::string_view>)
    constexpr bool operator()(const L &lhs, const R &rhs) const noexcept {
        return std::string_view(lhs) < std::string_view(rhs);
    }
};

struct TransparentStringEqual {
    using is_transparent = void;

    template <class L, class R>
        requires(std::is_convertible_v<L, std::string_view> && std::is_convertible_v<R, std::string_view>)
    constexpr bool operator()(const L &lhs, const R &rhs) const noexcept {
        return std::string_view(lhs) == std::string_view(rhs);
    }
};

struct TransparentStringHash {
    using is_transparent = void;

    template <class T>
        requires std::is_convertible_v<T, std::string_view>
    std::size_t operator()(const T &v) const noexcept {
        return std::hash<std::string_view>{}(std::string_view(v));
    }
};

}  // namespace bookdb
