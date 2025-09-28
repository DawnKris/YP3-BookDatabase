#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

#include "book.hpp"

namespace bookdb {

template <typename T>
concept BookIterator = std::input_iterator<T> && std::convertible_to<std::iter_reference_t<T>, const bookdb::Book &>;

template <typename S, typename I>
concept BookSentinel = std::sentinel_for<S, I>;

template <typename T>
concept BookContainerLike = std::ranges::range<T> && BookIterator<std::ranges::iterator_t<T>> &&
                            BookSentinel<std::ranges::sentinel_t<T>, std::ranges::iterator_t<T>> &&
                            std::convertible_to<std::ranges::range_reference_t<T>, const bookdb::Book &>;

template <typename P>
concept BookPredicate = std::predicate<P, const bookdb::Book &>;

template <typename C>
concept BookComparator = std::strict_weak_order<C, const bookdb::Book &, const bookdb::Book &>;

}  // namespace bookdb