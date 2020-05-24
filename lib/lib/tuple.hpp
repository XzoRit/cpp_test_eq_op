#pragma once

#include <lib/utility.hpp>

#include <boost/mp11.hpp>

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/functional.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/fusion/view.hpp>

#include <boost/functional/value_factory.hpp>

#include <cstddef>
#include <memory>
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
    boost::fusion::invoke_function_object(emp, std::forward<Tuple>(args));
}
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
{
template <class A, class Tuple>
inline auto make_from_tuple(Tuple&& tuple)
    -> decltype(boost::fusion::invoke_function_object(boost::value_factory<A>{}, std::forward<Tuple>(tuple)))
{
    boost::value_factory<A> factory{};
    return boost::fusion::invoke_function_object(factory, std::forward<Tuple>(tuple));
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
inline constexpr auto replace_at(Tuple&& a, Tuple&& b, boost::mp11::index_sequence<Is...> idxs)
    -> decltype(boost::fusion::join(boost::fusion::join(xzr::tuple::view::take_front<N>(a),
                                                        boost::fusion::as_nview<(N + Is)...>(b)),
                                    xzr::tuple::view::drop_front<N + sizeof...(Is)>(a)))
{
    return boost::fusion::join(
        boost::fusion::join(xzr::tuple::view::take_front<N>(a), boost::fusion::as_nview<(N + Is)...>(b)),
        xzr::tuple::view::drop_front<N + sizeof...(Is)>(a));
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
} // namespace view
} // namespace tuple
} // namespace xzr

namespace xzr
{
namespace tuple
{
namespace impl
{
template <std::size_t Width, class Tuple, class UnFunc, std::size_t... Is>
inline constexpr auto slide_window_with(Tuple&& a, Tuple&& b, UnFunc unFunc, boost::mp11::index_sequence<Is...>) -> void
{
    int dummy[] = {0, (unFunc(xzr::tuple::view::replace_at<Is, Width>(a, b)), 0)...};
    static_cast<void>(dummy);
}
} // namespace impl

template <std::size_t Width, class Tuple, class UnFunc, class TT = typename std::remove_reference<Tuple>::type>
inline constexpr auto slide_window_with(Tuple&& a, Tuple&& b, UnFunc unFunc) -> void
{
    constexpr auto tuple_size{boost::fusion::tuple_size<TT>::value};
    static_assert(Width <= tuple_size, "width is greater than tuple size");
    impl::slide_window_with<Width>(a, b, unFunc, boost::mp11::make_index_sequence<tuple_size>{});
}
} // namespace tuple
} // namespace xzr
