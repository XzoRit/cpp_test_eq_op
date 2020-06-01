#pragma once

#include <lib/iterator.hpp>
#include <lib/tuple.hpp>

#include <boost/fusion/container.hpp>
#include <boost/fusion/sequence.hpp>

#include <type_traits>
#include <vector>

namespace xzr
{
namespace test
{
namespace impl
{
template <class A, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
inline auto create_test_objects(Tuple&& a, Tuple&& b) -> std::vector<A>
{
    constexpr auto tuple_size{boost::fusion::tuple_size<TT>::value};
    std::vector<TT> ts{};
    ts.reserve(tuple_size);
    xzr::tuple::slide_window<xzr::tuple::width<1>>(a, b, std::back_inserter(ts));

    std::vector<A> as{};
    as.reserve(tuple_size);
    std::copy(std::begin(ts), std::end(ts), xzr::iterator::back_emplacer(as));

    return as;
}
} // namespace impl

template <class... Args>
inline constexpr auto ctor_params(Args&&... args) -> decltype(boost::fusion::make_vector(std::forward<Args>(args)...))
{
    return boost::fusion::make_vector(std::forward<Args>(args)...);
}

template <class A, class Tuple>
inline auto test_eq_op(Tuple&& a, Tuple&& b) -> bool
{
    const auto& origin{xzr::tuple::make_from_tuple<A>(a)};
    const auto& c{impl::create_test_objects<A>(a, b)};
    for (const auto& it : c)
        if (origin == it && !(origin != it))
            return false;
    return true;
}
} // namespace xzr
} // namespace xzr
