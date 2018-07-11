
#include <chrono>
#include <random>
#include "local.h++"


LocalGossipConnector::LocalGossipConnector(void)
{
    co = nullptr;
    peers = nullptr;
}
    
LocalGossipConnector::LocalGossipConnector(const LocalGossipConnector& c)
{
    co = c.co;
    peers = c.peers;
    if(peers != nullptr)
    {
        std::shared_lock<std::shared_mutex> lck(peers->mtx);
        ++peers->hdlrs[co->getAddress()];
    }
}

LocalGossipConnector::LocalGossipConnector(GenericConnector* c, PeerSet* ps)
{
    co = c;
    peers = ps;
    if(peers != nullptr)
    {
        std::unique_lock<std::shared_mutex> lck(peers->mtx);
        if(peers->set.find(co->getAddress()))
        {
            ++peers->hdlrs[co->getAddress()];
        }
        else
        {
            peers->set.insert(co->getAddress());
            peers->hdlrs.emplace(co->getAddress(),1);
        }
    }
}

LocalGossipConnector::~LocalGossipConnector(void)
{
    if(peers != nullptr)
    {
        std::unique_lock<std::shared_mutex> lck(peers->mtx);
        if(peers->hdlrs[co->getAddress()] > 1)
        {
            --peers->hdlrs[co->getAddress()];
        }
        else
        {
            peers->set.remove(co->getAddress());
            peers->hdlrs.erase(co->getAddress());
        }
    }
}

LocalGossipConnector& LocalGossipConnector::operator=(const LocalGossipConnector& c)
{
    if(peers != nullptr)
    {
        std::unique_lock<std::shared_mutex> lck(peers->mtx);
        if(peers->hdlrs[co->getAddress()] > 1)
        {
            --peers->hdlrs[co->getAddress()];
        }
        else
        {
            peers->set.remove(co->getAddress());
            peers->hdlrs.erase(co->getAddress());
        }
    }
    co = c.co;
    peers = c.peers;
    if(peers != nullptr)
    {
        std::shared_lock<std::shared_mutex> lck(peers->mtx);
        ++peers->hdlrs[co->getAddress()];
    }
    return *this;
}

boost::dynamic_bitset<> LocalGossipConnector::getAddress(void) const
{
    if(co == nullptr)
    {
        return boost::dynamic_bitset<>();
    }
    return co->getAddress();
}

size_t LocalGossipConnector::getAddressLen(void) const noexcept
{
    if(co == nullptr)
    {
        return 0;
    }
    return co->getAddressLen();
}

void LocalGossipConnector::sendTo(const boost::dynamic_bitset<>& dest, size_t bs, const void* buff)
{
    if(co != nullptr)
    {
        co->sendTo(dest,bs,buff);
    }
}

bool LocalGossipConnector::recvFrom(boost::dynamic_bitset<>& orig, size_t& bs, void* buff, const std::chrono::steady_clock::time_point& tp)
{
    return (co == nullptr)?true:co->recvFrom(orig,bs,buff,tp);
}
    
boost::dynamic_bitset<> LocalGossipConnector::getRandomPeer(void) const
{
    if(peers == nullptr)
    {
        return boost::dynamic_bitset<>();
    }
    return peers->set.rand();
}

void LocalGossipConnector::injectPeer(const boost::dynamic_bitset<>&)
{
    return;
}

void LocalGossipConnector::reportBadPeer(const boost::dynamic_bitset<>&)
{
    return;
}
