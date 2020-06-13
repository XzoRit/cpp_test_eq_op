#include <boost/mp11/integer_sequence.hpp>
#include <boost/mp11/tuple.hpp>

#include <boost/test/unit_test.hpp>

#include <iterator>
#include <memory>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <utility>

template <typename Type, std::size_t N, std::size_t Last>
struct tuple_printer
{

    static void print(std::ostream& out, const Type& value)
    {
        out << std::get<N>(value) << ", ";
        tuple_printer<Type, N + 1, Last>::print(out, value);
    }
};

template <typename Type, std::size_t N>
struct tuple_printer<Type, N, N>
{

    static void print(std::ostream& out, const Type& value)
    {
        out << std::get<N>(value);
    }
};

template <typename Type>
struct tuple_printer<Type, 0, std::numeric_limits<std::size_t>::max()>
{

    static void print(std::ostream& out, const Type& value)
    {
    }
};

namespace std
{
template <typename... Types>
std::ostream& operator<<(std::ostream& out, const std::tuple<Types...>& value)
{
    out << "(";
    tuple_printer<std::tuple<Types...>, 0, sizeof...(Types) - 1>::print(out, value);
    out << ")";
    return out;
}
} // namespace std

// namespace std
// {
// template <class T, class... Args>
// std::unique_ptr<T> make_unique(Args&&... args)
// {
//     return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
// }
// } // namespace std

template <class T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <class Tuple>
using tuple_size = std::tuple_size<remove_reference_t<Tuple>>;

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

template <std::size_t N, std::size_t M>
using make_index_range = make_integer_range<std::size_t, N, M>;

namespace Impl
{
template <class Tuple, std::size_t... I>
inline constexpr auto slice(Tuple&& tuple, boost::mp11::index_sequence<I...>)
    -> decltype(std::make_tuple(std::get<I>(std::forward<Tuple>(tuple))...))
{
    return std::make_tuple(std::get<I>(std::forward<Tuple>(tuple))...);
}
} // namespace Impl
template <std::size_t N, std::size_t M, class Tuple>
inline constexpr auto slice(Tuple&& tuple)
    -> decltype(Impl::slice(std::forward<Tuple>(tuple), make_index_range<N, M>{}))
{
    return Impl::slice(std::forward<Tuple>(tuple), make_index_range<N, M>{});
}

template <std::size_t N, class Tuple>
inline constexpr auto take_front(Tuple&& tuple) -> decltype(slice<0, N>(std::forward<Tuple>(tuple)))
{
    return slice<0, N>(std::forward<Tuple>(tuple));
}

template <std::size_t N, class Tuple>
inline constexpr auto drop_front(Tuple&& tuple)
    -> decltype(slice<N, tuple_size<Tuple>::value>(std::forward<Tuple>(tuple)))
{
    return slice<N, tuple_size<Tuple>::value>(std::forward<Tuple>(tuple));
}

template <std::size_t N, class Tuple>
inline constexpr auto replace_at(Tuple&& a, Tuple&& b)
    -> decltype(std::tuple_cat(take_front<N>(a), slice<N, N>(b), drop_front<N>(a)))
{
    return std::tuple_cat(take_front<N>(a), slice<N, N + 1>(b), drop_front<N + 1>(a));
}

namespace impl
{
template <class Tuple, class OutIter, std::size_t... Is>
inline constexpr auto slide(Tuple&& a, Tuple&& b, OutIter out, boost::mp11::index_sequence<Is...>) -> void
{
    int dummy[] = {0, ((*out++ = replace_at<Is>(a, b)), 0)...};
    static_cast<void>(dummy);
}
} // namespace impl

template <class Tuple, class OutIter>
inline constexpr auto slide(Tuple&& a, Tuple&& b, OutIter out) -> void
{
    impl::slide(a, b, out, boost::mp11::make_index_sequence<tuple_size<Tuple>::value>{});
}

namespace impl
{
template <class Container>
struct emplace_back_t
{
    emplace_back_t(Container& c)
        : container(std::addressof(c))
    {
    }

    template <class... Args>
    void operator()(Args&&... args)
    {
        container->emplace_back(std::forward<Args>(args)...);
    }

    Container* container{};
};
} // namespace impl

template <class Container, class Tuple>
inline void emplace_back_from_tuple(Container& container, Tuple&& tuple)
{
    boost::mp11::tuple_apply(impl::emplace_back_t<Container>{container}, std::forward<Tuple>(tuple));
}

template <typename Container>
class back_emplace_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
  public:
    using container_type = Container;

    explicit back_emplace_iterator(Container& c)
        : container{std::addressof(c)}
    {
    }
    // single argument version not needed
    // use back_inserter/inserter instead
    // the push_back/insert functions are overloadad with value_type&&
    // e.g.
    // std::transforms(begin(vec_of_int), end(vec_of_int), back_inserter(vec_of_a), [](int i){ return a{i}; });
    // thus we only care about tuples holding an argument-pack
    template <class Tuple>
    back_emplace_iterator& operator=(Tuple&& tuple)
    {
        emplace_back_from_tuple(*container, std::forward<Tuple>(tuple));
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
    Container* container;
};

template <typename Container>
inline back_emplace_iterator<Container> back_emplacer(Container& c)
{
    return back_emplace_iterator<Container>{c};
}

template <class A, class Tuple, class TT = typename std::remove_reference<Tuple>::type>
inline auto create_test_objects(Tuple&& a, Tuple&& b) -> std::vector<A>
{
    constexpr auto size = tuple_size<Tuple>::value;
    TT ts[size] = {};
    slide(a, b, std::begin(ts));

    std::vector<A> as{};
    as.reserve(size);
    std::copy(std::begin(ts), std::end(ts), back_emplacer(as));

    return as;
}

template <class... Args>
inline constexpr auto ctor_params(Args&&... args) -> decltype(std::make_tuple(std::forward<Args>(args)...))
{
    return std::make_tuple(std::forward<Args>(args)...);
}

template <class A, class Tuple>
inline auto test_eq_op(Tuple&& a, Tuple&& b) -> bool
{
    const auto& origin = boost::mp11::construct_from_tuple<A>(a);
    const auto& c = create_test_objects<A>(a, b);
    for (const auto& it : c)
        if (origin == it && !(origin != it))
            return false;
    return true;
}

BOOST_AUTO_TEST_SUITE(cpp11);

struct A
{
    A(int aa, int bb, int cc, std::unique_ptr<int> dd = nullptr)
        : a{aa}
        , b{bb}
        , c{cc}
    //, d{std::move(dd)}
    {
    }

    int a{};
    int b{};
    int c{};
    // std::unique_ptr<int> d{};
    int* d{nullptr};
};

bool operator==(const A& a, const A& b)
{
    if (a.d && b.d)
        return std::tie(a.a, a.b, a.c, *a.d) == std::tie(b.a, b.b, b.c, *b.d);
    if (!a.d && !b.d)
        return std::tie(a.a, a.b, a.c) == std::tie(b.a, b.b, b.c);
    else
        return false;
}

bool operator!=(const A& a, const A& b)
{
    return !(a == b);
}

BOOST_AUTO_TEST_CASE(test_slice)
{
    const auto a = std::make_tuple(1, 22, 333);
    {
        const auto b = slice<0, 3>(a);
        const auto c = std::make_tuple(1, 22, 333);
        BOOST_TEST(c == b);
    }
    {
        const auto b = slice<1, 3>(a);
        const auto c = std::make_tuple(22, 333);
        BOOST_TEST(c == b);
    }
    {
        const auto b = slice<2, 3>(a);
        const auto c = std::make_tuple(333);
        BOOST_TEST(c == b);
    }
    {
        const auto b = slice<3, 3>(a);
        const auto c = std::make_tuple();
        BOOST_TEST(c == b);
    }
    {
        const auto b = slice<0, 2>(a);
        const auto c = std::make_tuple(1, 22);
        BOOST_TEST(c == b);
    }
    {
        const auto b = slice<0, 1>(a);
        const auto c = std::make_tuple(1);
        BOOST_TEST(c == b);
    }
    {
        const auto b = slice<0, 0>(a);
        const auto c = std::make_tuple();
        BOOST_TEST(c == b);
    }
    {
        const auto b = slice<1, 2>(a);
        const auto c = std::make_tuple(22);
        BOOST_TEST(c == b);
    }
}

BOOST_AUTO_TEST_CASE(test_take_front)
{
    const auto a = std::make_tuple(1, 22, 333);
    {
        const auto b = take_front<3>(a);
        const auto c = std::make_tuple(1, 22, 333);
        BOOST_TEST(c == b);
    }
    {
        const auto b = take_front<2>(a);
        const auto c = std::make_tuple(1, 22);
        BOOST_TEST(c == b);
    }
    {
        const auto b = take_front<1>(a);
        const auto c = std::make_tuple(1);
        BOOST_TEST(c == b);
    }
    {
        const auto b = take_front<0>(a);
        const auto c = std::make_tuple();
        BOOST_TEST(c == b);
    }
}

BOOST_AUTO_TEST_CASE(test_replace_at)
{
    const auto a = std::make_tuple(1, 22, 333);
    const auto b = std::make_tuple(4, 55, 666);
    {
        const auto c = replace_at<0>(a, b);
        const auto d = std::make_tuple(4, 22, 333);
        BOOST_TEST(d == c);
    }
    {
        const auto c = replace_at<1>(a, b);
        const auto d = std::make_tuple(1, 55, 333);
        BOOST_TEST(d == c);
    }
    {
        const auto c = replace_at<2>(a, b);
        const auto d = std::make_tuple(1, 22, 666);
        BOOST_TEST(d == c);
    }
}

BOOST_AUTO_TEST_CASE(test_slide)
{
    using Tuple = std::tuple<int, int, int>;
    using Tuples = std::vector<Tuple>;
    const auto a = Tuple{1, 22, 333};
    const auto b = Tuple{4, 55, 666};
    Tuples c{};
    {
        slide(a, b, std::back_inserter(c));
        Tuples d{Tuple{4, 22, 333}, Tuple{1, 55, 333}, Tuple{1, 22, 666}};
        BOOST_TEST(d == c);
    }
}

BOOST_AUTO_TEST_CASE(test_back_emplacer)
{
    using Tuple = std::tuple<int, int, int>;
    const auto a = Tuple{1, 22, 333};
    const auto b = Tuple{4, 55, 666};
    using As = std::vector<A>;
    As as{};
    {
        slide(a, b, back_emplacer(as));
        A c[] = {A{4, 22, 333}, A{1, 55, 333}, A{1, 22, 666}};
        const As d{std::make_move_iterator(std::begin(c)), std::make_move_iterator(std::end(c))};
        BOOST_TEST(d == as);
    }
}

BOOST_AUTO_TEST_CASE(test_test_eq_op)
{
    {
        BOOST_TEST(test_eq_op<A>(ctor_params(1, 2, 3), ctor_params(4, 5, 6)));
    }
    // // this shall not compile
    // // if we have a move-only ctor param it get into its moved-from-state once
    // // we use the parameter to create an actual object to test the ==-operator
    // // of the created object
    // // thus any further objects being created are initialized with a moved-from
    // // parameter rendering this test-utility useless
    // {
    // auto a = ctor_params(1, 2, 3, std::make_unique<int>(11));
    // auto b = ctor_params(5, 6, 7, std::make_unique<int>(12));

    // BOOST_TEST(test_eq_op<A>(a, b));
    // }
}

BOOST_AUTO_TEST_SUITE_END();
