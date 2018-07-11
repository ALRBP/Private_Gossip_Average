#include <chrono>
#include <random>
#include <vector>
#include "helpers.h++"


boost::dynamic_bitset<> randAddr(size_t l)
{
    boost::dynamic_bitset<> ans(l);
    std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::bernoulli_distribution randGen(0.5);
    for(size_t i = 0; i < l; ++i)
    {
        ans[i] = randGen(gen);
    }
    return ans;
}

namespace std
{
size_t hash<boost::dynamic_bitset<>>::operator()(const boost::dynamic_bitset<>& s) const
{
    std::vector<bool> v(s.size());
    for(size_t i = 0; i < s.size(); ++i)
    {
        v[i] = s[i];
    }
    return std::hash<std::vector<bool>>{}(v);
}

void operator+=(std::valarray<double>& v1, const std::valarray<unsigned>& v2)
{
    for(size_t i = 0; i < ((v1.size()<v2.size())?v1.size():v2.size()); ++i)
    {
        v1[i]+= v2[i];
    }
}
}
