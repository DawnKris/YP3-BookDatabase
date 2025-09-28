#pragma once

#include <print>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "book.hpp"
#include "concepts.hpp"
#include "heterogeneous_lookup.hpp"

namespace bookdb {

template <bookdb::BookContainerLike BookContainer = std::vector<bookdb::Book>>
class BookDatabase {
public:
    using value_type = bookdb::Book;
    using container_type = BookContainer;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = typename BookContainer::iterator;
    using const_iterator = typename BookContainer::const_iterator;

    using AuthorContainer = std::set<std::string, bookdb::TransparentStringLess>;

    BookDatabase() = default;

    BookDatabase(std::initializer_list<bookdb::Book> init) {
        for (const auto &b : init) {
            PushBack(b);
        }
    }

    void Clear() {
        books_.clear();
        authors_.clear();
    }

    size_type size() const noexcept { return books_.size(); }
    bool empty() const noexcept { return books_.empty(); }

    iterator begin() noexcept { return books_.begin(); }
    const_iterator begin() const noexcept { return books_.begin(); }
    const_iterator cbegin() const noexcept { return books_.cbegin(); }

    iterator end() noexcept { return books_.end(); }
    const_iterator end() const noexcept { return books_.end(); }
    const_iterator cend() const noexcept { return books_.cend(); }

    void PushBack(const bookdb::Book &b) {
        bookdb::Book copy = b;
        copy.author = InternAuthor(copy.author);
        books_.push_back(std::move(copy));
    }

    void PushBack(bookdb::Book &&b) {
        b.author = InternAuthor(b.author);
        books_.push_back(std::move(b));
    }

    template <class... Args>
    bookdb::Book &EmplaceBack(Args &&...args) {
        books_.emplace_back(std::forward<Args>(args)...);
        bookdb::Book &back = books_.back();
        back.author = InternAuthor(back.author);
        return back;
    }

    const BookContainer &GetBooks() const noexcept { return books_; }
    const AuthorContainer &GetAuthors() const noexcept { return authors_; }

private:
    std::string_view InternAuthor(std::string_view name) {
        auto [it, inserted] = authors_.emplace(name);
        const std::string &s = *it;
        return std::string_view{s};
    }

    BookContainer books_;
    AuthorContainer authors_;
};

}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::BookDatabase<std::vector<bookdb::Book>>, char> {
    template <typename FormatContext>
    auto format(const bookdb::BookDatabase<std::vector<bookdb::Book>> &db, FormatContext &fc) const {

        format_to(fc.out(), "BookDatabase (size = {}): ", db.size());

        format_to(fc.out(), "Books:\n");
        for (const auto &book : db.GetBooks()) {
            format_to(fc.out(), "- {}\n", book);
        }

        format_to(fc.out(), "Authors:\n");
        for (const auto &author : db.GetAuthors()) {
            format_to(fc.out(), "- {}\n", author);
        }
        return fc.out();
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};
}  // namespace std
