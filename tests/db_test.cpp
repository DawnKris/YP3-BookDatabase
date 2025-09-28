#include <gtest/gtest.h>

#include "book.hpp"
#include "book_database.hpp"
#include "comparators.hpp"
#include "filters.hpp"
#include "heterogeneous_lookup.hpp"
#include "statsistics.hpp"

using bookdb::Book;
using bookdb::BookDatabase;
using bookdb::Genre;

static BookDatabase<> makeDemoDB() {
    return BookDatabase<>{
        Book{"George Orwell", "1984", 1949, Genre::SciFi, 4.0, 190},
        Book{"George Orwell", "Animal Farm", 1945, Genre::Fiction, 4.4, 143},
        Book{"F. Scott Fitzgerald", "The Great Gatsby", 1925, Genre::Fiction, 4.5, 120},
        Book{"Harper Lee", "To Kill a Mockingbird", 1960, Genre::Fiction, 4.8, 156},
        Book{"Jane Austen", "Pride and Prejudice", 1813, Genre::Fiction, 4.7, 178},
        Book{"J.D. Salinger", "The Catcher in the Rye", 1951, Genre::Fiction, 4.3, 112},
        Book{"Aldous Huxley", "Brave New World", 1932, Genre::SciFi, 4.5, 98},
        Book{"Charlotte Bront", "Jane Eyre", 1847, Genre::Fiction, 4.6, 110},
        Book{"J.R.R. Tolkien", "The Hobbit", 1937, Genre::Fiction, 4.9, 203},
        Book{"William Golding", "Lord of the Flies", 1954, Genre::Fiction, 4.2, 89},
    };
}

TEST(BookDatabase, InitAndSizes) {
    auto db = makeDemoDB();
    EXPECT_EQ(db.size(), 10u);
    EXPECT_FALSE(db.empty());
    EXPECT_EQ(db.GetAuthors().size(), 9u);
}

TEST(BookDatabase, InternAuthorsAndStringViewStability) {
    auto db = makeDemoDB();

    auto it = db.GetAuthors().find("George Orwell");
    ASSERT_NE(it, db.GetAuthors().end());
    const std::string &pooled = *it;

    const auto &books = db.GetBooks();
    std::vector<const void *> addrs;
    for (const auto &b : books)
        if (b.author == "George Orwell")
            addrs.push_back(b.author.data());
    ASSERT_EQ(addrs.size(), 2u);
    EXPECT_EQ(addrs[0], pooled.data());
    EXPECT_EQ(addrs[1], pooled.data());
    EXPECT_EQ(addrs[0], addrs[1]);
}

TEST(Statistics, AuthorHistogramFlat) {
    auto db = makeDemoDB();
    auto hist = bookdb::buildAuthorHistogramFlat(db);

    auto itA = hist.data.find("George Orwell");
    ASSERT_NE(itA, hist.data.end());
    EXPECT_EQ(itA->second, 2u);
}

TEST(Statistics, GenreRatingsAndAverage) {
    auto db = makeDemoDB();

    auto gr = bookdb::calculateGenreRatings(db.begin(), db.end());

    EXPECT_NEAR(gr.data.find(Genre::SciFi)->second, 4.25, 1e-12);
    EXPECT_NEAR(gr.data.find(Genre::Fiction)->second, 4.55, 1e-12);
    EXPECT_NEAR(bookdb::calculateAverageRating(db), 4.49, 1e-12);
}

TEST(Filters, AllOfAnyOfAndFilterBooks) {
    auto db = makeDemoDB();

    auto p = bookdb::all_of(bookdb::YearBetween(1920, 1935), bookdb::GenreIs(Genre::Fiction), bookdb::RatingAbove(4.5));
    auto v = bookdb::filterBooks(db, p);
    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0].get().title, "The Great Gatsby");

    auto p2 = bookdb::any_of(bookdb::GenreIs(Genre::SciFi), bookdb::RatingAbove(4.85));
    auto v2 = bookdb::filterBooks(db, p2);
    ASSERT_EQ(v2.size(), 3u);
    std::vector<std::string> titles;
    for (auto &r : v2)
        titles.push_back(r.get().title);
    EXPECT_NE(std::find(titles.begin(), titles.end(), "1984"), titles.end());
    EXPECT_NE(std::find(titles.begin(), titles.end(), "Brave New World"), titles.end());
    EXPECT_NE(std::find(titles.begin(), titles.end(), "The Hobbit"), titles.end());
}

TEST(Algorithms, GetTopNByModifiesAndSortsTop) {
    auto db = makeDemoDB();

    auto top3 = bookdb::getTopNBy(db, 3);
    ASSERT_EQ(top3.size(), 3u);
    EXPECT_EQ(top3[0].get().title, "The Hobbit");
    EXPECT_EQ(top3[1].get().title, "To Kill a Mockingbird");
    EXPECT_EQ(top3[2].get().title, "Pride and Prejudice");

    const auto &b = db.GetBooks();
    ASSERT_GE(b.size(), 3u);
    EXPECT_EQ(&top3[0].get(), &b[0]);
    EXPECT_EQ(&top3[1].get(), &b[1]);
    EXPECT_EQ(&top3[2].get(), &b[2]);
}

TEST(Algorithms, SampleRandomBooksDeterministicSeed) {
    auto db = makeDemoDB();

    auto s1 = bookdb::sampleRandomBooks(db, 3, 123456u);
    auto s2 = bookdb::sampleRandomBooks(db, 3, 123456u);
    ASSERT_EQ(s1.size(), 3u);
    ASSERT_EQ(s2.size(), 3u);
    for (std::size_t i = 0; i < 3; ++i) {
        EXPECT_EQ(&s1[i].get(), &s2[i].get());
    }
    for (auto &rw : s1) {
        bool found = false;
        for (const auto &bk : db.GetBooks())
            if (&rw.get() == &bk) {
                found = true;
                break;
            }
        EXPECT_TRUE(found);
    }
}

TEST(Boundaries, EmptyDatabase) {
    BookDatabase<> empty;
    EXPECT_TRUE(empty.empty());
    EXPECT_EQ(empty.size(), 0u);

    auto h = bookdb::buildAuthorHistogramFlat(empty);
    EXPECT_TRUE(h.data.empty());

    auto gr = bookdb::calculateGenreRatings(empty.begin(), empty.end());
    EXPECT_TRUE(gr.data.empty());

    EXPECT_EQ(bookdb::calculateAverageRating(empty), 0.0);
    EXPECT_TRUE(bookdb::sampleRandomBooks(empty, 5, 42).empty());
    EXPECT_TRUE(bookdb::getTopNBy(empty, 3).empty());
}

static_assert(bookdb::BookPredicate<decltype(bookdb::GenreIs(Genre::SciFi))>);
static_assert(bookdb::BookPredicate<decltype(bookdb::RatingAbove(4.0))>);
