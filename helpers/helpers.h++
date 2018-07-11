#include <cstddef>
#include <valarray>
#include <functional>
#include <boost/dynamic_bitset.hpp>


#ifndef PGA_HLP
boost::dynamic_bitset<> randAddr(size_t);

namespace std
{
template <>
class hash<boost::dynamic_bitset<>>
{
public:
    size_t operator()(const boost::dynamic_bitset<>&) const;
};

void operator+=(std::valarray<double>&, const std::valarray<unsigned>&);
}
#define PGA_HLP
#endif
