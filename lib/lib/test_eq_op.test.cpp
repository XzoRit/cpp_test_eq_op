#include <lib/a.test.hpp>

#include <lib/test_eq_op.hpp>

#include <boost/smart_ptr/make_unique.hpp>

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace std::string_literals;

using boost::make_unique;

using xzr::test::ctor_params;
using xzr::test::test_eq_op;
using xzr::test::impl::create_test_objects;

using xzr_test::A;

namespace
{
BOOST_AUTO_TEST_CASE(test_create_test_objects)
{
    auto a = ctor_params(1, "2"s, 3, 4.0);
    auto b = ctor_params(5, "6"s, 7, 8.0);

    const auto& objs{create_test_objects<A>(a, b)};
    A as[] = {{5, "2"s, 3, 4.0}, {1, "6"s, 3, 4.0}, {1, "2"s, 7, 4.0}, {1, "2"s, 3, 8.0}};
    const std::vector<A> expected{std::make_move_iterator(std::begin(as)), std::make_move_iterator(std::end(as))};

    BOOST_TEST(objs == expected);
}

BOOST_AUTO_TEST_CASE(test_test_eq_op)
{
    {
        BOOST_TEST(test_eq_op<A>(ctor_params(1, "2"s, 3, 4.0), ctor_params(5, "6"s, 7, 8.0)));
    }
    // // this shall not compile
    // // if we have a move-only ctor param it get into its moved-from-state once
    // // we use the parameter to create an actual object to test the ==-operator
    // // of the created object
    // // thus any further objects being created are initialized with a moved-from
    // // parameter rendering this test-utility useless
    // {
    //     auto a = ctor_params(1, "2"s, 3, 4.0, make_unique<int>(11));
    //     auto b = ctor_params(5, "6"s, 7, 8.0, make_unique<int>(12));

    //     BOOST_TEST(test_eq_op<A>(a, b));
    // }
}
} // namespace
