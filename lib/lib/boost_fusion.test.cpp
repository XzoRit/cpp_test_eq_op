#include <boost/test/unit_test.hpp>

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/functional.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/fusion/view.hpp>

#include <boost/smart_ptr/make_unique.hpp>

#include <boost/functional/value_factory.hpp>

#include <boost/mp11.hpp>

#include <boost/functional/value_factory.hpp>

namespace fs = boost::fusion;
namespace mp = boost::mp11;

using namespace std::string_literals;

namespace impl
{
template <typename T, T Begin, T Steps, bool Increase, T Delta = T(1), typename = mp::make_integer_sequence<T, Steps>>
struct generate_range;

template <typename T, T B, T S, T D, T... Ns>
struct generate_range<T, B, S, true, D, mp::integer_sequence<T, Ns...>>
{
    using type = mp::integer_sequence<T, B + D * Ns...>;
};

template <typename T, T B, T S, T D, T... Ns>
struct generate_range<T, B, S, false, D, mp::integer_sequence<T, Ns...>>
{
    using type = mp::integer_sequence<T, B - D * Ns...>;
};
} // namespace impl

template <typename T, T N, T M>
using make_integer_range = typename impl::generate_range<T, N, (N <= M) ? (M - N) : (N - M), (N <= M)>::type;

template <std::size_t N, std::size_t M>
using make_index_range = make_integer_range<std::size_t, N, M>;

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
    fs::invoke(emp, args);
}

struct print
{
    template <class A>
    void operator()(const A& a) const
    {
        std::cout << a << ' ';
    }
    template <class A>
    void operator()(const std::unique_ptr<A>& a) const
    {
        if (a)
            std::cout << "*uptr=" << *a << ' ';
        else
            std::cout << "nullptr ";
    }
};

template <class A>
void print_all(const A& a)
{
    const print& printer{};
    for (const auto& it : a)
        printer(it);
}

namespace with_mp11
{
} // namespace with_mp11

namespace with_mp11
{
} // namespace with_mp11
namespace with_mp11
{
namespace impl
{
template <std::size_t N, class Tuple, std::size_t... Is>
constexpr auto drop_front(Tuple&& a, mp::index_sequence<Is...>) -> decltype(fs::as_nview<Is...>(a))
{
    return fs::as_nview<Is...>(a);
}

template <std::size_t N, class Tuple, std::size_t... Is>
constexpr auto take_front(Tuple&& a, mp::index_sequence<Is...>) -> decltype(fs::as_nview<Is...>(a))
{
    return fs::as_nview<Is...>(a);
}
} // namespace impl

template <std::size_t N, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
constexpr auto drop_front(Tuple&& a)
    -> decltype(impl::drop_front<N>(a, make_index_range<N, fs::tuple_size<TT>::value>()))
{
    return impl::drop_front<N>(a, make_index_range<N, fs::tuple_size<TT>::value>());
}

template <std::size_t N, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
constexpr auto take_front(Tuple&& a) -> decltype(impl::take_front<N>(a, mp::make_index_sequence<N>()))
{
    return impl::take_front<N>(a, mp::make_index_sequence<N>());
}

template <std::size_t N, class Tuple>
constexpr auto slide(Tuple&& a, Tuple&& b)
    -> decltype(fs::join(fs::join(take_front<N>(a), fs::as_nview<N>(b)), drop_front<N + 1>(a)))
{
    return fs::join(fs::join(take_front<N>(a), fs::as_nview<N>(b)), drop_front<N + 1>(a));
}

template <class Tuple, class TT = typename std::remove_reference<Tuple>::type, std::size_t... Is>
auto slide_all_impl(Tuple&& a, Tuple&& b, std::vector<TT>& out, mp::index_sequence<Is...>) -> void
{
    int dummy[] = {0, ((void)out.push_back(slide<Is>(std::forward<Tuple>(a), std::forward<Tuple>(b))), 0)...};
    static_cast<void>(dummy);
}

template <class Tuple, class TT = typename std::remove_reference<Tuple>::type>
auto slide_all(Tuple&& a, Tuple&& b) -> std::vector<TT>
{
    constexpr auto tuple_size{fs::tuple_size<TT>::value};
    std::vector<TT> slided{};
    slided.reserve(tuple_size);
    slide_all_impl(a, b, slided, mp::make_index_sequence<tuple_size>{});
    return slided;
}

template <class A, class Tuple>
constexpr auto make_from_tuple(Tuple&& tuple) -> A
{
    return fs::invoke(boost::value_factory<A>{}, tuple);
}

template <class A, class Tuples>
auto make_from_tuples(Tuples&& tuples) -> std::vector<A>
{
    std::vector<A> as{};
    as.reserve(tuples.size());
    for (const auto& tuple : tuples)
        emplace_back_from_tuple(tuple, as);
    return as;
}

template <class A, class Tuple>
auto test_eq_op(Tuple&& a, Tuple&& b) -> bool
{
    const auto& origin{with_mp11::make_from_tuple<A>(std::forward<Tuple>(a))};
    const auto& c{make_from_tuples<A>(slide_all(a, b))};
    for (const auto& it : c)
        if (origin == it)
            return false;
    return true;
}

struct A
{
    A(int aa, std::string bb, int cc, double dd, std::unique_ptr<int> ee = nullptr)
        : a{aa}
        , b{bb}
        , c{cc}
        , d{dd}
        , e{std::move(ee)}
    {
    }
    int a{};
    std::string b{};
    int c{};
    double d{};
    std::unique_ptr<int> e{};
};
bool operator==(const A& a, const A& b)
{
    if (a.e && b.e)
        return std::tie(a.a, a.b, a.c, a.d, *a.e) == std::tie(b.a, b.b, b.c, b.d, *b.e);
    if (!a.e && !b.e)
        return std::tie(a.a, a.b, a.c, a.d) == std::tie(b.a, b.b, b.c, b.d);
    return false;
}
bool operator!=(const A& a, const A& b)
{
    return !(a == b);
}
std::ostream& operator<<(std::ostream& out, const A& a)
{
    out << "a=" << a.a << " b=" << a.b << " c=" << a.c << " d=" << a.d;
    if (a.e)
        out << " *e=" << *a.e;
    return out;
}

BOOST_AUTO_TEST_CASE(take_with_mp11)
{
    auto a = fs::make_vector(boost::make_unique<int>(0), "1"s, 2);

    fs::for_each(take_front<0>(a), print{});
    BOOST_TEST((take_front<0>(a) == (fs::make_vector())));
    fs::for_each(take_front<1>(a), print{});
    BOOST_TEST((take_front<1>(a) == (fs::as_nview<0>(a))));
    fs::for_each(take_front<2>(a), print{});
    BOOST_TEST((take_front<2>(a) == (fs::as_nview<0, 1>(a))));
    fs::for_each(take_front<3>(a), print{});
    BOOST_TEST((take_front<3>(a) == (fs::as_nview<0, 1, 2>(a))));
}

BOOST_AUTO_TEST_CASE(drop_with_mp11)
{
    auto a = fs::make_vector(boost::make_unique<int>(0), "1"s, 2);

    fs::for_each(drop_front<0>(a), print{});
    BOOST_TEST((drop_front<0>(a) == (fs::as_nview<0, 1, 2>(a))));
    fs::for_each(drop_front<1>(a), print{});
    BOOST_TEST((drop_front<1>(a) == (fs::as_nview<1, 2>(a))));
    fs::for_each(drop_front<2>(a), print{});
    BOOST_TEST((drop_front<2>(a) == (fs::as_nview<2>(a))));
    fs::for_each(drop_front<3>(a), print{});
    BOOST_TEST((drop_front<3>(a) == (fs::make_vector())));
}

BOOST_AUTO_TEST_CASE(slide_n_with_mp11)
{
    auto a = fs::make_vector(boost::make_unique<int>(1), "2", 3, 4.0);
    auto b = fs::make_vector(boost::make_unique<int>(5), "6", 7, 8.0);

    fs::for_each(slide<0>(a, b), print{});
    BOOST_TEST((slide<0>(a, b) == fs::join(fs::as_nview<0>(b), fs::as_nview<1, 2, 3>(a))));
    fs::for_each(slide<1>(a, b), print{});
    BOOST_TEST((slide<1>(a, b) == fs::join(fs::join(fs::as_nview<0>(a), fs::as_nview<1>(b)), fs::as_nview<2, 3>(a))));
    fs::for_each(slide<2>(a, b), print{});
    BOOST_TEST((slide<2>(a, b) == fs::join(fs::join(fs::as_nview<0, 1>(a), fs::as_nview<2>(b)), fs::as_nview<3>(a))));
    fs::for_each(slide<3>(a, b), print{});
    BOOST_TEST((slide<3>(a, b) == fs::join(fs::as_nview<0, 1, 2>(a), fs::as_nview<3>(b))));
}

BOOST_AUTO_TEST_CASE(slide_all_tuple_with_mp11)
{
    auto a = fs::make_vector(1, "2"s, 3, 4.0);
    auto b = fs::make_vector(5, "6"s, 7, 8.0);

    const auto& c{slide_all(a, b)};
    print_all(c);

    BOOST_TEST(c.size() == 4);
    BOOST_TEST(c[0] == fs::make_vector(5, "2"s, 3, 4.0));
    BOOST_TEST(c[1] == fs::make_vector(1, "6"s, 3, 4.0));
    BOOST_TEST(c[2] == fs::make_vector(1, "2"s, 7, 4.0));
    BOOST_TEST(c[3] == fs::make_vector(1, "2"s, 3, 8.0));
}

BOOST_AUTO_TEST_CASE(make_from_tuples_with_mp11)
{
    auto a = fs::make_vector(1, "2"s, 3, 4.0);
    auto b = fs::make_vector(5, "6"s, 7, 8.0);

    const auto& c = make_from_tuples<A>(slide_all(a, b));
    print_all(c);

    BOOST_TEST(c.size() == 4);
    BOOST_TEST((c[0] == A{5, "2"s, 3, 4.0}));
    BOOST_TEST((c[1] == A{1, "6"s, 3, 4.0}));
    BOOST_TEST((c[2] == A{1, "2"s, 7, 4.0}));
    BOOST_TEST((c[3] == A{1, "2"s, 3, 8.0}));
}

BOOST_AUTO_TEST_CASE(test_eq_op_with_mp11)
{
    {
        auto a = fs::make_vector(1, "2"s, 3, 4.0);
        auto b = fs::make_vector(5, "6"s, 7, 8.0);

        BOOST_TEST(test_eq_op<A>(a, b));
    }
    // {
    //     auto a = fs::make_vector(1, "2"s, 3, 4.0, boost::make_unique<int>(11));
    //     auto b = fs::make_vector(5, "6"s, 7, 8.0, boost::make_unique<int>(12));

    //     BOOST_TEST(test_eq_op<A>(a, b));
    // }
}
} // namespace with_mp11
