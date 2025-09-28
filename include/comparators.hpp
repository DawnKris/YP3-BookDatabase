#pragma once

#include "book.hpp"

namespace bookdb::comp {

struct LessByAuthor {
    using is_transparent = void;

    constexpr bool operator()(const bookdb::Book &a, const bookdb::Book &b) const noexcept {
        return (a.author < b.author) || (a.author == b.author && a.title < b.title);
    }
    constexpr bool operator()(std::string_view a, const bookdb::Book &b) const noexcept { return a < b.author; }
    constexpr bool operator()(const bookdb::Book &a, std::string_view b) const noexcept { return a.author < b; }
};

struct LessByPopularity {
    constexpr bool operator()(const bookdb::Book &a, const bookdb::Book &b) const noexcept {
        if (a.read_count != b.read_count)
            return a.read_count < b.read_count;
        if (a.title != b.title)
            return a.title < b.title;
        return a.author < b.author;
    }
};

struct LessByRating {
    constexpr bool operator()(const bookdb::Book &a, const bookdb::Book &b) const noexcept {
        if (a.rating != b.rating)
            return a.rating < b.rating;
        if (a.title != b.title)
            return a.title < b.title;
        return a.author < b.author;
    }
};

struct GreaterByRating {
    constexpr bool operator()(const bookdb::Book &a, const bookdb::Book &b) const noexcept {
        return (a.rating > b.rating) || (a.rating == b.rating && a.title < b.title);
    }
};

}  // namespace bookdb::comp