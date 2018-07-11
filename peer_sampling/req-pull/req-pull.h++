
#include <cstddef>
#include <utility>
#include <list>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <unordered_set>
#include <boost/dynamic_bitset.hpp>
#include "../../random_queue/random_queue.h++"
#include "../../helpers/helpers.h++"
#include "../../connector/generic.h++"
#include "../generic.h++"

#ifndef RP_GOS_CON
class ReqPullGossipConnector : public GenericGossipConnector
{
    enum class Type {Req = 'R', Ans = 'A', Dat = 'D'};
    
    GenericConnector* co;
    
    size_t addrLen;
    
    std::chrono::steady_clock::duration pullEvery;
    
    uint64_t pullSize;
    
    size_t bufferSize;
    
    RandomQueue<boost::dynamic_bitset<>> peers;
    
    bool active;
    
    std::mutex mtx;
    
    mutable std::shared_mutex pmtx;
        
    std::condition_variable cv;
    
    std::queue<std::pair<boost::dynamic_bitset<>,std::pair<size_t,void*>>> inc;
    
    std::thread thread;
    
public:
    ReqPullGossipConnector(void);
    
    ReqPullGossipConnector(const ReqPullGossipConnector&);
    
    ReqPullGossipConnector(GenericConnector* c, int, uint64_t, size_t, size_t);
    
    ~ReqPullGossipConnector(void);

    ReqPullGossipConnector& operator=(const ReqPullGossipConnector&);
    
    void activate(void);
    
    void inactivate(void);
    
    boost::dynamic_bitset<> getAddress(void) const override;
    
    size_t getAddressLen(void) const noexcept override;
    
    void sendTo(const boost::dynamic_bitset<>&, size_t, const void*) override;
    
    bool recvFrom(boost::dynamic_bitset<>&, size_t&, void*, const std::chrono::steady_clock::time_point&) override;
    
    boost::dynamic_bitset<> getRandomPeer(void) const override;
    
    void injectPeer(const boost::dynamic_bitset<>&) override;
    
    void reportBadPeer(const boost::dynamic_bitset<>&) override;
    
    std::unordered_set<boost::dynamic_bitset<>> getPeers(void) const;
    
    size_t viewSize(void) const;
    
private:
    void threadFunction(void);

    void serializeAddresses(const std::unordered_set<boost::dynamic_bitset<>>, void*) const;

    std::list<boost::dynamic_bitset<>> extractAddresses(const void*, size_t, size_t) const;
};
#define RP_GOS_CON
#endif
