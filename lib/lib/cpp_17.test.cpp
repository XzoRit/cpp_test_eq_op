#include <boost/test/unit_test.hpp>

#include <cassert>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>

namespace cpp_17
{

class test_struct
{
  public:
    test_struct(int aa, std::string bb, std::unique_ptr<int> cc)
        : a{aa}
        , b{std::move(bb)}
        , c{std::move(cc)}
    {
    }
    test_struct(int aa, std::string bb)
        : a{aa}
        , b{std::move(bb)}
        , c{}
    {
    }
    int a{};
    std::string b{};
    std::unique_ptr<int> c{};
};

bool operator==(const test_struct& a, const test_struct& b)
{
    if (a.c && b.c)
        return std::tie(a.a, a.b, *a.c) == std::tie(b.a, b.b, *b.c);
    if (!a.c && !b.c)
        return std::tie(a.a, a.b) == std::tie(b.a, b.b);
    return false;
}

bool operator!=(const test_struct& a, const test_struct& b)
{
    return !(a == b);
}

std::ostream& operator<<(std::ostream& str, const test_struct& a)
{
    str << "a=" << a.a << " b=" << a.b << " c=";
    if (a.c)
        str << *a.c;
    else
        str << "nullptr";
    return str;
}

template <class Tuple>
constexpr decltype(auto) get_back(Tuple&& t)
{
    return std::get<std::tuple_size<typename std::remove_reference<Tuple>::type>::value - 1>(std::forward<Tuple>(t));
}

template <typename T, typename TT = typename std::remove_reference<T>::type, size_t... I>
constexpr auto reverse_impl(T&& t, std::index_sequence<I...>)
    -> std::tuple<typename std::tuple_element<sizeof...(I) - 1 - I, TT>::type...>
{
    return std::make_tuple(std::get<sizeof...(I) - 1 - I>(std::forward<T>(t))...);
}

template <typename T, typename TT = typename std::remove_reference<T>::type>
constexpr auto reverse(T&& t)
    -> decltype(reverse_impl(std::forward<T>(t), std::make_index_sequence<std::tuple_size<TT>::value>()))
{
    return reverse_impl(std::forward<T>(t), std::make_index_sequence<std::tuple_size<TT>::value>());
}

template <class Tuple, std::size_t... Is>
constexpr decltype(auto) make_tuple_from(Tuple&& a, std::index_sequence<Is...>)
{
    return std::make_tuple(std::get<Is>(std::forward<Tuple>(a))...);
}

template <std::size_t N, class Tuple>
constexpr decltype(auto) take_front(Tuple&& a)
{
    return make_tuple_from(std::forward<Tuple>(a), std::make_index_sequence<N>{});
}

template <std::size_t N, class Tuple>
constexpr decltype(auto) drop_back(Tuple&& a)
{
    constexpr auto size = std::tuple_size<std::decay_t<Tuple>>::value - N;
    return make_tuple_from(std::forward<Tuple>(a), std::make_index_sequence<size>{});
}

template <std::size_t N, class Tuple>
constexpr decltype(auto) drop_front(Tuple&& t)
{
    auto a = reverse(std::forward<Tuple>(t));
    auto b = drop_back<N>(std::forward<std::decay_t<decltype(a)>>(a));
    auto c = reverse(std::forward<std::decay_t<decltype(b)>>(b));
    return c;
}

template <std::size_t N, class Tuple>
constexpr decltype(auto) sliding_window(Tuple&& a, Tuple&& b)
{
    return std::tuple_cat(take_front<N>(std::forward<Tuple>(a)),
                          std::make_tuple(std::get<N>(std::forward<Tuple>(b))),
                          drop_front<N + 1>(std::forward<Tuple>(a)));
}

template <class T, class Tuple, std::size_t... Is>
constexpr decltype(auto) make_test_vector_impl(Tuple&& a, Tuple&& b, std::index_sequence<Is...>)
{
    std::vector<T> out{};
    int dummy[] = {0,
                   ((void)std::apply(
                        [&]<class... Ts>(Ts && ... ts) { out.emplace_back(std::forward<std::decay_t<Ts>>(ts)...); },
                        sliding_window<Is>(std::forward<std::decay_t<Tuple>>(a), std::forward<std::decay_t<Tuple>>(b))),
                    0)...};
    static_cast<void>(dummy);
    return out;
}

template <class T, class Tuple>
constexpr decltype(auto) make_test_vector(Tuple&& a, Tuple&& b)
{
    return make_test_vector_impl<T>(std::forward<Tuple>(a),
                                    std::forward<Tuple>(b),
                                    std::make_index_sequence<std::tuple_size<Tuple>::value>{});
}

template <class T, class Tuple>
constexpr void check_eq_op(Tuple&& a, Tuple&& b)
{
    const auto& c = make_test_vector<T>(std::forward<Tuple>(a), std::forward<Tuple>(b));
    const auto& e = std::make_from_tuple<T>(std::forward<Tuple>(a));
    std::all_of(cbegin(c), cend(c), [&](const auto& d) { return e != d; });
}

BOOST_AUTO_TEST_SUITE(cpp_17)

BOOST_AUTO_TEST_CASE(get_back_of_tuple)
{
    std::tuple<int, int> a{std::make_tuple(1, 2)};
    BOOST_TEST(get_back(a) == 2);
}

BOOST_AUTO_TEST_CASE(take_front_of_tuple)
{
    {
        const auto a{std::make_tuple(std::make_unique<int>(1))};
    }
    const auto a{std::make_tuple(1, 2, 3)};
    assert(take_front<0>(a) == std::make_tuple());
    assert(take_front<1>(a) == std::make_tuple(1));
    assert(take_front<2>(a) == std::make_tuple(1, 2));
    assert(take_front<3>(a) == std::make_tuple(1, 2, 3));
}

BOOST_AUTO_TEST_CASE(drop_back_of_tuple)
{
    const auto a{std::make_tuple(1, 2, 3)};
    assert(drop_back<0>(a) == std::make_tuple(1, 2, 3));
    assert(drop_back<1>(a) == std::make_tuple(1, 2));
    assert(drop_back<2>(a) == std::make_tuple(1));
    assert(drop_back<3>(a) == std::make_tuple());

    const auto b{std::make_tuple(3, 2, 1)};
    assert(drop_back<0>(b) == std::make_tuple(3, 2, 1));
    assert(drop_back<1>(b) == std::make_tuple(3, 2));
    assert(drop_back<2>(b) == std::make_tuple(3));
    assert(drop_back<3>(b) == std::make_tuple());
}

BOOST_AUTO_TEST_CASE(drop_front_of_tuple)
{
    const auto a{std::make_tuple(1, 2, 3)};
    assert(drop_front<0>(a) == std::make_tuple(1, 2, 3));
    assert(drop_front<1>(a) == std::make_tuple(2, 3));
    assert(drop_front<2>(a) == std::make_tuple(3));
    assert(drop_front<3>(a) == std::make_tuple());
}

BOOST_AUTO_TEST_CASE(sliding_window_through_tuple)
{
    const auto a{std::make_tuple(1, 2, 3)};
    const auto b{std::make_tuple(4, 5, 6)};
    assert(sliding_window<0>(a, b) == std::make_tuple(4, 2, 3));
    assert(sliding_window<1>(a, b) == std::make_tuple(1, 5, 3));
    assert(sliding_window<2>(a, b) == std::make_tuple(1, 2, 6));
}

BOOST_AUTO_TEST_CASE(make_test_vector_from_tuples)
{
    {
        const auto a = make_test_vector<test_struct>(std::make_tuple(1, "2"), std::make_tuple(3, "4"));
        BOOST_TEST(a.size() == 2);
        std::vector<test_struct> expected{};
        expected.emplace_back(3, "2");
        expected.emplace_back(1, "4");
        BOOST_TEST(a == expected);
    }
    {
        const auto a = make_test_vector<test_struct>(std::make_tuple(1, "2", std::make_unique<int>(3)),
                                                     std::make_tuple(4, "5", std::make_unique<int>(6)));

        std::vector<test_struct> expected{};
        expected.emplace_back(4, "2", std::make_unique<int>(3));
        expected.emplace_back(1, "5", std::make_unique<int>(3));
        expected.emplace_back(1, "2", std::make_unique<int>(6));

        for (const auto& b : a)
            std::cout << b << ' ';
        std::cout << std::endl;
        for (const auto& b : expected)
            std::cout << b << ' ';
        std::cout << std::endl;

        BOOST_TEST(a.size() == 3);
        // BOOST_TEST(a == expected);
    }
}

BOOST_AUTO_TEST_CASE(check_eq_op_with_test_struct)
{
    check_eq_op<test_struct>(std::make_tuple(1, std::string{"2"}, std::make_unique<int>(3)),
                             std::make_tuple(4, std::string{"5"}, std::make_unique<int>(6)));
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace cpp_17
