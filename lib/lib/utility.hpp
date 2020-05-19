#pragma once

#include <boost/mp11.hpp>

namespace xzr
{
namespace utility
{
namespace impl
{
template <typename T,
          T Begin,
          T Steps,
          bool Increase,
          T Delta = T(1),
          typename = boost::mp11::make_integer_sequence<T, Steps>>
struct generate_range;

template <typename T, T B, T S, T D, T... Ns>
struct generate_range<T, B, S, true, D, boost::mp11::integer_sequence<T, Ns...>>
{
    using type = boost::mp11::integer_sequence<T, B + D * Ns...>;
};

template <typename T, T B, T S, T D, T... Ns>
struct generate_range<T, B, S, false, D, boost::mp11::integer_sequence<T, Ns...>>
{
    using type = boost::mp11::integer_sequence<T, B - D * Ns...>;
};
} // namespace impl

template <typename T, T N, T M>
using make_integer_range = typename impl::generate_range<T, N, (N <= M) ? (M - N) : (N - M), (N <= M)>::type;

// see https://github.com/taocpp/sequences
template <std::size_t N, std::size_t M>
using make_index_range = make_integer_range<std::size_t, N, M>;
} // namespace utility
} // namespace xzr
