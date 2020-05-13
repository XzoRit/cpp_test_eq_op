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
inline void emplace_back_from_tuple(Tuple&& args, Container& out)
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
inline auto make_from_tuple(Tuple&& tuple) -> A
{
    return boost::fusion::invoke(boost::value_factory<A>{}, tuple);
}

template <class A, class InIter, class OutIter>
inline auto make_from_tuples(InIter b, InIter e, OutIter out) -> void
{
    for (; b != e; ++b)
        out++ = xzr::tuple::make_from_tuple<A>(*b);
}

template <class A, class Tuples, class OutIter>
inline constexpr auto make_from_tuples(Tuples&& tuples, OutIter out) -> void
{
    xzr::tuple::make_from_tuples<A>(std::begin(tuples), std::end(tuples), out);
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
inline constexpr auto drop_front(Tuple&& a, boost::mp11::index_sequence<Is...>)
    -> decltype(boost::fusion::as_nview<Is...>(a))
{
    return boost::fusion::as_nview<Is...>(a);
}
} // namespace impl

template <std::size_t N, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
inline constexpr auto drop_front(Tuple&& a)
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
inline constexpr auto take_front(Tuple&& a, boost::mp11::index_sequence<Is...>)
    -> decltype(boost::fusion::as_nview<Is...>(a))
{
    return boost::fusion::as_nview<Is...>(a);
}
} // namespace impl

template <std::size_t N, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
inline constexpr auto take_front(Tuple&& a) -> decltype(impl::take_front<N>(a, boost::mp11::make_index_sequence<N>()))
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
namespace impl
{
template <std::size_t N, class Tuple, std::size_t... Is>
inline constexpr auto replace_with_at(Tuple&& a, Tuple&& b, boost::mp11::index_sequence<Is...> idxs)
{
    return boost::fusion::join(
        boost::fusion::join(xzr::tuple::view::take_front<N>(a), boost::fusion::as_nview<(N + Is)...>(b)),
        xzr::tuple::view::drop_front<N + sizeof...(Is)>(a));
}
} // namespace impl

template <std::size_t N, std::size_t Width, class Tuple>
constexpr auto replace_with_at(Tuple&& a, Tuple&& b);

template <std::size_t N, class Tuple>
constexpr auto replace_with_at(Tuple&& a, Tuple&& b);

template <std::size_t N, std::size_t Width, class Tuple>
inline constexpr auto replace_with_at(Tuple&& a, Tuple&& b)
{
    return impl::replace_with_at<N>(a, b, boost::mp11::make_index_sequence<Width>{});
}

template <std::size_t N, class Tuple>
inline constexpr auto replace_with_at(Tuple&& a, Tuple&& b)
{
    return replace_with_at<N, 1>(a, b);
}
} // namespace view
} // namespace tuple
} // namespace xzr
