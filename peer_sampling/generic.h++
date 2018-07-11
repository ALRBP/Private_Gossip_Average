
#include <cstddef>
#include <chrono>
#include <boost/dynamic_bitset.hpp>

#ifndef GEN_GOS_CON
class GenericGossipConnector
{
public:
    virtual boost::dynamic_bitset<> getAddress(void) const = 0;
    
    virtual size_t getAddressLen(void) const noexcept = 0;
    
    virtual void sendTo(const boost::dynamic_bitset<>&, size_t, const void*) = 0;
    
    virtual bool recvFrom(boost::dynamic_bitset<>&, size_t&, void*, const std::chrono::steady_clock::time_point&) = 0;
    
    virtual boost::dynamic_bitset<> getRandomPeer(void) const = 0;
    
    virtual void injectPeer(const boost::dynamic_bitset<>&) = 0;
    
    virtual void reportBadPeer(const boost::dynamic_bitset<>&) = 0;
};
#define GEN_GOS_CON
#endif
