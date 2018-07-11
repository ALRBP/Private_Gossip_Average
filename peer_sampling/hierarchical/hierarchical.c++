
#include <cstdint>
#include <cstring>
#include <chrono>
#include <random>
#include "hierarchical.h++"


HierarchicalGossipConnector::HierarchicalGossipConnector(void)
{
    co = nullptr;
    addrLen = 0;
    bufferSize = 0;
    active = false;
}
    
HierarchicalGossipConnector::HierarchicalGossipConnector(const HierarchicalGossipConnector& c)
{
    co = c.co;
    addrLen = c.addrLen;
    pullSize = c.pullSize;
    cleanEvery = c.cleanEvery;
    pullEvery = c.pullEvery;
    bufferSize = c.bufferSize;
    peers = c.peers;
    active = false;
}

HierarchicalGossipConnector::HierarchicalGossipConnector(GenericConnector* c, int pullFreq, uint64_t pullNum, size_t deterLimit, size_t keepLimit, size_t cleanFreq, size_t datagramMaxLen)
{
    co = c;
    addrLen = c->getAddressLen();
    pullSize = pullNum;
    cleanEvery = cleanFreq;
    pullEvery = std::chrono::milliseconds(pullFreq);
    bufferSize = sizeof(Type)+datagramMaxLen;
    size_t pl = sizeof(Type)+((((pullSize*addrLen)%8)==0)?(pullSize*addrLen)/8:((pullSize*addrLen)/8)+1);
    if(pl > bufferSize)
    {
        bufferSize = pl;
    }
    if(sizeof(Type)+sizeof(uint64_t) > bufferSize)
    {
        bufferSize = sizeof(Type)+sizeof(uint64_t);
    }
    peers = BpbaTree(addrLen,deterLimit,keepLimit);
    active = false;
}

HierarchicalGossipConnector::~HierarchicalGossipConnector(void)
{
    inactivate();
    while(!inc.empty())
    {
        free(inc.front().second.second);
        inc.pop();
    }
}

HierarchicalGossipConnector& HierarchicalGossipConnector::operator=(const HierarchicalGossipConnector& c)
{
    inactivate();
    while(!inc.empty())
    {
        free(inc.front().second.second);
        inc.pop();
    }
    co = c.co;
    addrLen = c.addrLen;
    pullSize = c.pullSize;
    cleanEvery = c.cleanEvery;
    pullEvery = c.pullEvery;
    bufferSize = c.bufferSize;
    peers = c.peers;
    active = false;
    return *this;
}

void HierarchicalGossipConnector::activate(void)
{
    std::unique_lock<std::mutex> lck(mtx);
    if((co != nullptr) && (!active))
    {
        active = true;
        thread = std::thread(&HierarchicalGossipConnector::threadFunction,this);
    }
}

void HierarchicalGossipConnector::inactivate(void)
{
    std::unique_lock<std::mutex> lck(mtx);
    if(active)
    {
        active = false;
        thread.join();
    }
}

boost::dynamic_bitset<> HierarchicalGossipConnector::getAddress(void) const
{
    if(co == nullptr)
    {
        return boost::dynamic_bitset<>();
    }
    return co->getAddress();
}

size_t HierarchicalGossipConnector::getAddressLen(void) const noexcept
{
    return addrLen;
}

void HierarchicalGossipConnector::sendTo(const boost::dynamic_bitset<>& dest, size_t bs, const void* buff)
{
    if(co != nullptr)
    {
        void* tbuff = malloc(sizeof(Type)+bs);
        *((Type*)tbuff) = Type::Dat;
        memcpy(((Type*)tbuff)+1,buff,bs);
        co->sendTo(dest,sizeof(Type)+bs,tbuff);
        free(tbuff);
    }
}

bool HierarchicalGossipConnector::recvFrom(boost::dynamic_bitset<>& orig, size_t& bs, void* buff, const std::chrono::steady_clock::time_point& tp)
{
    bool ans = true;
    std::unique_lock<std::mutex> lck(mtx);
    if(!inc.empty())
    {
        ans = false;
        orig = inc.front().first;
        if(inc.front().second.first < bs)
        {
            bs = inc.front().second.first;
        }
        if(bs > 0)
        {
            memcpy(buff,inc.front().second.second,bs);
        }
        if(inc.front().second.second != nullptr)
        {
            free(inc.front().second.second);
        }
        inc.pop();
    }
    else if(active)
    {
        while(inc.empty() && std::chrono::steady_clock::now() < tp)
        {
            cv.wait_until(lck,tp);
        }
        if(!inc.empty())
        {
            ans = false;
            orig = inc.front().first;
            if(inc.front().second.first < bs)
            {
                bs = inc.front().second.first;
            }
            if(bs > 0)
            {
                memcpy(buff,inc.front().second.second,bs);
            }
            if(inc.front().second.second != nullptr)
            {
                free(inc.front().second.second);
            }
            inc.pop();
        }
    }
    else if(co != nullptr)
    {
        size_t tbs = sizeof(Type)+bs;
        void* tbuff = malloc(tbs);
        do
        {
            tbs = sizeof(Type)+bs;
            ans = co->recvFrom(orig,tbs,tbuff,tp);
        }
        while((!ans) && ((tbs < sizeof(Type)) || (*((Type*)tbuff) != Type::Dat)));
        if(!ans && tbs >= sizeof(Type))
        {
            bs = tbs-sizeof(Type);
            if(bs > 0)
            {
                memcpy(buff,((Type*)tbuff)+1,bs);
            }
        }
        free(tbuff);
    }
    return ans;
}
    
boost::dynamic_bitset<> HierarchicalGossipConnector::getRandomPeer(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    if(peers.isEmpty())
    {
        return boost::dynamic_bitset<>();
    }
    return peers.rand();
}

void HierarchicalGossipConnector::injectPeer(const boost::dynamic_bitset<>& p)
{
    std::unique_lock<std::shared_mutex> lck(pmtx);
    peers.insert(p);
}

void HierarchicalGossipConnector::reportBadPeer(const boost::dynamic_bitset<>& p)
{
    std::unique_lock<std::shared_mutex> lck(pmtx);
    peers.remove(p);
}
    
std::unordered_set<boost::dynamic_bitset<>> HierarchicalGossipConnector::allPeers(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    return peers.getElems();
}

std::unordered_set<boost::dynamic_bitset<>> HierarchicalGossipConnector::deterPeers(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    return peers.getDeterElems();
}

std::unordered_set<boost::dynamic_bitset<>> HierarchicalGossipConnector::keepPeers(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    return peers.getKeepElems();
}

std::unordered_set<boost::dynamic_bitset<>> HierarchicalGossipConnector::randPeers(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    return peers.getRandElems();
}

std::unordered_set<boost::dynamic_bitset<>> HierarchicalGossipConnector::tempPeers(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    return peers.getTempElems();
}

size_t HierarchicalGossipConnector::viewSize(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    return peers.getSize();
}

size_t HierarchicalGossipConnector::deterViewSize(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    return peers.getDeterSize();
}

size_t HierarchicalGossipConnector::keepViewSize(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    return peers.getKeepSize();
}
    
std::unordered_map<boost::dynamic_bitset<>,double> HierarchicalGossipConnector::outEdges(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    return peers.elemsProba();
}
    
std::unordered_map<boost::dynamic_bitset<>,double> HierarchicalGossipConnector::keepEdges(void) const
{
    std::shared_lock<std::shared_mutex> lck(pmtx);
    return peers.elemsKeep();
}

void HierarchicalGossipConnector::threadFunction(void)
{
    size_t pulls = 0;
    while(active)
    {
        std::chrono::steady_clock::time_point tOut = std::chrono::steady_clock::now() + pullEvery;
        std::shared_lock<std::shared_mutex> slck(pmtx);
        boost::dynamic_bitset<> peer = peers.rand();
        slck.unlock();
        void* buff = malloc(sizeof(Type)+sizeof(uint64_t));
        *((Type*)buff) = Type::Req;
        *((uint64_t*)(((Type*)buff)+1)) = pullSize;
        co->sendTo(peer,sizeof(Type)+sizeof(uint64_t),buff);
        free(buff);
        boost::dynamic_bitset<> orig;
        size_t s = bufferSize;
        buff = malloc(s);
        while(!co->recvFrom(orig,s,buff,tOut) && active)
        {
            if(s >= sizeof(Type))
            {
                switch(*((Type*)buff))
                {
                    case Type::Req:
                        if(s >= (sizeof(Type)+sizeof(uint64_t)))
                        {
                            uint64_t ps = *((uint64_t*)(((Type*)buff)+1));
                            slck.lock();
                            std::unordered_set<boost::dynamic_bitset<>> ans = peers.randSet(ps);
                            slck.unlock();
                            size_t bs = sizeof(Type)+((((ans.size()*addrLen)%8)==0)?(ans.size()*addrLen)/8:((ans.size()*addrLen)/8)+1);
                            void* buff2 = calloc(bs,1);
                            *((Type*)buff2) = Type::Ans;
                            serializeAddresses(ans,(((Type*)buff2)+1));
                            co->sendTo(orig,bs,buff2);
                            free(buff2);
                            std::unique_lock<std::shared_mutex> lck(pmtx);
                            peers.insert(orig);
                        }
                        break;
                    case Type::Ans:
                        if(orig == peer)
                        {
                            std::list<boost::dynamic_bitset<>> ans = extractAddresses((((Type*)buff)+1),pullSize,s-sizeof(Type));
                            for(auto& addr : ans)
                            {
                                if(addr != co->getAddress())
                                {
                                    std::unique_lock<std::shared_mutex> lck(pmtx);
                                    peers.insert(addr);
                                }
                            }
                            ++pulls;
                            peer = boost::dynamic_bitset<>(addrLen);
                        }
                        break;
                    case Type::Dat:
                    {
                        std::unique_lock<std::mutex> lck(mtx);
                        void* container;
                        if(s == sizeof(Type))
                        {
                            container = nullptr;
                        }
                        else
                        {
                            container = malloc(s-sizeof(Type));
                            memcpy(container,(((Type*)buff)+1),s-sizeof(Type));
                        }
                        inc.push(std::pair<boost::dynamic_bitset<>,std::pair<size_t,void*>>(orig,std::pair<size_t,void*>(s-sizeof(Type),container)));
                        cv.notify_one();
                        break;
                    }
                }
            }
            s = bufferSize;
        }
        free(buff);
        if(pulls == cleanEvery)
        {
            std::unique_lock<std::shared_mutex> lck(pmtx);
            peers.clean();
            pulls = 0;
        }
    }
}

void HierarchicalGossipConnector::serializeAddresses(const std::unordered_set<boost::dynamic_bitset<>> s, void* buff) const
{
    size_t i = s.size()*addrLen-1;
    for(auto& addr : s)
    {
        for(size_t k = addr.size()-1; k < addr.size(); --k)
        {
            if(i%8 != 7)
            {
                *((uint8_t*)buff+(i/8))<<=1;
            }
            if(addr[k])
            {
                ++((uint8_t*)buff)[i/8];
            }
            --i;
        }
    }
}

std::list<boost::dynamic_bitset<>> HierarchicalGossipConnector::extractAddresses(const void* data, size_t s, size_t bs) const
{
    std::list<boost::dynamic_bitset<>> ans;
    for(size_t i = 0; i < s && ((((((i+1)*addrLen)%8)==0)?((i+1)*addrLen)/8:(((i+1)*addrLen)/8)+1) <= bs); ++i)
    {
        ans.push_back(boost::dynamic_bitset<>(addrLen));
        for(size_t j = 0; j < addrLen; ++j)
        {
            ans.back()[j] = ((uint8_t)((*((uint8_t*)data+((i*addrLen+j)/8)))<<(7-((i*addrLen+j)%8))))>>7;
        }
    }
    return ans;
}
