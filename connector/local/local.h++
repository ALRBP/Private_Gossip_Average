
#include <cstddef>
#include <chrono>
#include <utility>
#include <queue>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <boost/dynamic_bitset.hpp>
#include "../../helpers/helpers.h++"
#include "../generic.h++"

#ifndef LOC_CON
class LocalConnector : public GenericConnector
{
public:
    struct ConnectMap
    {
        struct Elem
        {
            std::atomic_uint nHandelers;
            
            std::mutex mtx;
        
            std::condition_variable cv;
        
            std::queue<std::pair<boost::dynamic_bitset<>,std::pair<size_t,void*>>> inc; 
        };
        
        std::shared_mutex mtx;
        
        std::unordered_map<boost::dynamic_bitset<>,Elem*> map;
    };

private:
    boost::dynamic_bitset<> addr;
    
    ConnectMap* map;
    
public:
    LocalConnector(void);
    
    LocalConnector(const LocalConnector&);
    
    LocalConnector(const boost::dynamic_bitset<>&, ConnectMap*);
    
    ~LocalConnector(void);

    LocalConnector& operator=(const LocalConnector&);
    
    boost::dynamic_bitset<> getAddress(void) const override;
    
    size_t getAddressLen(void) const noexcept override;
    
    void sendTo(const boost::dynamic_bitset<>&, size_t, const void*) override;
    
    bool recvFrom(boost::dynamic_bitset<>&, size_t&, void*, const std::chrono::steady_clock::time_point&) override;
};
#define LOC_CON
#endif
