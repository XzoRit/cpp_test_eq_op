#include <lib/lib.hpp>

#include <boost/test/unit_test.hpp>

namespace
{
using xzr::lib::add;

BOOST_AUTO_TEST_SUITE(lib_tests)

BOOST_AUTO_TEST_CASE(lib_add)
{
    BOOST_TEST(add(1, 3) == 4);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace
