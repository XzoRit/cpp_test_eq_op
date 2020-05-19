#pragma once

#include <memory>
#include <type_traits>
#include <utility>

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
