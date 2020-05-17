#pragma once

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
namespace iterator
{
template <typename Container>
class back_emplace_iterator
{
  public:
    explicit back_emplace_iterator(Container& c)
        : container{std::addressof(c)}
    {
    }

    template <typename... Args>
    back_emplace_iterator& operator=(Args&&... args)
    {
        static_assert(std::is_constructible<typename Container::value_type, Args...>::value,
                      "value_type should be constructible from args");

        container->emplace_back(std::forward<Args>(args)...);
        return *this;
    }

    back_emplace_iterator& operator*()
    {
        return *this;
    }

    back_emplace_iterator& operator++()
    {
        return *this;
    }

    back_emplace_iterator operator++(int)
    {
        return *this;
    }

  private:
    Container* container{nullptr};
};

template <typename Container>
inline back_emplace_iterator<Container> back_emplacer(Container& c)
{
    return back_emplace_iterator<Container>{c};
}
} // namespace iterator
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
    -> decltype(boost::fusion::join(boost::fusion::join(xzr::tuple::view::take_front<N>(a),
                                                        boost::fusion::as_nview<(N + Is)...>(b)),
                                    xzr::tuple::view::drop_front<N + sizeof...(Is)>(a)))
{
    return boost::fusion::join(
        boost::fusion::join(xzr::tuple::view::take_front<N>(a), boost::fusion::as_nview<(N + Is)...>(b)),
        xzr::tuple::view::drop_front<N + sizeof...(Is)>(a));
}
} // namespace impl

template <std::size_t N, std::size_t Width, class Tuple>
inline constexpr auto replace_with_at(Tuple&& a, Tuple&& b)
    -> decltype(impl::replace_with_at<N>(a, b, boost::mp11::make_index_sequence<Width>{}))
{
    return impl::replace_with_at<N>(a, b, boost::mp11::make_index_sequence<Width>{});
}

template <std::size_t N, class Tuple>
inline constexpr auto replace_with_at(Tuple&& a, Tuple&& b) -> decltype(replace_with_at<N, 1>(a, b))
{
    return replace_with_at<N, 1>(a, b);
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
auto slide_window_with(Tuple&& a, Tuple&& b, UnFunc unFunc, boost::mp11::index_sequence<Is...>) -> void
{
    int dummy[] = {0, (unFunc(xzr::tuple::view::replace_with_at<Is, Width>(a, b)), 0)...};
    static_cast<void>(dummy);
}
} // namespace impl

template <std::size_t Width, class Tuple, class UnFunc, class TT = typename std::remove_reference<Tuple>::type>
auto slide_window_with(Tuple&& a, Tuple&& b, UnFunc unFunc) -> void
{
    constexpr auto tuple_size{boost::fusion::tuple_size<TT>::value};
    impl::slide_window_with<Width>(a, b, unFunc, boost::mp11::make_index_sequence<tuple_size>{});
}
} // namespace tuple
} // namespace xzr
