#include <lib/a.test.hpp>
#include <lib/iterator.hpp>

#include <boost/fusion/container.hpp>

#include <boost/smart_ptr/make_unique.hpp>

#include <boost/test/unit_test.hpp>

#include <string>
#include <tuple>
#include <vector>

using boost::make_unique;

using xzr::iterator::back_emplacer;
using xzr_test::A;

using namespace std::string_literals;

namespace
{
BOOST_AUTO_TEST_CASE(test_back_emplacer)
{
    std::vector<A> as{};
    auto outIter{back_emplacer(as)};

    // single arg
    *outIter = 1;
    BOOST_TEST(as.back() == A{1});
    // single move-only arg
    *outIter = make_unique<int>(2);
    BOOST_TEST(as.back() == A{make_unique<int>(2)});
    // single argument pack
    *outIter = boost::fusion::make_vector(1);
    BOOST_TEST(as.back() == A{1});
    // single argument pack with move-only
    *outIter = std::make_tuple(make_unique<int>(1));
    BOOST_TEST(as.back() == A{make_unique<int>(1)});
    // argument pack
    *outIter = boost::fusion::make_vector(1, "2"s, 3, 4.0);
    BOOST_TEST(as.back() == (A{1, "2"s, 3, 4.0}));
    // argument pack with move-only
    *outIter = std::make_tuple(1, "2"s, 3, 4.0, make_unique<int>(5));
    BOOST_TEST(as.back() == (A{1, "2"s, 3, 4.0, make_unique<int>(5)}));
    // move-only args does not work with fusion-sequences
    // use std::tuple instead
    //
    // // single argument pack with move-only
    // *outIter = boost::fusion::make_vector(make_unique<int>(2));
    // BOOST_TEST(as.back() == A{make_unique<int>(2)});
    // // argument pack with move-only
    // *outIter = boost::fusion::make_vector(1, "2"s, 3, 4.0, make_unique<int>(5));
    // BOOST_TEST(as.back() == (A{1, "2"s, 3, 4.0, make_unique<int>(5)}));
}
} // namespace
