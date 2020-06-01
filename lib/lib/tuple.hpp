#pragma once

#include <lib/utility.hpp>

#include <boost/functional/value_factory.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/functional.hpp>
#include <boost/fusion/support.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/fusion/view.hpp>
#include <boost/mp11.hpp>
#include <boost/mpl/bool.hpp>

#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace xzr
{
namespace tuple
{
namespace impl
{
template <class Container>
struct emplace_back_t
{
    template <class... Args>
    void operator()(Args&&... args)
    {
        container.emplace_back(std::forward<Args>(args)...);
    }
    Container& container{};
};
} // namespace impl
template <class... Args, class Container>
inline void emplace_back_from_tuple(std::tuple<Args...>&& args, Container& out)
{
    boost::mp11::tuple_apply(impl::emplace_back_t<Container>{out}, std::forward<std::tuple<Args...>>(args));
}

template <class FusionSeq, class Container, class FS = typename std::remove_reference<FusionSeq>::type>
inline typename std::enable_if<boost::fusion::traits::is_sequence<FS>::type::value>::type emplace_back_from_tuple(
    FusionSeq&& seq,
    Container& out)
{
    boost::fusion::invoke_function_object(impl::emplace_back_t<Container>{out}, std::forward<FusionSeq>(seq));
}

template <class Arg, class Container, class AA = typename std::remove_reference<Arg>::type>
inline typename std::enable_if<!boost::fusion::traits::is_sequence<AA>::type::value>::type emplace_back_from_tuple(
    Arg&& arg,
    Container& out)
{
    out.emplace_back(std::forward<Arg>(arg));
}
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
{
namespace impl
{
template <class A, class Tuple>
inline auto make_from_tuple(Tuple&& tuple, boost::mpl::false_)
{
    return boost::mp11::construct_from_tuple<A>(std::forward<Tuple>(tuple));
}

template <class A, class Tuple>
inline auto make_from_tuple(Tuple&& tuple, boost::mpl::true_)
{
    boost::value_factory<A> factory{};
    return boost::fusion::invoke_function_object(factory, std::forward<Tuple>(tuple));
}
} // namespace impl
template <class A,
          class Tuple,
          class TT = typename std::remove_reference<Tuple>::type,
          class IsFusionSeq = typename boost::fusion::traits::is_sequence<TT>::type>
inline auto make_from_tuple(Tuple&& tuple)
{
    return impl::make_from_tuple<A>(std::forward<Tuple>(tuple), IsFusionSeq{});
}
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
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
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
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
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
{
namespace impl
{
template <std::size_t N, class Tuple, std::size_t... Is>
inline constexpr auto replace_at(Tuple&& a, Tuple&& b, boost::mp11::index_sequence<Is...> idxs)
    -> decltype(boost::fusion::join(boost::fusion::join(xzr::tuple::take_front<N>(a),
                                                        boost::fusion::as_nview<(N + Is)...>(b)),
                                    xzr::tuple::drop_front<N + sizeof...(Is)>(a)))
{
    return boost::fusion::join(
        boost::fusion::join(xzr::tuple::take_front<N>(a), boost::fusion::as_nview<(N + Is)...>(b)),
        xzr::tuple::drop_front<N + sizeof...(Is)>(a));
}
} // namespace impl

template <std::size_t N, std::size_t Width, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
inline constexpr auto replace_at(Tuple&& a, Tuple&& b)
    -> decltype(impl::replace_at<N>(a, b, boost::mp11::make_index_sequence<Width>{}))
{
    static_assert((N + Width) <= boost::fusion::tuple_size<TT>::value, "index + width is bigger than size of tuple");
    return impl::replace_at<N>(a, b, boost::mp11::make_index_sequence<Width>{});
}

template <std::size_t N, class Tuple>
inline constexpr auto replace_at(Tuple&& a, Tuple&& b) -> decltype(replace_at<N, 1>(a, b))
{
    return replace_at<N, 1>(a, b);
}
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
{
namespace impl
{
template <std::size_t Width, class Tuple, class OutIter, std::size_t... Is>
inline constexpr auto slide_window_c(Tuple&& a, Tuple&& b, OutIter out, boost::mp11::index_sequence<Is...>) -> void
{
    int dummy[] = {0, ((*out++ = xzr::tuple::replace_at<Is, Width>(a, b)), 0)...};
    static_cast<void>(dummy);
}
} // namespace impl

template <std::size_t Width, class Tuple, class OutIter, class TT = typename std::remove_reference<Tuple>::type>
inline constexpr auto slide_window(Tuple&& a, Tuple&& b, OutIter out) -> void
{
    constexpr auto tuple_size{boost::fusion::tuple_size<TT>::value};
    static_assert(Width <= tuple_size, "width shall not be greater than size of tuple");
    impl::slide_window_c<Width>(a, b, out, boost::mp11::make_index_sequence<tuple_size - Width + 1>{});
}

template <class Tuple, class OutIter, class TT = typename std::remove_reference<Tuple>::type>
inline constexpr auto slide_window(Tuple&& a, Tuple&& b, OutIter out) -> void
{
    slide_window<1>(a, b, out);
}
} // namespace tuple
} // namespace xzr
