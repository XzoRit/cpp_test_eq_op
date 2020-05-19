#include <lib/tuple.hpp>

#include <boost/fusion/container.hpp>
#include <boost/fusion/view.hpp>

#include <boost/smart_ptr/make_unique.hpp>

#include <boost/test/unit_test.hpp>

using boost::fusion::as_nview;
using boost::fusion::join;
using boost::fusion::make_vector;

using boost::make_unique;

using xzr::tuple::view::drop_front;
using xzr::tuple::view::replace_with_at;
using xzr::tuple::view::take_front;

namespace
{
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

BOOST_AUTO_TEST_CASE(test_replace_with_at)
{
    const auto a = make_vector(make_unique<int>(1), "2", 3, 4.0);
    const auto b = make_vector(make_unique<int>(5), "6", 7, 8.0);

    // implicit width = 1
    BOOST_TEST((replace_with_at<0>(a, b) == join(as_nview<0>(b), as_nview<1, 2, 3>(a))));
    BOOST_TEST((replace_with_at<1>(a, b) == join(join(as_nview<0>(a), as_nview<1>(b)), as_nview<2, 3>(a))));
    BOOST_TEST((replace_with_at<2>(a, b) == join(join(as_nview<0, 1>(a), as_nview<2>(b)), as_nview<3>(a))));
    BOOST_TEST((replace_with_at<3>(a, b) == join(as_nview<0, 1, 2>(a), as_nview<3>(b))));

    // explicit width = 1
    BOOST_TEST((replace_with_at<0, 1>(a, b) == replace_with_at<0>(a, b)));
    BOOST_TEST((replace_with_at<1, 1>(a, b) == replace_with_at<1>(a, b)));
    BOOST_TEST((replace_with_at<2, 1>(a, b) == replace_with_at<2>(a, b)));
    BOOST_TEST((replace_with_at<3, 1>(a, b) == replace_with_at<3>(a, b)));

    // explicit width = 2
    BOOST_TEST((replace_with_at<0, 2>(a, b) == join(as_nview<0, 1>(b), as_nview<2, 3>(a))));
    BOOST_TEST((replace_with_at<1, 2>(a, b) == join(as_nview<0>(a), join(as_nview<1, 2>(b), as_nview<3>(a)))));
    BOOST_TEST((replace_with_at<2, 2>(a, b) == join(as_nview<0, 1>(a), as_nview<2, 3>(b))));
}
} // namespace
