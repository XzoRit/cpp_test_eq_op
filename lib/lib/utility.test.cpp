#include <lib/utility.hpp>

#include <boost/mp11.hpp>

#include <type_traits>

using boost::mp11::index_sequence;
using boost::mp11::integer_sequence;

using xzr::utility::make_index_range;
using xzr::utility::make_integer_range;

namespace
{
static_assert(std::is_same<make_integer_range<int, 0, 0>, integer_sequence<int>>::value, "");
static_assert(std::is_same<make_integer_range<unsigned, 0, 0>, integer_sequence<unsigned>>::value, "");

static_assert(std::is_same<make_integer_range<int, 1, 1>, integer_sequence<int>>::value, "");
static_assert(std::is_same<make_integer_range<unsigned, 5, 5>, integer_sequence<unsigned>>::value, "");

static_assert(std::is_same<make_integer_range<int, 1, 5>, integer_sequence<int, 1, 2, 3, 4>>::value, "");
static_assert(std::is_same<make_integer_range<unsigned, 5, 8>, integer_sequence<unsigned, 5, 6, 7>>::value, "");

static_assert(std::is_same<make_integer_range<int, -3, 3>, integer_sequence<int, -3, -2, -1, 0, 1, 2>>::value, "");
static_assert(std::is_same<make_integer_range<int, 3, -3>, integer_sequence<int, 3, 2, 1, 0, -1, -2>>::value, "");

static_assert(std::is_same<make_index_range<42, 42>, index_sequence<>>::value, "");
static_assert(std::is_same<make_index_range<3, 7>, index_sequence<3, 4, 5, 6>>::value, "");

static_assert(std::is_same<make_index_range<7, 3>, index_sequence<7, 6, 5, 4>>::value, "");
} // namespace
