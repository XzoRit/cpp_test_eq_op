#pragma once

#include <lib/tuple.hpp>

#include <iterator>
#include <utility>

namespace xzr
{
namespace iterator
{
template <typename Container>
class back_emplace_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
  public:
    using container_type = Container;

    explicit back_emplace_iterator(Container& c)
        : container{c}
    {
    }

    template <class Args>
    back_emplace_iterator& operator=(Args&& args)
    {
        xzr::tuple::emplace_back_from_tuple(container, std::forward<Args>(args));
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
    Container& container;
};

template <typename Container>
inline back_emplace_iterator<Container> back_emplacer(Container& c)
{
    return back_emplace_iterator<Container>{c};
}
} // namespace iterator
} // namespace xzr
