#pragma once

#include <boost/mp11.hpp>

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/functional.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/fusion/view.hpp>

#include <boost/functional/value_factory.hpp>

#include <cstddef>

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

namespace xzr
{
namespace tuple
{
namespace impl
{
template <class Container>
struct emplace_back_t
{
    template <class... A>
    void operator()(A&&... a)
    {
        container.emplace_back(std::forward<A>(a)...);
    }
    Container& container{};
};
} // namespace impl

template <class Tuple, class Container>
void emplace_back_from_tuple(Tuple&& args, Container& out)
{
    impl::emplace_back_t<Container> emp{out};
    boost::fusion::invoke(emp, args);
}
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
{
template <class A, class Tuple>
constexpr auto make_from_tuple(Tuple&& tuple) -> A
{
    return boost::fusion::invoke(boost::value_factory<A>{}, tuple);
}

template <class A, class Tuples>
auto make_from_tuples(Tuples&& tuples) -> std::vector<A>
{
    std::vector<A> as{};
    as.reserve(tuples.size());
    for (const auto& tuple : tuples)
        xzr::tuple::emplace_back_from_tuple(tuple, as);
    return as;
}
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
{
namespace view
{
namespace impl
{
template <std::size_t N, class Tuple, std::size_t... Is>
constexpr auto drop_front(Tuple&& a, boost::mp11::index_sequence<Is...>) -> decltype(boost::fusion::as_nview<Is...>(a))
{
    return boost::fusion::as_nview<Is...>(a);
}
} // namespace impl

template <std::size_t N, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
constexpr auto drop_front(Tuple&& a)
    -> decltype(impl::drop_front<N>(a, xzr::utility::make_index_range<N, boost::fusion::tuple_size<TT>::value>()))
{
    return impl::drop_front<N>(a, xzr::utility::make_index_range<N, boost::fusion::tuple_size<TT>::value>());
}
} // namespace view
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
{
namespace view
{
namespace impl
{
template <std::size_t N, class Tuple, std::size_t... Is>
constexpr auto take_front(Tuple&& a, boost::mp11::index_sequence<Is...>) -> decltype(boost::fusion::as_nview<Is...>(a))
{
    return boost::fusion::as_nview<Is...>(a);
}
} // namespace impl

template <std::size_t N, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
constexpr auto take_front(Tuple&& a) -> decltype(impl::take_front<N>(a, boost::mp11::make_index_sequence<N>()))
{
    return impl::take_front<N>(a, boost::mp11::make_index_sequence<N>());
}
} // namespace view
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
{
namespace view
{
template <std::size_t N, class Tuple>
constexpr auto slide(Tuple&& a, Tuple&& b)
    -> decltype(boost::fusion::join(boost::fusion::join(take_front<N>(a), boost::fusion::as_nview<N>(b)),
                                    drop_front<N + 1>(a)))
{
    return boost::fusion::join(boost::fusion::join(take_front<N>(a), boost::fusion::as_nview<N>(b)),
                               drop_front<N + 1>(a));
}
} // namespace view
} // namespace tuple
} // namespace xzr
