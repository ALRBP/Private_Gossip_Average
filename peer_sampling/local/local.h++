
#include <cstddef>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <boost/dynamic_bitset.hpp>
#include "../../helpers/helpers.h++"
#include "../../random_set/random_set.h++"
#include "../../connector/generic.h++"
#include "../generic.h++"

#ifndef LOC_GOS_CON
class LocalGossipConnector : public GenericGossipConnector
{
public:
    struct PeerSet
    {
        std::shared_mutex mtx;
        
        RandomSet<boost::dynamic_bitset<>> set;
        
        std::unordered_map<boost::dynamic_bitset<>,std::atomic_uint> hdlrs;
    };

private:
    GenericConnector* co;
    
    PeerSet* peers;
    
public:
    LocalGossipConnector(void);
    
    LocalGossipConnector(const LocalGossipConnector&);
    
    LocalGossipConnector(GenericConnector*, PeerSet*);
    
    ~LocalGossipConnector(void);

    LocalGossipConnector& operator=(const LocalGossipConnector&);
    
    boost::dynamic_bitset<> getAddress(void) const override;
    
    size_t getAddressLen(void) const noexcept override;
    
    void sendTo(const boost::dynamic_bitset<>&, size_t, const void*) override;
    
    bool recvFrom(boost::dynamic_bitset<>&, size_t&, void*, const std::chrono::steady_clock::time_point&) override;
    
    boost::dynamic_bitset<> getRandomPeer(void) const override;
    
    void injectPeer(const boost::dynamic_bitset<>&) override;
    
    void reportBadPeer(const boost::dynamic_bitset<>&) override;
};
#define LOC_GOS_CON
#endif
