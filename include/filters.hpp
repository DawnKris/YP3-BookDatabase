#pragma once

#include <algorithm>
#include <functional>
#include <vector>

#include "book.hpp"
#include "book_database.hpp"
#include "concepts.hpp"

namespace bookdb {

inline auto YearBetween(int a, int b) {
    const int lo = std::min(a, b);
    const int hi = std::max(a, b);
    return [lo, hi](const bookdb::Book &bk) noexcept { return bk.year >= lo && bk.year <= hi; };
}

inline auto RatingAbove(double threshold) {
    return [threshold](const bookdb::Book &bk) noexcept { return bk.rating >= threshold; };
}

inline auto GenreIs(bookdb::Genre g) {
    return [g](const bookdb::Book &bk) noexcept { return bk.genre == g; };
}

inline auto GenreIs(std::string_view gstr) { return GenreIs(bookdb::GenreFromString(gstr)); }

template <typename... Preds>
    requires(bookdb::BookPredicate<Preds> && ...)
auto all_of(Preds... ps) {
    if constexpr (sizeof...(Preds) == 0) {
        return [](const bookdb::Book &) noexcept { return true; };
    } else {
        return [=](const bookdb::Book &bk) noexcept { return (ps(bk) && ...); };
    }
}

template <typename... Preds>
    requires(bookdb::BookPredicate<Preds> && ...)
auto any_of(Preds... ps) {
    if constexpr (sizeof...(Preds) == 0) {
        return [](const bookdb::Book &) noexcept { return false; };
    } else {
        return [=](const bookdb::Book &bk) noexcept { return (ps(bk) || ...); };
    }
}

template <bookdb::BookIterator It, bookdb::BookPredicate Pred>
auto filterBooks(It first, It last, Pred pred) -> std::vector<std::reference_wrapper<const bookdb::Book>> {
    std::vector<std::reference_wrapper<const bookdb::Book>> out;
    for (auto it = first; it != last; ++it) {
        const bookdb::Book &bk = *it;
        if (pred(bk))
            out.emplace_back(std::cref(bk));
    }
    return out;
}

template <bookdb::BookContainerLike C, bookdb::BookPredicate Pred>
auto filterBooks(const bookdb::BookDatabase<C> &db, Pred pred)
    -> std::vector<std::reference_wrapper<const bookdb::Book>> {
    return filterBooks(db.begin(), db.end(), pred);
}

}  // namespace bookdb