
#include <cstring>
#include <cmath>
#include <chrono>
#include <random>
#include "average.h++"


PrivateGossipAveragePeer::PrivateGossipAveragePeer(void)
{
    val = 0.0;
    working = false;
    active = false;
    cnt = nullptr;
}

PrivateGossipAveragePeer::PrivateGossipAveragePeer(const PrivateGossipAveragePeer& p)
{
    origVal = p.origVal;
    val = p.val;
    err = p.err;
    minRand = p.minRand;
    maxRand = p.maxRand;
    minAcpt = p.minAcpt;
    maxAcpt = p.maxAcpt;
    timeOut = p.timeOut;
    secAcpt = p.secAcpt;
    remAcpt = p.remAcpt;
    cnt = p.cnt;
    secLvl = p.secLvl;
    securing = p.securing;
    working = p.working;
    sample = p.sample;
    vals = p.vals;
    active = false;
}

PrivateGossipAveragePeer::PrivateGossipAveragePeer(double value, unsigned sec, double minRandom, double maxRandom, double protFactor, int waitTime, unsigned maxAcptSec, unsigned sampleEvery, GenericGossipConnector* c)
{
    origVal = value;
    val = value;
    err = 0;
    minRand = minRandom;
    maxRand = maxRandom;
    minAcpt = -(protFactor*(sec+1.0)*fmax(abs(minRandom),abs(maxRandom)));
    maxAcpt = protFactor*(sec+1.0)*fmax(abs(minRandom),abs(maxRandom));
    timeOut = std::chrono::milliseconds(waitTime);
    secAcpt = maxAcptSec;
    remAcpt = maxAcptSec;
    cnt = c;
    secLvl = sec;
    securing = sec;
    sample = sampleEvery;
    working = false;
    active = false;
}

PrivateGossipAveragePeer::~PrivateGossipAveragePeer(void)
{
    inactivate();
}

PrivateGossipAveragePeer& PrivateGossipAveragePeer::operator=(const PrivateGossipAveragePeer& p)
{
    inactivate();
    origVal = p.origVal;
    val = p.val;
    err = p.err;
    minRand = p.minRand;
    maxRand = p.maxRand;
    minAcpt = p.minAcpt;
    maxAcpt = p.maxAcpt;
    timeOut = p.timeOut;
    secAcpt = p.secAcpt;
    remAcpt = p.remAcpt;
    cnt = p.cnt;
    secLvl = p.secLvl;
    securing = p.securing;
    sample = p.sample;
    vals = p.vals;
    std::unique_lock<std::mutex> lck(mtx);
    working = p.working;
    cv.notify_all();
    return *this;
}

void PrivateGossipAveragePeer::activate(void)
{
    if((cnt != nullptr) && (!active))
    {
        active = true;
        thread = std::thread(&PrivateGossipAveragePeer::threadFunction,this);
    }
}

void PrivateGossipAveragePeer::inactivate(void)
{
    if(active)
    {
        active = false;
        thread.join();
    }
}

bool PrivateGossipAveragePeer::ready(void) const noexcept
{
    return working;
}

double PrivateGossipAveragePeer::getMean(void) const
{
    std::unique_lock<std::mutex> lck(mtx);
    while(!working)
    {
        cv.wait(lck);
    }
    return val;
}

double PrivateGossipAveragePeer::getCurrentVal(void) const noexcept
{
    return val;
}

std::list<std::pair<std::chrono::steady_clock::time_point,double>> PrivateGossipAveragePeer::getValSamples(void)
{
    return vals;
}

size_t PrivateGossipAveragePeer::getMaxDatagramLen(void) noexcept
{
    return sizeof(Msg);
}

void PrivateGossipAveragePeer::threadFunction(void)
{
    std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> randGen(minRand,maxRand);
    while(active && securing)
    {
        startProto:
        boost::dynamic_bitset<> peer = cnt->getRandomPeer();
        double fakeVal = randGen(gen);
        sndTo(peer,{MsgType::Call,fakeVal});
        boost::dynamic_bitset<> orig;
        tOut = std::chrono::steady_clock::now() + timeOut;
        Msg rcv = rcvFrom(orig);
        while((orig != peer) && (rcv.type != MsgType::None))
        {
            if(rcv.type == MsgType::Call)
            {
                if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
                {
                    sndTo(orig,{MsgType::Rjct,0.0});
                }
                else
                {
                    cnt->reportBadPeer(orig);
                }
            }
            rcv = rcvFrom(orig);
        }
        if(rcv.type == MsgType::Answ)
        {
            if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
            {
                sndTo(orig,{MsgType::Conf,0.0});
                err+= val-fakeVal;
                val = (fakeVal+rcv.val)/2;
                if(minAcpt >= val || val >= maxAcpt)
                {
                    val = origVal;
                    securing = secLvl;
                    remAcpt = secAcpt;
                }
                else
                {
                    --securing;
                }
            }
            else
            {
                cnt->reportBadPeer(orig);
            }
        }
        else if(rcv.type == MsgType::Call)
        {
            if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
            {
                sndTo(orig,{MsgType::Conf,0.0});
                tOut = std::chrono::steady_clock::now() + timeOut;
                double memVal = rcv.val;
                rcv = rcvFrom(orig);
                while((orig != peer) && (rcv.type != MsgType::None))
                {
                    if(rcv.type == MsgType::Call)
                    {
                        if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
                        {
                            sndTo(orig,{MsgType::Rjct,0.0});
                        }
                        else
                        {
                            cnt->reportBadPeer(orig);
                        }
                    }
                    rcv = rcvFrom(orig);
                }
                if(rcv.type == MsgType::Conf)
                {
                    err+= val-fakeVal;
                    val = (fakeVal+memVal)/2;
                    if(minAcpt >= val || val >= maxAcpt)
                    {
                        val = origVal;
                        securing = secLvl;
                        remAcpt = secAcpt;
                    }
                    else
                    {
                        --securing;
                    }
                }
            }
            else
            {
                cnt->reportBadPeer(orig);
            }
        }
        if(active && remAcpt)
        {
            tOut = std::chrono::steady_clock::now() + timeOut;
            do
            {
                rcv = rcvFrom(peer);
                if(rcv.type == MsgType::Call)
                {
                    if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
                    {
                        fakeVal = randGen(gen);
                        sndTo(peer,{MsgType::Answ,fakeVal});
                        tOut = std::chrono::steady_clock::now() + timeOut;
                        double memVal = rcv.val;
                        rcv = rcvFrom(orig);
                        while((orig != peer) && (rcv.type != MsgType::None))
                        {
                            if(rcv.type == MsgType::Call)
                            {
                                if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
                                {
                                    sndTo(orig,{MsgType::Rjct,0.0});
                                }
                                else
                                {
                                    cnt->reportBadPeer(orig);
                                }
                            }
                            rcv = rcvFrom(orig);
                        }
                        if(rcv.type == MsgType::Conf)
                        {
                            err+= val-fakeVal;
                            val = (fakeVal+memVal)/2;
                            --remAcpt;
                            if(minAcpt >= val || val >= maxAcpt)
                            {
                                val = origVal;
                                securing = secLvl;
                                remAcpt = secAcpt;
                            }
                        }
                    }
                    else
                    {
                        cnt->reportBadPeer(peer);
                    }
                }
            }
            while(active && ((rcv.type != MsgType::None) && remAcpt));
        }
    }
    if(active && !working)
    {
        val+= err;
        if(minAcpt >= val || val >= maxAcpt)
        {
            val = origVal;
            securing = secLvl;
            remAcpt = secAcpt;
            err = 0;
            goto startProto;
        }
        std::unique_lock<std::mutex> lck(mtx);
        working = true;
        cv.notify_all();
    }
    while(active && working)
    {
        unsigned untilSample = sample;
        vals.push_back(std::pair(std::chrono::steady_clock::now(),val));
        boost::dynamic_bitset<> peer;
        peer = cnt->getRandomPeer();
        sndTo(peer,{MsgType::Call,val});
        boost::dynamic_bitset<> orig;
        tOut = std::chrono::steady_clock::now() + timeOut;
        Msg rcv = rcvFrom(orig);
        while((orig != peer) && (rcv.type != MsgType::None))
        {
            if(rcv.type == MsgType::Call)
            {
                if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
                {
                    sndTo(orig,{MsgType::Rjct,0.0});
                }
                else
                {
                    cnt->reportBadPeer(orig);
                }
            }
            rcv = rcvFrom(orig);
        }
        if(rcv.type == MsgType::Answ)
        {
            if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
            {
                sndTo(orig,{MsgType::Conf,0.0});
                val = (val+rcv.val)/2;
                if(minAcpt >= val || val >= maxAcpt)
                {
                    val = origVal;
                    securing = secLvl;
                    remAcpt = secAcpt;
                    err = 0;
                    working = false;
                    goto startProto;
                }
                untilSample--;
                if(!untilSample)
                {
                    vals.push_back(std::pair(std::chrono::steady_clock::now(),val));
                    untilSample = sample;
                }
            }
            else
            {
                cnt->reportBadPeer(orig);
            }
        }
        else if(rcv.type == MsgType::Call)
        {
            if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
            {
                sndTo(orig,{MsgType::Conf,0.0});
                tOut = std::chrono::steady_clock::now() + timeOut;
                double memVal = rcv.val;
                rcv = rcvFrom(orig);
                while((orig != peer) && (rcv.type != MsgType::None))
                {
                    if(rcv.type == MsgType::Call)
                    {
                        if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
                        {
                            sndTo(orig,{MsgType::Rjct,0.0});
                        }
                        else
                        {
                            cnt->reportBadPeer(orig);
                        }
                    }
                    rcv = rcvFrom(orig);
                }
                if(rcv.type == MsgType::Conf)
                {
                    val = (val+memVal)/2;
                    if(minAcpt >= val || val >= maxAcpt)
                    {
                        val = origVal;
                        securing = secLvl;
                        remAcpt = secAcpt;
                        err = 0;
                        working = false;
                        goto startProto;
                    }
                    untilSample--;
                    if(!untilSample)
                    {
                        vals.push_back(std::pair(std::chrono::steady_clock::now(),val));
                        untilSample = sample;
                    }
                }
            }
            else
            {
                cnt->reportBadPeer(orig);
            }
        }
        if(active)
        {
            tOut = std::chrono::steady_clock::now() + timeOut;
            do
            {
                rcv = rcvFrom(peer);
                if(rcv.type == MsgType::Call)
                {
                    if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
                    {
                        sndTo(peer,{MsgType::Answ,val});
                        tOut = std::chrono::steady_clock::now() + timeOut;
                        double memVal = rcv.val;
                        rcv = rcvFrom(orig);
                        while((orig != peer) && (rcv.type != MsgType::None))
                        {
                            if(rcv.type == MsgType::Call)
                            {
                                if(minAcpt <= rcv.val && rcv.val <= maxAcpt)
                                {
                                    sndTo(orig,{MsgType::Rjct,0.0});
                                }
                                else
                                {
                                    cnt->reportBadPeer(orig);
                                }
                            }
                            rcv = rcvFrom(orig);
                        }
                        if(rcv.type == MsgType::Conf)
                        {
                            val = (val+memVal)/2;
                            if(minAcpt >= val || val >= maxAcpt)
                            {
                                val = origVal;
                                securing = secLvl;
                                remAcpt = secAcpt;
                                err = 0;
                                working = false;
                                goto startProto;
                            }
                            untilSample--;
                            if(!untilSample)
                            {
                                vals.push_back(std::pair(std::chrono::steady_clock::now(),val));
                                untilSample = sample;
                            }
                        }
                    }
                    else
                    {
                        cnt->reportBadPeer(peer);
                    }
                }
            }
            while(active && (rcv.type != MsgType::None));
        }
    }
}

void PrivateGossipAveragePeer::sndTo(const boost::dynamic_bitset<>& dest, Msg m)
{
    if(m.type == MsgType::Call || m.type == MsgType::Answ)
    {
        cnt->sendTo(dest,sizeof(Msg),&m);
    }
    else
    {
        cnt->sendTo(dest,sizeof(MsgType),&m);
    }
}

PrivateGossipAveragePeer::Msg PrivateGossipAveragePeer::rcvFrom(boost::dynamic_bitset<>& orig)
{
    size_t bs = sizeof(Msg);
    Msg ans;
    if(cnt->recvFrom(orig,bs,&ans,tOut))
    {
        ans.type = MsgType::None;
    }
    else if((bs < sizeof(MsgType)) || ((ans.type != MsgType::Call) && (ans.type != MsgType::Answ) && (ans.type != MsgType::Conf) && (ans.type != MsgType::Rjct)) || (((ans.type == MsgType::Call) || (ans.type == MsgType::Answ)) && (bs < sizeof(Msg))))
    {
        cnt->reportBadPeer(orig);
        ans.type = MsgType::None;
    }
    return ans;
}
