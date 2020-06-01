#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <ostream>

namespace xzr_test
{
struct A
{
    explicit A(int aa)
        : a{aa}
    {
    }
    explicit A(std::unique_ptr<int> ee)
        : e{std::move(ee)}
    {
    }
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

inline bool operator==(const A& a, const A& b)
{
    if (a.e && b.e)
        return std::tie(a.a, a.b, a.c, a.d, *a.e) == std::tie(b.a, b.b, b.c, b.d, *b.e);
    if (!a.e && !b.e)
        return std::tie(a.a, a.b, a.c, a.d) == std::tie(b.a, b.b, b.c, b.d);
    return false;
}

inline bool operator!=(const A& a, const A& b)
{
    return !(a == b);
}

inline std::ostream& operator<<(std::ostream& out, const A& a)
{
    out << "a=" << a.a << " b=" << a.b << " c=" << a.c << " d=" << a.d << " e=";
    if (a.e)
        out << (*a.e);
    else
        out << "nullptr";
    return out;
}
} // namespace xzr_test
