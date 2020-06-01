#include <lib/a.test.hpp>

#include <lib/tuple.hpp>

#include <boost/fusion/container.hpp>
#include <boost/fusion/view.hpp>

#include <boost/smart_ptr/make_unique.hpp>

#include <boost/test/unit_test.hpp>

#include <iterator>
#include <string>

using namespace std::string_literals;

using boost::fusion::as_nview;
using boost::fusion::join;
using boost::fusion::make_vector;

using boost::make_unique;

using xzr::tuple::emplace_back_from_tuple;
using xzr::tuple::make_from_tuple;
using xzr::tuple::slide_window;
using xzr::tuple::view::drop_front;
using xzr::tuple::view::replace_at;
using xzr::tuple::view::take_front;

using xzr_test::A;

namespace
{
BOOST_AUTO_TEST_CASE(test_make_from_tuple)
{
    const auto a = xzr::tuple::make_from_tuple<A>(make_vector(1, "2"s, 3, 4.0));
    const auto b = A{1, "2"s, 3, 4.0};
    BOOST_REQUIRE_EQUAL(a, b);
}

BOOST_AUTO_TEST_CASE(test_emplace_back_from_tuple)
{
    std::vector<A> as{};

    // single arg
    emplace_back_from_tuple(1, as);
    BOOST_TEST(as.back() == A{1});
    // single move-only arg
    emplace_back_from_tuple(make_unique<int>(2), as);
    BOOST_TEST(as.back() == A{make_unique<int>(2)});
    // single argument pack
    emplace_back_from_tuple(boost::fusion::make_vector(1), as);
    BOOST_TEST(as.back() == A{1});
    // single argument pack with move-only
    emplace_back_from_tuple(std::make_tuple(make_unique<int>(1)), as);
    BOOST_TEST(as.back() == A{make_unique<int>(1)});
    // argument pack
    emplace_back_from_tuple(boost::fusion::make_vector(1, "2"s, 3, 4.0), as);
    BOOST_TEST(as.back() == (A{1, "2"s, 3, 4.0}));
    // argument pack with move-only
    emplace_back_from_tuple(std::make_tuple(1, "2"s, 3, 4.0, make_unique<int>(5)), as);
    BOOST_TEST(as.back() == (A{1, "2"s, 3, 4.0, make_unique<int>(5)}));
    // move-only args does not work with fusion-sequences
    // use std::tuple instead
    //
    // // single argument pack with move-only
    // emplace_back_from_tuple(boost::fusion::make_vector(make_unique<int>(2)), as);
    // BOOST_TEST(as.back() == A{make_unique<int>(2)});
    // // argument pack with move-only
    // emplace_back_from_tuple(boost::fusion::make_vector(1, "2"s, 3, 4.0, make_unique<int>(5)), as);
    // BOOST_TEST(as.back() == (A{1, "2"s, 3, 4.0, make_unique<int>(5)}));
}

BOOST_AUTO_TEST_CASE(test_take_front)
{
    const auto a = make_vector(make_unique<int>(0), "1", 2);

    BOOST_TEST((take_front<0>(a) == (make_vector())));
    BOOST_TEST((take_front<1>(a) == (as_nview<0>(a))));
    BOOST_TEST((take_front<2>(a) == (as_nview<0, 1>(a))));
    BOOST_TEST((take_front<3>(a) == (as_nview<0, 1, 2>(a))));
}

BOOST_AUTO_TEST_CASE(test_drop)
{
    const auto a = make_vector(make_unique<int>(0), "1", 2);

    BOOST_TEST((drop_front<0>(a) == (as_nview<0, 1, 2>(a))));
    BOOST_TEST((drop_front<1>(a) == (as_nview<1, 2>(a))));
    BOOST_TEST((drop_front<2>(a) == (as_nview<2>(a))));
    BOOST_TEST((drop_front<3>(a) == (make_vector())));
}

BOOST_AUTO_TEST_CASE(test_replace_at)
{
    const auto a = make_vector(1, "2"s, 3, 4.0);
    const auto b = make_vector(5, "6"s, 7, 8.0);

    // implicit width = 1
    BOOST_TEST((replace_at<0>(a, b) == join(as_nview<0>(b), as_nview<1, 2, 3>(a))));
    BOOST_TEST((replace_at<1>(a, b) == join(join(as_nview<0>(a), as_nview<1>(b)), as_nview<2, 3>(a))));
    BOOST_TEST((replace_at<2>(a, b) == join(join(as_nview<0, 1>(a), as_nview<2>(b)), as_nview<3>(a))));
    BOOST_TEST((replace_at<3>(a, b) == join(as_nview<0, 1, 2>(a), as_nview<3>(b))));

    // explicit width = 1
    BOOST_TEST((replace_at<0, 1>(a, b) == replace_at<0>(a, b)));
    BOOST_TEST((replace_at<1, 1>(a, b) == replace_at<1>(a, b)));
    BOOST_TEST((replace_at<2, 1>(a, b) == replace_at<2>(a, b)));
    BOOST_TEST((replace_at<3, 1>(a, b) == replace_at<3>(a, b)));

    // explicit width = 2
    BOOST_TEST((replace_at<0, 2>(a, b) == join(as_nview<0, 1>(b), as_nview<2, 3>(a))));
    BOOST_TEST((replace_at<1, 2>(a, b) == join(as_nview<0>(a), join(as_nview<1, 2>(b), as_nview<3>(a)))));
    BOOST_TEST((replace_at<2, 2>(a, b) == join(as_nview<0, 1>(a), as_nview<2, 3>(b))));
}

BOOST_AUTO_TEST_CASE(test_slide_window)
{
    using four_ints = boost::fusion::vector<int, int, int, int>;
    using many_four_ints = std::vector<four_ints>;

    const four_ints a{make_vector(1, 2, 3, 4)};
    const four_ints b{make_vector(5, 6, 7, 8)};

    {
        many_four_ints exp{four_ints{5, 2, 3, 4}, four_ints{1, 6, 3, 4}, four_ints{1, 2, 7, 4}, four_ints{1, 2, 3, 8}};
        {
            many_four_ints ints(exp.size());

            slide_window(a, b, std::begin(ints));
            BOOST_TEST(ints == exp);
        }
    }
    {
        many_four_ints exp{four_ints{5, 6, 3, 4}, four_ints{1, 6, 7, 4}, four_ints{1, 2, 7, 8}};

        many_four_ints ints(exp.size());
        slide_window<2>(a, b, std::begin(ints));

        BOOST_TEST(ints == exp);
    }
    {
        many_four_ints exp{four_ints{5, 6, 7, 4}, four_ints{1, 6, 7, 8}};

        many_four_ints ints(exp.size());
        slide_window<3>(a, b, std::begin(ints));

        BOOST_TEST(ints == exp);
    }
    {
        many_four_ints exp{four_ints{5, 6, 7, 8}};

        many_four_ints ints(exp.size());
        slide_window<4>(a, b, std::begin(ints));

        BOOST_TEST(ints == exp);
    }
}
} // namespace
