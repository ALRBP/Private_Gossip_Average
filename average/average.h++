
#include <list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "../peer_sampling/generic.h++"

#ifndef PRV_GOS_AVG
class PrivateGossipAveragePeer
{
    enum class MsgType {None = 'N', Call = 'C', Answ = 'A', Conf = 'K', Rjct = 'R'};
    
    struct Msg
    {
        MsgType type;
        double val;
    };
    
    double origVal;
    
    double val;
    
    double err;
    
    double minRand;
    
    double maxRand;
    
    double minAcpt;
    
    double maxAcpt;
    
    std::chrono::steady_clock::duration timeOut;
    
    unsigned secLvl;
    
    unsigned secAcpt;
    
    unsigned remAcpt;
    
    unsigned securing;
    
    bool working;
    
    bool active;
    
    std::chrono::steady_clock::time_point tOut;
    
    mutable std::mutex mtx;
    
    mutable std::condition_variable cv;
    
    std::thread thread;
    
    GenericGossipConnector* cnt;
    
    unsigned sample;
    
    std::list<std::pair<std::chrono::steady_clock::time_point,double>> vals;
    
public:
    PrivateGossipAveragePeer(void);
    
    PrivateGossipAveragePeer(const PrivateGossipAveragePeer&);
    
    PrivateGossipAveragePeer(double, unsigned, double, double, double, int, unsigned, unsigned, GenericGossipConnector*);
    
    ~PrivateGossipAveragePeer(void);

    PrivateGossipAveragePeer& operator=(const PrivateGossipAveragePeer&);
    
    void activate(void);
    
    void inactivate(void);
    
    bool ready(void) const noexcept;
    
    double getMean(void) const;
    
    double getCurrentVal(void) const noexcept;
    
    std::list<std::pair<std::chrono::steady_clock::time_point,double>> getValSamples(void);
    
    static size_t getMaxDatagramLen(void) noexcept;
    
private:
    void threadFunction(void);
    
    void sndTo(const boost::dynamic_bitset<>&, Msg);
    
    Msg rcvFrom(boost::dynamic_bitset<>&);
};
#define PRV_GOS_AVG
#endif
