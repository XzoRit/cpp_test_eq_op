#include <lib/tuple.hpp>

#include <boost/smart_ptr/make_unique.hpp>

#include <boost/test/unit_test.hpp>

namespace fs = boost::fusion;
namespace mp = boost::mp11;

using namespace std::string_literals;

namespace xzr
{
namespace test
{
namespace utility
{
namespace impl
{
template <std::size_t Width, class Tuple, class OutIter, std::size_t... Is>
auto slide_window_with(Tuple&& a, Tuple&& b, OutIter out, mp::index_sequence<Is...>) -> void
{
    int dummy[] = {
        0,
        (out++ = xzr::tuple::view::replace_with_at<Is, Width>(std::forward<Tuple>(a), std::forward<Tuple>(b)), 0)...};
    static_cast<void>(dummy);
}
} // namespace impl

template <std::size_t Width, class Tuple, class OutIter, class TT = typename std::remove_reference<Tuple>::type>
auto slide_window_with(Tuple&& a, Tuple&& b, OutIter out) -> void
{
    constexpr auto tuple_size{fs::tuple_size<TT>::value};
    impl::slide_window_with<Width>(a, b, out, mp::make_index_sequence<tuple_size>{});
}

template <class Tuple, class TT = typename std::remove_reference<Tuple>::type>
auto create_ctor_params(Tuple&& a, Tuple&& b) -> std::vector<TT>
{
    constexpr auto tuple_size{fs::tuple_size<TT>::value};
    std::vector<TT> replaced{};
    replaced.reserve(tuple_size);
    slide_window_with<1>(a, b, std::back_inserter(replaced));
    return replaced;
}

template <class A, class Tuples>
auto create_with(Tuples&& tuples) -> std::vector<A>
{
    std::vector<A> as{};
    as.reserve(tuples.size());
    xzr::tuple::make_from_tuples<A>(tuples, std::back_inserter(as));
    return as;
}

template <class A, class Tuple>
auto test_eq_op(Tuple&& a, Tuple&& b) -> bool
{
    const auto& origin{xzr::tuple::make_from_tuple<A>(std::forward<Tuple>(a))};
    const auto& c{create_with<A>(create_ctor_params(a, b))};
    for (const auto& it : c)
        if (origin == it && !(origin != it))
            return false;
    return true;
}
} // namespace utility
} // namespace test
} // namespace xzr

namespace
{
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

using xzr::tuple::view::drop_front;
using xzr::tuple::view::replace_with_at;
using xzr::tuple::view::take_front;

using xzr::test::utility::create_ctor_params;
using xzr::test::utility::create_with;
using xzr::test::utility::test_eq_op;

BOOST_AUTO_TEST_CASE(take_with_mp11)
{
    auto a = fs::make_vector(boost::make_unique<int>(0), "1"s, 2);

    BOOST_TEST((take_front<0>(a) == (fs::make_vector())));
    BOOST_TEST((take_front<1>(a) == (fs::as_nview<0>(a))));
    BOOST_TEST((take_front<2>(a) == (fs::as_nview<0, 1>(a))));
    BOOST_TEST((take_front<3>(a) == (fs::as_nview<0, 1, 2>(a))));
}

BOOST_AUTO_TEST_CASE(drop_with_mp11)
{
    auto a = fs::make_vector(boost::make_unique<int>(0), "1"s, 2);

    BOOST_TEST((drop_front<0>(a) == (fs::as_nview<0, 1, 2>(a))));
    BOOST_TEST((drop_front<1>(a) == (fs::as_nview<1, 2>(a))));
    BOOST_TEST((drop_front<2>(a) == (fs::as_nview<2>(a))));
    BOOST_TEST((drop_front<3>(a) == (fs::make_vector())));
}

BOOST_AUTO_TEST_CASE(replace_with_at_n_with_mp11)
{
    auto a = fs::make_vector(boost::make_unique<int>(1), "2", 3, 4.0);
    auto b = fs::make_vector(boost::make_unique<int>(5), "6", 7, 8.0);

    // implicit width = 1
    BOOST_TEST((replace_with_at<0>(a, b) == fs::join(fs::as_nview<0>(b), fs::as_nview<1, 2, 3>(a))));
    BOOST_TEST((replace_with_at<1>(a, b) ==
                fs::join(fs::join(fs::as_nview<0>(a), fs::as_nview<1>(b)), fs::as_nview<2, 3>(a))));
    BOOST_TEST((replace_with_at<2>(a, b) ==
                fs::join(fs::join(fs::as_nview<0, 1>(a), fs::as_nview<2>(b)), fs::as_nview<3>(a))));
    BOOST_TEST((replace_with_at<3>(a, b) == fs::join(fs::as_nview<0, 1, 2>(a), fs::as_nview<3>(b))));

    // explicit width = 1
    BOOST_TEST((replace_with_at<0, 1>(a, b) == replace_with_at<0>(a, b)));
    BOOST_TEST((replace_with_at<1, 1>(a, b) == replace_with_at<1>(a, b)));
    BOOST_TEST((replace_with_at<2, 1>(a, b) == replace_with_at<2>(a, b)));
    BOOST_TEST((replace_with_at<3, 1>(a, b) == replace_with_at<3>(a, b)));

    // explicit width = 2
    BOOST_TEST((replace_with_at<0, 2>(a, b) == fs::join(fs::as_nview<0, 1>(b), fs::as_nview<2, 3>(a))));
    BOOST_TEST((replace_with_at<1, 2>(a, b) ==
                fs::join(fs::as_nview<0>(a), fs::join(fs::as_nview<1, 2>(b), fs::as_nview<3>(a)))));
    BOOST_TEST((replace_with_at<2, 2>(a, b) == fs::join(fs::as_nview<0, 1>(a), fs::as_nview<2, 3>(b))));
}

BOOST_AUTO_TEST_CASE(create_ctor_params_tuple_with_mp11)
{
    auto a = fs::make_vector(1, "2"s, 3, 4.0);
    auto b = fs::make_vector(5, "6"s, 7, 8.0);

    const auto& c{create_ctor_params(a, b)};

    BOOST_TEST(c.size() == 4);
    BOOST_TEST(c[0] == fs::make_vector(5, "2"s, 3, 4.0));
    BOOST_TEST(c[1] == fs::make_vector(1, "6"s, 3, 4.0));
    BOOST_TEST(c[2] == fs::make_vector(1, "2"s, 7, 4.0));
    BOOST_TEST(c[3] == fs::make_vector(1, "2"s, 3, 8.0));
}

BOOST_AUTO_TEST_CASE(create_with_with_mp11)
{
    auto a = fs::make_vector(1, "2"s, 3, 4.0);
    auto b = fs::make_vector(5, "6"s, 7, 8.0);

    const auto& c = create_with<A>(create_ctor_params(a, b));

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
} // namespace
