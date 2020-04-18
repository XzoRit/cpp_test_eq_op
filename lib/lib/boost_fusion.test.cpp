#include <boost/test/unit_test.hpp>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/algorithm/transformation/join.hpp>
#include <boost/fusion/algorithm/transformation/push_back.hpp>
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
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/view/flatten_view.hpp>
#include <boost/fusion/view/iterator_range.hpp>
#include <boost/fusion/view/joint_view.hpp>
#include <boost/fusion/view/single_view.hpp>

#include <boost/smart_ptr/make_unique.hpp>

#include <boost/functional/value_factory.hpp>

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace boost_fusion
{
namespace fs = boost::fusion;

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
            std::cout << *a << ' ';
        else
            std::cout << "nullptr ";
    }
};

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

BOOST_AUTO_TEST_CASE(boost_fusion_idea)
{
    {
        auto a = fs::make_vector(boost::make_unique<int>(1), "2", 3);
        assert(drop_front<0>(a) == a);
        assert(drop_front<1>(a) == fs::make_vector(fs::at_c<1>(a), fs::at_c<2>(a)));
        assert(drop_front<2>(a) == fs::make_vector(fs::at_c<2>(a)));
        assert(drop_front<3>(a) == fs::make_vector());
    }
    {
        auto a = fs::make_vector(3, "2", boost::make_unique<int>(1));

        assert(take_front<0>(a) == fs::make_vector());
        assert(take_front<1>(a) == fs::make_vector(fs::at_c<0>(a)));
        assert(take_front<2>(a) == fs::make_vector(fs::at_c<0>(a), fs::at_c<1>(a)));
        assert(take_front<3>(a) == a);
    }
    {
        S sa{boost::make_unique<int>(1)};
        S sb{boost::make_unique<int>(2)};
        // sa = sb;
        // S sc{sa};
        S sc{std::move(sa)};
        sc = std::move(sb);
    }
    {
        std::vector<S> vec{};
        emplace_back_from_tuple<S>(fs::make_vector(3, "2"), vec);
        assert(vec[0] == (S{3, "2"}));

        emplace_back_from_tuple<S>(fs::make_vector(3, "2", boost::make_unique<int>(1)), vec);
        assert(vec[0] == (S{3, "2"}));
        assert(vec[1] == (S{3, "2", boost::make_unique<int>(1)}));
    }
    {
        auto a = fs::make_vector(boost::make_unique<int>(1), "2", 3);
        auto b = fs::make_vector(boost::make_unique<int>(4), "5", 6);
        auto c = slide<0>(a, b);
        std::cout << "\nslide<0>(a, b) -> ";
        fs::for_each(c, print{});
        auto d = slide<1>(a, b);
        std::cout << "\nslide<1>(a, b) -> ";
        fs::for_each(d, print{});
        auto e = slide<2>(a, b);
        std::cout << "\nslide<2>(a, b) -> ";
        fs::for_each(e, print{});
    }
    {
        const auto c = slide_all<S>(fs::make_vector(1, "2", boost::make_unique<int>(3)),
                                    fs::make_vector(4, "5", boost::make_unique<int>(6)));
        std::cout << "\nslide_all<S>(a, b):\n";
        for (const auto& a : c)
        {
            std::cout << a << '\n';
        }
        std::cout << std::endl;

        // const auto d = slide_all(fs::make_vector(1, "2", boost::make_unique<int>(3)),
        //                          fs::make_vector(4, "5", boost::make_unique<int>(6)));
        // std::cout << "\nslide_all(a, b):\n";
        // // for (const auto& a : d)
        // //   {
        // //     std::cout << a << '\n';
        // //   }
        // std::cout << std::endl;
    }
    BOOST_TEST(false);
}
} // namespace boost_fusion
