#include <boost/test/unit_test.hpp>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/algorithm/query/all.hpp>
#include <boost/fusion/algorithm/transformation/join.hpp>
#include <boost/fusion/algorithm/transformation/push_back.hpp>
#include <boost/fusion/algorithm/transformation/zip.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/iterator/advance.hpp>
#include <boost/fusion/iterator/deref.hpp>
#include <boost/fusion/iterator/distance.hpp>
#include <boost/fusion/iterator/equal_to.hpp>
#include <boost/fusion/iterator/next.hpp>
#include <boost/fusion/sequence/comparison.hpp>
#include <boost/fusion/sequence/comparison/equal_to.hpp>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>
#include <boost/fusion/sequence/intrinsic/begin.hpp>
#include <boost/fusion/sequence/intrinsic/end.hpp>
#include <boost/fusion/sequence/intrinsic/front.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/view/flatten_view.hpp>
#include <boost/fusion/view/iterator_range.hpp>
#include <boost/fusion/view/joint_view.hpp>
#include <boost/fusion/view/nview.hpp>
#include <boost/fusion/view/single_view.hpp>
#include <boost/fusion/view/zip_view.hpp>

#include <boost/smart_ptr/make_unique.hpp>

#include <boost/functional/value_factory.hpp>

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace fs = boost::fusion;

struct print
{
    template <class A>
    void operator()(const A& a)
    {
        std::cout << a << ' ';
    }
    template <class A>
    void operator()(const std::unique_ptr<A>& a)
    {
        if (a)
            std::cout << "*uptr=" << *a << ' ';
        else
            std::cout << "nullptr ";
    }
};

namespace boost_fusion
{
struct S
{
    S(std::unique_ptr<int> cc)
        : a{}
        , b{}
        , c{std::move(cc)}
    {
    }
    S(int aa, std::string bb, std::unique_ptr<int> cc)
        : a{aa}
        , b{std::move(bb)}
        , c{std::move(cc)}
    {
    }
    S(int aa, std::string bb)
        : a{aa}
        , b{std::move(bb)}
        , c{}
    {
    }
    int a;
    std::string b;
    std::unique_ptr<int> c;
};

bool operator==(const S& a, const S& b)
{
    if (a.c && b.c)
        return std::tie(a.a, a.b, *a.c) == std::tie(b.a, b.b, *b.c);
    if (!a.c && !b.c)
        return std::tie(a.a, a.b) == std::tie(b.a, b.b);
    return false;
}

bool operator!=(const S& a, const S& b)
{
    return !(a == b);
}

std::ostream& operator<<(std::ostream& o, const S& a)
{
    o << "a=" << a.a << " b=" << a.b << " c=";
    if (a.c)
        o << *a.c;
    else
        o << "nullptr";

    return o;
}

template <std::size_t N, class Tuple>
constexpr auto drop_front(Tuple&& a) -> decltype(
    fs::iterator_range<typename fs::result_of::
                           advance_c<typename fs::result_of::begin<typename std::decay<Tuple>::type>::type, N>::type,
                       typename fs::result_of::end<typename std::decay<Tuple>::type>::type>{
        typename fs::result_of::advance_c<typename fs::result_of::begin<typename std::decay<Tuple>::type>::type,
                                          N>::type{a},
        typename fs::result_of::end<typename std::decay<Tuple>::type>::type{a}})
{
    return fs::iterator_range<
        typename fs::result_of::advance_c<typename fs::result_of::begin<typename std::decay<Tuple>::type>::type,
                                          N>::type,
        typename fs::result_of::end<typename std::decay<Tuple>::type>::type>{
        typename fs::result_of::advance_c<typename fs::result_of::begin<typename std::decay<Tuple>::type>::type,
                                          N>::type{a},
        typename fs::result_of::end<typename std::decay<Tuple>::type>::type{a}};
}

template <std::size_t N, class Tuple>
constexpr auto take_front(Tuple&& a) -> decltype(
    fs::iterator_range<typename fs::result_of::begin<typename std::decay<Tuple>::type>::type,
                       typename fs::result_of::
                           advance_c<typename fs::result_of::begin<typename std::decay<Tuple>::type>::type, N>::type

                       >{
        typename fs::result_of::begin<typename std::decay<Tuple>::type>::type{a},
        typename fs::result_of::advance_c<typename fs::result_of::begin<typename std::decay<Tuple>::type>::type,
                                          N>::type{a}})
{
    return fs::iterator_range<
        typename fs::result_of::begin<typename std::decay<Tuple>::type>::type,
        typename fs::result_of::advance_c<typename fs::result_of::begin<typename std::decay<Tuple>::type>::type,
                                          N>::type

        >{typename fs::result_of::begin<typename std::decay<Tuple>::type>::type{a},
          typename fs::result_of::advance_c<typename fs::result_of::begin<typename std::decay<Tuple>::type>::type,
                                            N>::type{a}};
}

template <class Container>
struct emplace_back_t
{
    template <class... A>
    void operator()(A&&... a)
    {
        container.emplace_back(std::forward<typename std::decay<A>::type>(a)...);
    }
    Container& container{};
};

template <class Type, class Tuple>
void emplace_back_from_tuple(Tuple&& args, std::vector<Type>& out)
{
    emplace_back_t<std::vector<Type>> emp{out};
    fs::invoke(emp, args);
}

template <std::size_t N, class Tuple>
auto slide(Tuple&& a, Tuple&& b) -> decltype(fs::join(
    fs::join(take_front<N>(a),
             fs::single_view<typename fs::result_of::at_c<typename std::decay<Tuple>::type, N>::type>(fs::at_c<N>(b))),
    drop_front<N + 1>(a)))
{
    return fs::join(fs::join(take_front<N>(a),
                             fs::single_view<typename fs::result_of::at_c<typename std::decay<Tuple>::type, N>::type>(
                                 fs::at_c<N>(b))),
                    drop_front<N + 1>(a));
}

template <std::size_t N>
struct slide_all_t;

template <>
struct slide_all_t<0>
{
    template <class Tuple, class Container>
    void operator()(Tuple&&, Tuple&&, Container&) const noexcept
    {
    }
};

template <std::size_t N>
struct slide_all_t
{
    template <class Tuple, class Container>
    void operator()(Tuple&& a, Tuple&& b, Container& container) const
    {
        using decayed = typename std::decay<Tuple>::type;
        const auto& s = slide<N - 1>(std::forward<decayed>(a), std::forward<decayed>(b));
        emplace_back_from_tuple<typename Container::value_type>(s, container);
        slide_all_t<N - 1>{}(a, b, container);
    }
};

template <class Tuple>
decltype(auto) slide_all(Tuple&& a, Tuple&& b)
{
    using decayed = typename std::decay<Tuple>::type;
    using size_t = typename fs::result_of::size<decayed>::type;
    std::vector<decayed> slided{};
    slided.reserve(size_t::value);
    slide_all_t<size_t::value>{}(a, b, slided);
    return slided;
}

template <class T, class Tuple>
decltype(auto) slide_all(Tuple&& a, Tuple&& b)
{
    using decayed = typename std::decay<Tuple>::type;
    using size_t = typename fs::result_of::size<decayed>::type;
    std::vector<T> slided{};
    slided.reserve(size_t::value);
    slide_all_t<size_t::value>{}(a, b, slided);
    return slided;
}

struct eq
{
    template <typename T>
    bool operator()(const T& t) const
    {
        return fs::at_c<0>(t) == fs::at_c<1>(t);
    }
};

template <class FwdSeq1, class FwdSeq2>
bool eq_seq(FwdSeq1&& a, FwdSeq2&& b)
{
    using size_1 = typename fs::result_of::size<std::decay_t<FwdSeq1>>::type;
    using size_2 = typename fs::result_of::size<std::decay_t<FwdSeq2>>::type;
    static_assert(size_1::value == size_2::value, "");

    using zip_seq = fs::vector<std::decay_t<FwdSeq1>&, std::decay_t<FwdSeq2>&>;
    auto&& zipped = fs::zip_view<zip_seq>(zip_seq{a, b});

    return fs::all(zipped, eq{});
}

BOOST_AUTO_TEST_CASE(S_is_move_only)
{
    S sa{boost::make_unique<int>(1)};
    S sb{boost::make_unique<int>(2)};
    // sa = sb;
    // S sc{sa};
    S sc{std::move(sa)};
    sc = std::move(sb);
}

BOOST_AUTO_TEST_CASE(boost_fusion_idea)
{
    {
        auto a = fs::make_vector(boost::make_unique<int>(1), "2", 3);

        BOOST_TEST(eq_seq(drop_front<0>(a), fs::as_nview<0, 1, 2>(a)));
        BOOST_TEST(eq_seq(drop_front<1>(a), fs::as_nview<1, 2>(a)));
        BOOST_TEST(eq_seq(drop_front<2>(a), fs::as_nview<2>(a)));
        BOOST_TEST(eq_seq(drop_front<3>(a), fs::make_vector()));
    }
    {
        auto a = fs::make_vector(boost::make_unique<int>(1), "2", 3);

        BOOST_TEST(eq_seq(take_front<0>(a), fs::make_vector()));
        BOOST_TEST(eq_seq(take_front<1>(a), fs::as_nview<0>(a)));
        BOOST_TEST(eq_seq(take_front<2>(a), fs::as_nview<0, 1>(a)));
        BOOST_TEST(eq_seq(take_front<3>(a), fs::as_nview<0, 1, 2>(a)));
    }
    {
        std::vector<S> vec{};
        emplace_back_from_tuple<S>(fs::make_vector(3, "2"), vec);
        BOOST_TEST(vec[0] == (S{3, "2"}));

        emplace_back_from_tuple<S>(fs::make_vector(3, "2", boost::make_unique<int>(1)), vec);
        BOOST_TEST(vec[0] == (S{3, "2"}));
        BOOST_TEST(vec[1] == (S{3, "2", boost::make_unique<int>(1)}));
    }
    {
        auto a = fs::make_vector(boost::make_unique<int>(1), "2", 3);
        auto b = fs::make_vector(boost::make_unique<int>(4), "5", 6);

        BOOST_TEST(eq_seq(slide<0>(a, b), fs::join(fs::as_nview<0>(b), fs::as_nview<1, 2>(a))));
        BOOST_TEST(
            eq_seq(slide<1>(a, b), fs::join(fs::join(fs::as_nview<0>(a), fs::as_nview<1>(b)), fs::as_nview<2>(a))));
        BOOST_TEST(eq_seq(slide<2>(a, b), fs::join(fs::as_nview<0, 1>(a), fs::as_nview<2>(b))));
    }
    // {
    const auto c = slide_all<S>(fs::make_vector(1, "2", boost::make_unique<int>(3)),
                                fs::make_vector(4, "5", boost::make_unique<int>(6)));
    std::cout << "\nslide_all<S>(a, b):\n";
    for (const auto& a : c)
    {
        std::cout << a << '\n';
    }
    std::cout << std::endl;

    //     // const auto d = slide_all(fs::make_vector(1, "2", boost::make_unique<int>(3)),
    //     //                          fs::make_vector(4, "5", boost::make_unique<int>(6)));
    //     // std::cout << "\nslide_all(a, b):\n";
    //     // // for (const auto& a : d)
    //     // //   {
    //     // //     std::cout << a << '\n';
    //     // //   }
    //     // std::cout << std::endl;
    // }
    // BOOST_TEST(false);
}
} // namespace boost_fusion

#include <boost/mp11.hpp>

namespace mp = boost::mp11;

namespace with_mp11
{
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
} // namespace with_mp11

namespace with_mp11
{
namespace impl
{
template <std::size_t N, class Tuple, std::size_t... Is>
decltype(auto) drop_front(const Tuple& a, mp::index_sequence<Is...>)
{
    return fs::as_nview<Is...>(a);
}

template <std::size_t N, class Tuple, std::size_t... Is>
decltype(auto) take_front(const Tuple& a, mp::index_sequence<Is...>)
{
    return fs::as_nview<Is...>(a);
}
} // namespace impl

template <std::size_t N, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
constexpr decltype(auto) drop_front(Tuple&& a)
{
    return impl::drop_front<N>(a, make_index_range<N, fs::result_of::size<TT>::value>());
}

template <std::size_t N, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
constexpr decltype(auto) take_front(const Tuple& a)
{
    return impl::take_front<N>(a, mp::make_index_sequence<N>());
}

template <std::size_t N, class Tuple>
auto slide(const Tuple& a, const Tuple& b)
    -> decltype(fs::join(fs::join(take_front<N>(a), fs::as_nview<N>(b)), drop_front<N + 1>(a)))
{
    return fs::join(fs::join(take_front<N>(a), fs::as_nview<N>(b)), drop_front<N + 1>(a));
}

BOOST_AUTO_TEST_CASE(take_with_mp11)
{
    auto a = fs::make_vector(boost::make_unique<int>(0), std::string{"1"}, 2);

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
    auto a = fs::make_vector(boost::make_unique<int>(0), std::string{"1"}, 2);

    fs::for_each(drop_front<0>(a), print{});
    BOOST_TEST((drop_front<0>(a) == (fs::as_nview<0, 1, 2>(a))));
    fs::for_each(drop_front<1>(a), print{});
    BOOST_TEST((drop_front<1>(a) == (fs::as_nview<1, 2>(a))));
    fs::for_each(drop_front<2>(a), print{});
    BOOST_TEST((drop_front<2>(a) == (fs::as_nview<2>(a))));
    fs::for_each(drop_front<3>(a), print{});
    BOOST_TEST((drop_front<3>(a) == (fs::make_vector())));
}

BOOST_AUTO_TEST_CASE(slide_with_mp11)
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
} // namespace with_mp11
