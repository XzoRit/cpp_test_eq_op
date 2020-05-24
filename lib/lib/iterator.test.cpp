#include <lib/iterator.hpp>

#include <boost/smart_ptr/make_unique.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <deque>
#include <iterator>
#include <list>
#include <memory>
#include <vector>

using boost::make_unique;

using xzr::iterator::back_emplacer;

namespace
{
class A
{
  public:
    explicit A(std::unique_ptr<int> aa)
        : a{std::move(aa)}
    {
    }

    friend bool operator==(const A& a, const A& b)
    {
        if (a.a && b.a)
            return (*a.a) == (*b.a);
        if (!a.a && !b.a)
            return true;
        return false;
    }

  private:
    std::unique_ptr<int> a{};
};

template <class A, class Params>
A create_from_ctor_param(const Params ps)
{
    A as{};
    std::transform(std::begin(ps), std::end(ps), back_emplacer(as), [](int a) { return make_unique<int>(a); });
    return as;
}

BOOST_AUTO_TEST_CASE(test_back_emplacer)
{
    const std::vector<int> is{1, 2, 3};
    const auto expected{[&]() {
        std::vector<A> as{};
        std::transform(std::begin(is), std::end(is), std::back_inserter(as), [](int i) {
            return A{make_unique<int>(i)};
        });
        return as;
    }()};

    {
        const auto actual{create_from_ctor_param<std::vector<A>>(is)};
        BOOST_TEST(std::equal(std::begin(actual), std::end(actual), std::begin(expected)));
    }
    {
        const auto actual{create_from_ctor_param<std::list<A>>(is)};
        BOOST_TEST(std::equal(std::begin(actual), std::end(actual), std::begin(expected)));
    }
    {
        const auto actual{create_from_ctor_param<std::deque<A>>(is)};
        BOOST_TEST(std::equal(std::begin(actual), std::end(actual), std::begin(expected)));
    }
}
} // namespace
