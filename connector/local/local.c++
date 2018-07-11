
#include <cstring>
#include <chrono>
#include "local.h++"


LocalConnector::LocalConnector(void)
{
    addr = boost::dynamic_bitset<>();
    map = nullptr;
}

LocalConnector::LocalConnector(const LocalConnector& c)
{
    addr = c.addr;
    map = c.map;
    if(map != nullptr)
    {
        std::shared_lock<std::shared_mutex> lck(map->mtx);
        ++map->map[addr]->nHandelers;
    }
}

LocalConnector::LocalConnector(const boost::dynamic_bitset<>& address, ConnectMap* cm)
{
    addr = address;
    map = cm;
    if(map != nullptr)
    {
        std::unique_lock<std::shared_mutex> lck(map->mtx);
        if(map->map.find(addr) == map->map.end())
        {
            ConnectMap::Elem* e = new ConnectMap::Elem;
            e->nHandelers = 1;
            map->map.emplace(addr,e);
        }
        else
        {
            ++map->map[addr]->nHandelers;
        }
    }
}

LocalConnector::~LocalConnector(void)
{
    if(map != nullptr)
    {
        std::unique_lock<std::shared_mutex> lck(map->mtx);
        if(map->map[addr]->nHandelers > 1)
        {
            --map->map[addr]->nHandelers;
        }
        else
        {
            while(!map->map[addr]->inc.empty())
            {
                free(map->map[addr]->inc.front().second.second);
                map->map[addr]->inc.pop();
            }
            delete map->map[addr];
            map->map.erase(addr);
        }
    }
}

LocalConnector& LocalConnector::operator=(const LocalConnector& c)
{
    if(map != nullptr)
    {
        std::unique_lock<std::shared_mutex> lck(map->mtx);
        if(map->map[addr]->nHandelers > 1)
        {
            --map->map[addr]->nHandelers;
        }
        else
        {
            while(!map->map[addr]->inc.empty())
            {
                free(map->map[addr]->inc.front().second.second);
                map->map[addr]->inc.pop();
            }
            delete map->map[addr];
            map->map.erase(addr);
        }
    }
    addr = c.addr;
    map = c.map;
    if(map != nullptr)
    {
        std::shared_lock<std::shared_mutex> lck(map->mtx);
        ++map->map[addr]->nHandelers;
    }
    return *this;
}

boost::dynamic_bitset<> LocalConnector::getAddress(void) const
{
    return addr;
}

size_t LocalConnector::getAddressLen(void) const noexcept
{
    return addr.size();
}

void LocalConnector::sendTo(const boost::dynamic_bitset<>& dest, size_t bs, const void* buff)
{
    if((map != nullptr) && (map->map.find(dest) != map->map.end()))
    {
        void* container;
        if(bs == 0)
        {
            container = nullptr;
        }
        else
        {
            container = malloc(bs);
            memcpy(container,buff,bs);
        }
        std::shared_lock<std::shared_mutex> lck1(map->mtx);
        std::unique_lock<std::mutex> lck2(map->map[dest]->mtx);
        map->map[dest]->inc.push(std::pair<boost::dynamic_bitset<>,std::pair<size_t,void*>>(addr,std::pair<size_t,void*>(bs,container)));
        map->map[dest]->cv.notify_one();
    }
}

bool LocalConnector::recvFrom(boost::dynamic_bitset<>& orig, size_t& bs, void* buff, const std::chrono::steady_clock::time_point& tp)
{
    bool ans = true;
    if(map != nullptr)
    {
        std::shared_lock<std::shared_mutex> lck1(map->mtx);
        std::unique_lock<std::mutex> lck2(map->map[addr]->mtx);
        while(map->map[addr]->inc.empty() && std::chrono::steady_clock::now() < tp)
        {
            map->map[addr]->cv.wait_until(lck2,tp);
        }
        if(!map->map[addr]->inc.empty())
        {
            ans = false;
            orig = map->map[addr]->inc.front().first;
            if(map->map[addr]->inc.front().second.first < bs)
            {
                bs = map->map[addr]->inc.front().second.first;
            }
            if(bs > 0)
            {
                memcpy(buff,map->map[addr]->inc.front().second.second,bs);
            }
            if(map->map[addr]->inc.front().second.second != nullptr)
            {
                free(map->map[addr]->inc.front().second.second);
            }
            map->map[addr]->inc.pop();
        }
    }
    return ans;
}
