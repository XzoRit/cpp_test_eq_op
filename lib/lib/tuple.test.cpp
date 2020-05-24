#include <lib/a.test.hpp>

#include <lib/tuple.hpp>

#include <boost/fusion/container.hpp>
#include <boost/fusion/view.hpp>

#include <boost/smart_ptr/make_unique.hpp>

#include <boost/test/unit_test.hpp>

#include <string>

using namespace std::string_literals;

using boost::fusion::as_nview;
using boost::fusion::join;
using boost::fusion::make_vector;

using boost::make_unique;

using xzr::tuple::emplace_back_from_tuple;
using xzr::tuple::make_from_tuple;
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
    const std::vector<boost::fusion::vector<int, std::string, int, double>> params{
        boost::fusion::make_vector(5, "2"s, 3, 4.0),
        boost::fusion::make_vector(1, "6"s, 3, 4.0),
        boost::fusion::make_vector(1, "2"s, 7, 4.0),
        boost::fusion::make_vector(1, "2"s, 3, 8.)};
    const std::vector<A> expected{[]() {
        A as[] = {{5, "2"s, 3, 4.0}, {1, "6"s, 3, 4.0}, {1, "2"s, 7, 4.0}, {1, "2"s, 3, 8.0}};
        return std::vector<A>{std::make_move_iterator(std::begin(as)), std::make_move_iterator(std::end(as))};
    }()};

    std::vector<A> actual{};
    for (const auto& p : params)
        xzr::tuple::emplace_back_from_tuple(p, actual);

    BOOST_TEST(expected == actual);
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
} // namespace
