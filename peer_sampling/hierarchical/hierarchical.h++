
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
#include "../../helpers/helpers.h++"
#include "../../bpba_tree/bpba_tree.h++"
#include "../../connector/generic.h++"
#include "../generic.h++"

#ifndef HIR_GOS_CON
class HierarchicalGossipConnector : public GenericGossipConnector
{
    enum class Type {Req = 'R', Ans = 'A', Dat = 'D'};
    
    GenericConnector* co;
    
    size_t addrLen;
    
    size_t cleanEvery;
    
    std::chrono::steady_clock::duration pullEvery;
    
    uint64_t pullSize;
    
    size_t bufferSize;
    
    BpbaTree peers;
    
    bool active;
    
    std::mutex mtx;
    
    mutable std::shared_mutex pmtx;
        
    std::condition_variable cv;
    
    std::queue<std::pair<boost::dynamic_bitset<>,std::pair<size_t,void*>>> inc;
    
    std::thread thread;
    
public:
    HierarchicalGossipConnector(void);
    
    HierarchicalGossipConnector(const HierarchicalGossipConnector&);
    
    HierarchicalGossipConnector(GenericConnector* c, int, uint64_t, size_t, size_t, size_t, size_t);
    
    ~HierarchicalGossipConnector(void);

    HierarchicalGossipConnector& operator=(const HierarchicalGossipConnector&);
    
    void activate(void);
    
    void inactivate(void);
    
    boost::dynamic_bitset<> getAddress(void) const override;
    
    size_t getAddressLen(void) const noexcept override;
    
    void sendTo(const boost::dynamic_bitset<>&, size_t, const void*) override;
    
    bool recvFrom(boost::dynamic_bitset<>&, size_t&, void*, const std::chrono::steady_clock::time_point&) override;
    
    boost::dynamic_bitset<> getRandomPeer(void) const override;
    
    void injectPeer(const boost::dynamic_bitset<>&) override;
    
    void reportBadPeer(const boost::dynamic_bitset<>&) override;
    
    std::unordered_set<boost::dynamic_bitset<>> allPeers(void) const;
    
    std::unordered_set<boost::dynamic_bitset<>> deterPeers(void) const;
    
    std::unordered_set<boost::dynamic_bitset<>> keepPeers(void) const;
    
    std::unordered_set<boost::dynamic_bitset<>> randPeers(void) const;
    
    std::unordered_set<boost::dynamic_bitset<>> tempPeers(void) const;
    
    size_t viewSize(void) const;
    
    size_t deterViewSize(void) const;
    
    size_t keepViewSize(void) const;
    
    std::unordered_map<boost::dynamic_bitset<>,double> outEdges(void) const;
    
    std::unordered_map<boost::dynamic_bitset<>,double> keepEdges(void) const;
    
private:
    void threadFunction(void);

    void serializeAddresses(const std::unordered_set<boost::dynamic_bitset<>>, void*) const;

    std::list<boost::dynamic_bitset<>> extractAddresses(const void*, size_t, size_t) const;
};
#define HIR_GOS_CON
#endif
