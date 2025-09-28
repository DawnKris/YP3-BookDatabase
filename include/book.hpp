#pragma once

#include <array>
#include <format>
#include <stdexcept>
#include <string_view>

namespace bookdb {

enum class Genre { Fiction, NonFiction, SciFi, Biography, Mystery, Unknown };

inline constexpr auto GenreMap = std::to_array<std::pair<std::string_view, bookdb::Genre>>({
    {"Fiction", bookdb::Genre::Fiction},
    {"NonFiction", bookdb::Genre::NonFiction},
    {"SciFi", bookdb::Genre::SciFi},
    {"Biography", bookdb::Genre::Biography},
    {"Mystery", bookdb::Genre::Mystery},
    {"Unknown", bookdb::Genre::Unknown},
});

constexpr Genre GenreFromString(std::string_view s) {
    for (const auto &kv : bookdb::GenreMap) {
        if (kv.first == s)
            return kv.second;
    }
    return bookdb::Genre::Unknown;
}

struct Book {
    // string_view для экономии памяти, чтобы ссылаться на оригинальную строку, хранящуюся в другом контейнере
    std::string_view author;
    std::string title;

    int year;
    Genre genre;
    double rating;
    int read_count;

    constexpr Book(std::string_view author_, std::string_view title_, int year_, bookdb::Genre genre_, double rating_,
                   int read_count_ = 0)
        : author(author_), title(title_), year(year_), genre(genre_), rating(rating_), read_count(read_count_) {}

    constexpr Book(std::string_view author_, std::string_view title_, int year_, std::string_view genre_str,
                   double rating_, int read_count_ = 0)
        : Book(author_, title_, year_, GenreFromString(genre_str), rating_, read_count_) {}
};
}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::Genre, char> {
    template <typename FormatContext>
    auto format(const bookdb::Genre g, FormatContext &fc) const {
        std::string genre_str;

        // clang-format off
        using bookdb::Genre;
        switch (g) {
            case Genre::Fiction:    genre_str = "Fiction"; break;
            case Genre::Mystery:    genre_str = "Mystery"; break;
            case Genre::NonFiction: genre_str = "NonFiction"; break;
            case Genre::SciFi:      genre_str = "SciFi"; break;
            case Genre::Biography:  genre_str = "Biography"; break;
            case Genre::Unknown:    genre_str = "Unknown"; break;
            default:
                throw logic_error{"Unsupported bookdb::Genre"};
            }
        // clang-format on
        return format_to(fc.out(), "{}", genre_str);
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};

template <>
struct formatter<bookdb::Book, char> {
    static constexpr const char *parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const bookdb::Book &b, FormatContext &fc) const {
        return std::format_to(fc.out(),
                              "{{ title: \"{}\", author: \"{}\", year: {}, genre: {}, rating: {:.2f}, reads: {} }}",
                              b.title, b.author, b.year, b.genre, b.rating, b.read_count);
    }
};

}  // namespace std
