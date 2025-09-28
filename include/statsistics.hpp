#pragma once

#include "book_database.hpp"
#include "heterogeneous_lookup.hpp"
#include <cstdint>
#include <flat_map>
#include <format>
#include <functional>
#include <numeric>
#include <random>
#include <vector>

namespace bookdb {

template <typename Comp = bookdb::TransparentStringLess>
struct AuthorHistogramFlat {
    using comparator_type = Comp;
    using map_type = std::flat_map<std::string_view, std::size_t, Comp>;
    map_type data;

    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
};

template <bookdb::BookContainerLike C, typename Comp = bookdb::TransparentStringLess>
auto buildAuthorHistogramFlat(const bookdb::BookDatabase<C> &db, Comp comp = {}) {
    bookdb::AuthorHistogramFlat<Comp> h{typename bookdb::AuthorHistogramFlat<Comp>::map_type{comp}};

    auto &m = h.data;
    for (const bookdb::Book &b : db) {
        auto it = m.lower_bound(b.author);
        if (it == m.end() || m.key_comp()(b.author, it->first)) {
            m.emplace(b.author, 1);
        } else {
            ++it->second;
        }
    }
    return h;
}

template <typename Comp = std::less<bookdb::Genre>>
struct GenreRatingsFlat {
    using comparator_type = Comp;
    using map_type = std::flat_map<bookdb::Genre, double, Comp>;
    map_type data;

    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
};

template <bookdb::BookIterator It>
auto calculateGenreRatings(It first, It last) {
    using Acc = std::flat_map<bookdb::Genre, std::pair<double, std::size_t>>;
    Acc acc;

    for (; first != last; ++first) {
        const bookdb::Book &b = *first;

        auto it = acc.lower_bound(b.genre);
        if (it == acc.end() || acc.key_comp()(b.genre, it->first)) {
            acc.emplace(b.genre, std::make_pair(b.rating, std::size_t{1}));
        } else {
            it->second.first += b.rating;
            ++it->second.second;
        }
    }

    bookdb::GenreRatingsFlat<> result;
    for (const auto &[g, sc] : acc) {
        const double sum = sc.first;
        const std::size_t cnt = sc.second;
        result.data.emplace(g, cnt ? (sum / static_cast<double>(cnt)) : 0.0);
    }
    return result;
}

template <bookdb::BookContainerLike T>
auto calculateGenreRatings(const bookdb::BookDatabase<T> &db) {
    return calculateGenreRatings(db.begin(), db.end());
}

template <bookdb::BookContainerLike T>
double calculateAverageRating(const bookdb::BookDatabase<T> &cont) {
    const std::size_t n = cont.size();
    if (n == 0)
        return 0.0;
    const double sum =
        std::accumulate(cont.begin(), cont.end(), 0.0, [](double s, const bookdb::Book &b) { return s + b.rating; });
    return sum / static_cast<double>(n);
}

template <bookdb::BookContainerLike T>
auto sampleRandomBooks(const bookdb::BookDatabase<T> &cont, std::size_t k, std::uint64_t seed = std::random_device{}())
    -> std::vector<std::reference_wrapper<const bookdb::Book>> {
    std::vector<std::reference_wrapper<const bookdb::Book>> out;
    out.reserve(std::min<std::size_t>(k, cont.size()));
    std::mt19937_64 gen(seed);

    std::size_t i = 0;
    for (const bookdb::Book &b : cont) {
        if (i < k) {
            out.emplace_back(std::cref(b));
        } else {
            std::uniform_int_distribution<std::size_t> dist(0, i);
            const std::size_t j = dist(gen);
            if (j < k)
                out[j] = std::cref(b);
        }
        ++i;
    }
    return out;
}

template <bookdb::BookContainerLike T, typename Comparator = bookdb::comp::GreaterByRating>
    requires std::random_access_iterator<typename bookdb::BookDatabase<T>::iterator>
auto getTopNBy(bookdb::BookDatabase<T> &cont, std::size_t n, Comparator comp = {})
    -> std::vector<std::reference_wrapper<const bookdb::Book>> {
    n = std::min<std::size_t>(n, cont.size());
    std::vector<std::reference_wrapper<const bookdb::Book>> res;
    if (n == 0)
        return res;

    std::partial_sort(cont.begin(), cont.begin() + static_cast<std::ptrdiff_t>(n), cont.end(), comp);

    res.reserve(n);
    for (auto it = cont.begin(); it != cont.begin() + static_cast<std::ptrdiff_t>(n); ++it) {
        res.emplace_back(std::cref(*it));
    }
    return res;
}

}  // namespace bookdb

namespace std {
template <typename Comp>
struct formatter<bookdb::AuthorHistogramFlat<Comp>, char> {
    static constexpr const char *parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const bookdb::AuthorHistogramFlat<Comp> &h, FormatContext &fc) const {
        std::format_to(fc.out(), "{{");
        bool first = true;
        for (const auto &[author, count] : h.data) {
            if (!first)
                std::format_to(fc.out(), ", ");
            first = false;
            std::format_to(fc.out(), "{}: {}", author, count);
        }
        std::format_to(fc.out(), "}}");
        return fc.out();
    }
};

template <typename Comp>
struct formatter<bookdb::GenreRatingsFlat<Comp>, char> {
    static constexpr const char *parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const bookdb::GenreRatingsFlat<Comp> &gr, FormatContext &fc) const {
        std::format_to(fc.out(), "{{");
        bool first = true;
        for (const auto &[g, avg] : gr.data) {
            if (!first)
                std::format_to(fc.out(), ", ");
            first = false;
            std::format_to(fc.out(), "{}: {:.2f}", g, avg);
        }
        std::format_to(fc.out(), "}}");
        return fc.out();
    }
};
}  // namespace std