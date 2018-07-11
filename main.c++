
#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <string>
#include "helpers/helpers.h++"
#include "connector/local/local.h++"
#include "peer_sampling/local/local.h++"
#include "peer_sampling/req-pull/req-pull.h++"
#include "peer_sampling/hierarchical/hierarchical.h++"
#include "average/average.h++"
#include "graph/graph.h++"

#define ARG_ERR "Error! Incorrect arguments!\nCorrect usage:\npga-test avg <address length> <peers count> <runing time> <security level> <min peer value> <max peer value> <security factor> <protection factor> <wait time> <max answers during securing>\npga-test ps <address length> <peers count> <runing time> <time between pulls> <pull size> <deterministic threshold> <keep threshold> <pulls between cleans>\npga-test full <address length> <peers count> <runing time> <security level> <min peer value> <max peer value> <security factor> <protection factor> <wait time> <max answers during securing> <time between pulls> <pull size> <deterministic threshold> <keep threshold> <pulls between cleans> <startup time>\npga-test stat <address length> <peers count> <runing time> <time between pulls> <pull size> <deterministic threshold> <keep threshold> <pulls between cleans> <number of runs>\npga-test conv <address length> <peers count> <runing time> <security level> <min peer value> <max peer value> <security factor> <protection factor> <wait time> <max answers during securing> <time between pulls> <pull size> <deterministic threshold> <keep threshold> <pulls between cleans> <startup time> <exchanges between samples> <step between sampled peers>\npga-test rp <address length> <peers count> <runing time> <time between pulls> <pull size> <max view size>"


int main(int argc, char *argv[])
{
    size_t addrLen, peersCnt;
    int rTime;
    if(argc < 5)
    {
        std::cout << ARG_ERR << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string mode(argv[1]);
    sscanf(argv[2],"%zu",&addrLen);
    sscanf(argv[3],"%zu",&peersCnt);
    sscanf(argv[4],"%d",&rTime);
    if(mode == "avg")
    {
        int wt;
        unsigned sec, ansSec;
        double minVal, maxVal, secFac, protFac, realAvg = 0.0, compAvg = 0.0;
        if(argc != 12)
        {
            std::cout << ARG_ERR << std::endl;
            exit(EXIT_FAILURE);
        }
        sscanf(argv[5],"%u",&sec);
        sscanf(argv[6],"%lf",&minVal);
        sscanf(argv[7],"%lf",&maxVal);
        sscanf(argv[8],"%lf",&secFac);
        sscanf(argv[9],"%lf",&protFac);
        sscanf(argv[10],"%d",&wt);
        sscanf(argv[11],"%u",&ansSec);
        std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<double> randGen(minVal,maxVal);
        LocalConnector::ConnectMap cm;
        LocalGossipConnector::PeerSet ps;
        LocalConnector lcTab[peersCnt];
        LocalGossipConnector lgcTab[peersCnt];
        PrivateGossipAveragePeer pgapTab[peersCnt];
        double initV[peersCnt], endV[peersCnt];
        for(size_t i = 0; i < peersCnt; ++i)
        {
            boost::dynamic_bitset<> addr;
            do
            {
                addr = randAddr(addrLen);
            }
            while(cm.map.find(addr) != cm.map.end());
            lcTab[i] = LocalConnector(addr,&cm);
            lgcTab[i] = LocalGossipConnector(lcTab+i,&ps);
            initV[i] = randGen(gen);
            realAvg+= initV[i]/peersCnt;
            pgapTab[i] = PrivateGossipAveragePeer(initV[i],sec,((secFac+1.0)*minVal-(secFac-1.0)*maxVal)/2.0,((secFac+1.0)*maxVal-(secFac-1.0)*minVal)/2.0,protFac,wt,ansSec,0,lgcTab+i);
        }
        for(size_t i = 0; i < peersCnt; ++i)
        {
            pgapTab[i].activate();
        }
        std::this_thread::sleep_for(std::chrono::seconds(rTime));
        for(size_t i = 0; i < peersCnt; ++i)
        {
            pgapTab[i].inactivate();
        }
        for(size_t i = 0; i < peersCnt; ++i)
        {
            endV[i] = pgapTab[i].getCurrentVal();
            compAvg+= endV[i]/peersCnt;
        }
        std::cout << "Initial values: [";
        for(size_t i = 0; i < peersCnt-1; ++i)
        {
            std::cout << std::setw(7) << std::setprecision(2) << initV[i] << ';';
        }
        std::cout << std::setw(7) << std::setprecision(2) << initV[peersCnt-1] << "] | " << realAvg << std::endl;
        std::cout << "Final   values: [";
        for(size_t i = 0; i < peersCnt-1; ++i)
        {
            std::cout << std::setw(7) << std::setprecision(2) << endV[i] << ';';
        }
        std::cout << std::setw(7) << std::setprecision(2) << endV[peersCnt-1] << "] | " << compAvg << std::endl;
    }
    else if(mode == "ps")
    {
        size_t cleanEvery, deterThld, keepThld;
        int pullEvery;
        uint64_t pullSize;
        if(argc != 10)
        {
            std::cout << ARG_ERR << std::endl;
            exit(EXIT_FAILURE);
        }
        sscanf(argv[5],"%d",&pullEvery);
        sscanf(argv[6],"%lu",&pullSize);
        sscanf(argv[7],"%zu",&deterThld);
        sscanf(argv[8],"%zu",&keepThld);
        sscanf(argv[9],"%zu",&cleanEvery);
        LocalConnector::ConnectMap cm;
        LocalConnector lcTab[peersCnt];
        HierarchicalGossipConnector hgcTab[peersCnt];
        lcTab[0] = LocalConnector(randAddr(addrLen),&cm);
        hgcTab[0] = HierarchicalGossipConnector(lcTab,pullEvery,pullSize,deterThld,keepThld,cleanEvery,0);
        for(size_t i = 1; i < peersCnt; ++i)
        {
            boost::dynamic_bitset<> addr;
            do
            {
                addr = randAddr(addrLen);
            }
            while(cm.map.find(addr) != cm.map.end());
            lcTab[i] = LocalConnector(addr,&cm);
            hgcTab[i] = HierarchicalGossipConnector(lcTab+i,pullEvery,pullSize,deterThld,keepThld,cleanEvery,0);
            hgcTab[i].injectPeer(hgcTab[0].getAddress());
        }
        for(size_t i = 0; i < peersCnt; ++i)
        {
            hgcTab[i].activate();
        }
        std::this_thread::sleep_for(std::chrono::seconds(rTime));
        for(size_t i = 0; i < peersCnt; ++i)
        {
            hgcTab[i].inactivate();
        }
        std::cout << "Local views:" << std::endl;
        for(size_t i = 0; i < peersCnt; ++i)
        {
            std::cout << hgcTab[i].getAddress() << ": (" << hgcTab[i].deterPeers().size() << ';' << hgcTab[i].randPeers().size() << ';' << hgcTab[i].tempPeers().size() << '|' << hgcTab[i].allPeers().size() << ")-(" << hgcTab[i].deterViewSize() << ',' << hgcTab[i].keepViewSize() << ',' << hgcTab[i].viewSize() << "):" << std::endl;
            for(auto& x : hgcTab[i].deterPeers())
            {
                std::cout << x << ' ';
            }
            std::cout << ';' << std::endl;
            for(auto& x : hgcTab[i].randPeers())
            {
                std::cout << x << ' ';
            }
            std::cout << ';' << std::endl;
            for(auto& x : hgcTab[i].tempPeers())
            {
                std::cout << x << ' ';
            }
            std::cout << '|' << std::endl;
            for(auto& x : hgcTab[i].allPeers())
            {
                std::cout << x << ' ';
            }
            std::cout << '\n' << std::endl;
        }
    }
    else if(mode == "full")
    {
        size_t cleanEvery, deterThld, keepThld;
        int wt, pullEvery;
        unsigned sec, ansSec, stt;
        uint64_t pullSize;
        double minVal, maxVal, secFac, protFac, realAvg = 0.0, compAvg = 0.0;
        if(argc != 18)
        {
            std::cout << ARG_ERR << std::endl;
            exit(EXIT_FAILURE);
        }
        sscanf(argv[5],"%u",&sec);
        sscanf(argv[6],"%lf",&minVal);
        sscanf(argv[7],"%lf",&maxVal);
        sscanf(argv[8],"%lf",&secFac);
        sscanf(argv[9],"%lf",&protFac);
        sscanf(argv[10],"%d",&wt);
        sscanf(argv[11],"%u",&ansSec);
        sscanf(argv[12],"%d",&pullEvery);
        sscanf(argv[13],"%lu",&pullSize);
        sscanf(argv[14],"%zu",&deterThld);
        sscanf(argv[15],"%zu",&keepThld);
        sscanf(argv[16],"%zu",&cleanEvery);
        sscanf(argv[17],"%u",&stt);
        std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<double> randGen(minVal,maxVal);
        LocalConnector::ConnectMap cm;
        LocalConnector lcTab[peersCnt];
        HierarchicalGossipConnector hgcTab[peersCnt];
        PrivateGossipAveragePeer pgapTab[peersCnt];
        double initV[peersCnt], endV[peersCnt];
        lcTab[0] = LocalConnector(randAddr(addrLen),&cm);
        hgcTab[0] = HierarchicalGossipConnector(lcTab,pullEvery,pullSize,deterThld,keepThld,cleanEvery,PrivateGossipAveragePeer::getMaxDatagramLen());
        initV[0] = randGen(gen);
        realAvg+= initV[0]/peersCnt;
        pgapTab[0] = PrivateGossipAveragePeer(initV[0],sec,((secFac+1.0)*minVal-(secFac-1.0)*maxVal)/2.0,((secFac+1.0)*maxVal-(secFac-1.0)*minVal)/2.0,protFac,wt,ansSec,0,hgcTab);
        for(size_t i = 1; i < peersCnt; ++i)
        {
            boost::dynamic_bitset<> addr;
            do
            {
                addr = randAddr(addrLen);
            }
            while(cm.map.find(addr) != cm.map.end());
            lcTab[i] = LocalConnector(addr,&cm);
            hgcTab[i] = HierarchicalGossipConnector(lcTab+i,pullEvery,pullSize,deterThld,keepThld,cleanEvery,PrivateGossipAveragePeer::getMaxDatagramLen());
            hgcTab[i].injectPeer(hgcTab[0].getAddress());
            initV[i] = randGen(gen);
            realAvg+= initV[i]/peersCnt;
            pgapTab[i] = PrivateGossipAveragePeer(initV[i],sec,((secFac+1.0)*minVal-(secFac-1.0)*maxVal)/2.0,((secFac+1.0)*maxVal-(secFac-1.0)*minVal)/2.0,protFac,wt,ansSec,0,hgcTab+i);
        }
        for(size_t i = 0; i < peersCnt; ++i)
        {
            hgcTab[i].activate();
        }
        std::this_thread::sleep_for(std::chrono::seconds(stt));
        for(size_t i = 0; i < peersCnt; ++i)
        {
            pgapTab[i].activate();
        }
        std::this_thread::sleep_for(std::chrono::seconds(rTime));
        for(size_t i = 0; i < peersCnt; ++i)
        {
            pgapTab[i].inactivate();
            hgcTab[i].inactivate();
        }
        for(size_t i = 0; i < peersCnt; ++i)
        {
            endV[i] = pgapTab[i].getCurrentVal();
            compAvg+= endV[i]/peersCnt;
        }
        std::cout << "Local views:" << std::endl;
        for(size_t i = 0; i < peersCnt; ++i)
        {
            std::cout << hgcTab[i].getAddress() << ": (" << hgcTab[i].deterPeers().size() << ';' << hgcTab[i].randPeers().size() << ';' << hgcTab[i].tempPeers().size() << '|' << hgcTab[i].allPeers().size() << ")-(" << hgcTab[i].deterViewSize() << ',' << hgcTab[i].keepViewSize() << ',' << hgcTab[i].viewSize() << "):" << std::endl;
            for(auto& x : hgcTab[i].deterPeers())
            {
                std::cout << x << ' ';
            }
            std::cout << ';' << std::endl;
            for(auto& x : hgcTab[i].randPeers())
            {
                std::cout << x << ' ';
            }
            std::cout << ';' << std::endl;
            for(auto& x : hgcTab[i].tempPeers())
            {
                std::cout << x << ' ';
            }
            std::cout << '|' << std::endl;
            for(auto& x : hgcTab[i].allPeers())
            {
                std::cout << x << ' ';
            }
            std::cout << '\n' << std::endl;
        }
        std::cout << "Initial values: [";
        for(size_t i = 0; i < peersCnt-1; ++i)
        {
            std::cout << std::setw(7) << std::setprecision(2) << initV[i] << ';';
        }
        std::cout << std::setw(7) << std::setprecision(2) << initV[peersCnt-1] << "] | " << realAvg << std::endl;
        std::cout << "Final   values: [";
        for(size_t i = 0; i < peersCnt-1; ++i)
        {
            std::cout << std::setw(7) << std::setprecision(2) << endV[i] << ';';
        }
        std::cout << std::setw(7) << std::setprecision(2) << endV[peersCnt-1] << "] | " << compAvg << std::endl;
    }
    else if(mode == "stat")
    {
        size_t cleanEvery, deterThld, keepThld, ns;
        int pullEvery;
        uint64_t pullSize;
        if(argc != 11)
        {
            std::cout << ARG_ERR << std::endl;
            exit(EXIT_FAILURE);
        }
        sscanf(argv[5],"%d",&pullEvery);
        sscanf(argv[6],"%lu",&pullSize);
        sscanf(argv[7],"%zu",&deterThld);
        sscanf(argv[8],"%zu",&keepThld);
        sscanf(argv[9],"%zu",&cleanEvery);
        sscanf(argv[10],"%zu",&ns);
        std::valarray<double> ind(peersCnt);
        std::valarray<double> outd(peersCnt);
        std::valarray<double> inw(peersCnt);
        std::valarray<double> outw(peersCnt);
        double cc = 0.0;
        double pl = 0.0;
        for(size_t k = 0; k < ns; ++k)
        {
            LocalConnector::ConnectMap cm;
            LocalConnector lcTab[peersCnt];
            HierarchicalGossipConnector hgcTab[peersCnt];
            lcTab[0] = LocalConnector(randAddr(addrLen),&cm);
            hgcTab[0] = HierarchicalGossipConnector(lcTab,pullEvery,pullSize,deterThld,keepThld,cleanEvery,0);
            for(size_t i = 1; i < peersCnt; ++i)
            {
                boost::dynamic_bitset<> addr;
                do
                {
                    addr = randAddr(addrLen);
                }
                while(cm.map.find(addr) != cm.map.end());
                lcTab[i] = LocalConnector(addr,&cm);
                hgcTab[i] = HierarchicalGossipConnector(lcTab+i,pullEvery,pullSize,deterThld,keepThld,cleanEvery,0);
                hgcTab[i].injectPeer(hgcTab[0].getAddress());
            }
            for(size_t i = 0; i < peersCnt; ++i)
            {
                hgcTab[i].activate();
            }
            std::this_thread::sleep_for(std::chrono::seconds(rTime));
            for(size_t i = 0; i < peersCnt; ++i)
            {
                hgcTab[i].inactivate();
            }
            PeersGraph g(hgcTab,peersCnt);
            std::valarray<unsigned> indl(g.inDegs());
            std::valarray<unsigned> outdl(g.outDegs());
            std::valarray<double> inwl(g.inWeights());
            std::valarray<double> outwl(g.outWeights());
            cc+= g.avgClustCoef();
            pl+= g.avgShortPathLength();
            std::sort(begin(indl),end(indl));
            std::sort(begin(outdl),end(outdl));
            std::sort(begin(inwl),end(inwl));
            std::sort(begin(outwl),end(outwl));
            ind+= indl;
            outd+= outdl;
            inw+= inwl;
            outw+= outwl;
        }
        ind/= ns;
        outd/= ns;
        inw/= ns;
        outw/= ns;
        cc/= ns;
        pl/= ns;
        std::ofstream df("degs.dat",std::ofstream::trunc);
        std::ofstream wf("weights.dat",std::ofstream::trunc);
        for(size_t i = 0; i < peersCnt; ++i)
        {
            df << i << ' ' << ind[i] << ' ' << outd[i] << std::endl;
            wf << i << ' ' << inw[i] << ' ' << outw[i] << std::endl;
        }
        std::cout << "Average Clustering Coefficient: " << cc << std::endl;
        std::cout << "Average Shortest Path Length: " << pl << std::endl;
    }
    else if(mode == "conv")
    {
        size_t cleanEvery, deterThld, keepThld, sampleFor;
        int wt, pullEvery;
        unsigned sec, ansSec, sampleEvery, stt;
        uint64_t pullSize;
        double minVal, maxVal, secFac, protFac;
        if(argc != 20)
        {
            std::cout << ARG_ERR << std::endl;
            exit(EXIT_FAILURE);
        }
        sscanf(argv[5],"%u",&sec);
        sscanf(argv[6],"%lf",&minVal);
        sscanf(argv[7],"%lf",&maxVal);
        sscanf(argv[8],"%lf",&secFac);
        sscanf(argv[9],"%lf",&protFac);
        sscanf(argv[10],"%d",&wt);
        sscanf(argv[11],"%u",&ansSec);
        sscanf(argv[12],"%d",&pullEvery);
        sscanf(argv[13],"%lu",&pullSize);
        sscanf(argv[14],"%zu",&deterThld);
        sscanf(argv[15],"%zu",&keepThld);
        sscanf(argv[16],"%zu",&cleanEvery);
        sscanf(argv[17],"%u",&stt);
        sscanf(argv[18],"%u",&sampleEvery);
        sscanf(argv[19],"%zu",&sampleFor);
        std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<double> randGen(minVal,maxVal);
        LocalConnector::ConnectMap cm;
        LocalConnector lcTab[peersCnt];
        HierarchicalGossipConnector hgcTab[peersCnt];
        PrivateGossipAveragePeer pgapTab[peersCnt];
        lcTab[0] = LocalConnector(randAddr(addrLen),&cm);
        hgcTab[0] = HierarchicalGossipConnector(lcTab,pullEvery,pullSize,deterThld,keepThld,cleanEvery,PrivateGossipAveragePeer::getMaxDatagramLen());
        pgapTab[0] = PrivateGossipAveragePeer(randGen(gen),sec,((secFac+1.0)*minVal-(secFac-1.0)*maxVal)/2.0,((secFac+1.0)*maxVal-(secFac-1.0)*minVal)/2.0,protFac,wt,ansSec,sampleEvery,hgcTab);
        for(size_t i = 1; i < peersCnt; ++i)
        {
            boost::dynamic_bitset<> addr;
            do
            {
                addr = randAddr(addrLen);
            }
            while(cm.map.find(addr) != cm.map.end());
            lcTab[i] = LocalConnector(addr,&cm);
            hgcTab[i] = HierarchicalGossipConnector(lcTab+i,pullEvery,pullSize,deterThld,keepThld,cleanEvery,PrivateGossipAveragePeer::getMaxDatagramLen());
            hgcTab[i].injectPeer(hgcTab[0].getAddress());
            pgapTab[i] = PrivateGossipAveragePeer(randGen(gen),sec,((secFac+1.0)*minVal-(secFac-1.0)*maxVal)/2.0,((secFac+1.0)*maxVal-(secFac-1.0)*minVal)/2.0,protFac,wt,ansSec,((i%sampleFor)?0:sampleEvery),hgcTab+i);
        }
        for(size_t i = 0; i < peersCnt; ++i)
        {
            hgcTab[i].activate();
        }
        std::this_thread::sleep_for(std::chrono::seconds(stt));
        std::chrono::steady_clock::time_point st = std::chrono::steady_clock::now();
        for(size_t i = 0; i < peersCnt; ++i)
        {
            pgapTab[i].activate();
        }
        std::this_thread::sleep_for(std::chrono::seconds(rTime));
        for(size_t i = 0; i < peersCnt; ++i)
        {
            pgapTab[i].inactivate();
            hgcTab[i].inactivate();
        }
        for(size_t i = 0; (i*sampleFor) < peersCnt; ++i)
        {
            std::ofstream f("hsample"+std::to_string(i)+".dat",std::ofstream::trunc);
            for(auto& s : pgapTab[i].getValSamples())
            {
                f << (s.first-st).count() << ' ' << s.second << std::endl;
            }
        }
        ReqPullGossipConnector rpgcTab[peersCnt];
        lcTab[0] = LocalConnector(randAddr(addrLen),&cm);
        rpgcTab[0] = ReqPullGossipConnector(lcTab,pullEvery,pullSize,1<<keepThld,PrivateGossipAveragePeer::getMaxDatagramLen());
        pgapTab[0] = PrivateGossipAveragePeer(randGen(gen),sec,((secFac+1.0)*minVal-(secFac-1.0)*maxVal)/2.0,((secFac+1.0)*maxVal-(secFac-1.0)*minVal)/2.0,protFac,wt,ansSec,sampleEvery,rpgcTab);
        for(size_t i = 1; i < peersCnt; ++i)
        {
            boost::dynamic_bitset<> addr;
            do
            {
                addr = randAddr(addrLen);
            }
            while(cm.map.find(addr) != cm.map.end());
            lcTab[i] = LocalConnector(addr,&cm);
            rpgcTab[i] = ReqPullGossipConnector(lcTab+i,pullEvery,pullSize,1<<keepThld,PrivateGossipAveragePeer::getMaxDatagramLen());
            rpgcTab[i].injectPeer(rpgcTab[0].getAddress());
            pgapTab[i] = PrivateGossipAveragePeer(randGen(gen),sec,((secFac+1.0)*minVal-(secFac-1.0)*maxVal)/2.0,((secFac+1.0)*maxVal-(secFac-1.0)*minVal)/2.0,protFac,wt,ansSec,((i%sampleFor)?0:sampleEvery),rpgcTab+i);
        }
        for(size_t i = 0; i < peersCnt; ++i)
        {
            rpgcTab[i].activate();
        }
        std::this_thread::sleep_for(std::chrono::seconds(stt));
        st = std::chrono::steady_clock::now();
        for(size_t i = 0; i < peersCnt; ++i)
        {
            pgapTab[i].activate();
        }
        std::this_thread::sleep_for(std::chrono::seconds(rTime));
        for(size_t i = 0; i < peersCnt; ++i)
        {
            pgapTab[i].inactivate();
            rpgcTab[i].inactivate();
        }
        for(size_t i = 0; (i*sampleFor) < peersCnt; ++i)
        {
            std::ofstream f("psample"+std::to_string(i)+".dat",std::ofstream::trunc);
            for(auto& s : pgapTab[i].getValSamples())
            {
                f << (s.first-st).count() << ' ' << s.second << std::endl;
            }
        }
        LocalGossipConnector::PeerSet ps;
        LocalGossipConnector lgcTab[peersCnt];
        for(size_t i = 0; i < peersCnt; ++i)
        {
            boost::dynamic_bitset<> addr;
            do
            {
                addr = randAddr(addrLen);
            }
            while(cm.map.find(addr) != cm.map.end());
            lcTab[i] = LocalConnector(addr,&cm);
            lgcTab[i] = LocalGossipConnector(lcTab+i,&ps);
            pgapTab[i] = PrivateGossipAveragePeer(randGen(gen),sec,((secFac+1.0)*minVal-(secFac-1.0)*maxVal)/2.0,((secFac+1.0)*maxVal-(secFac-1.0)*minVal)/2.0,protFac,wt,ansSec,((i%sampleFor)?0:sampleEvery),lgcTab+i);
        }
        st = std::chrono::steady_clock::now();
        for(size_t i = 0; i < peersCnt; ++i)
        {
            pgapTab[i].activate();
        }
        std::this_thread::sleep_for(std::chrono::seconds(rTime));
        for(size_t i = 0; i < peersCnt; ++i)
        {
            pgapTab[i].inactivate();
        }
        for(size_t i = 0; (i*sampleFor) < peersCnt; ++i)
        {
            std::ofstream f("fsample"+std::to_string(i)+".dat",std::ofstream::trunc);
            for(auto& s : pgapTab[i].getValSamples())
            {
                f << (s.first-st).count() << ' ' << s.second << std::endl;
            }
        }
    }
    else if(mode == "rp")
    {
        size_t mvs;
        int pullEvery;
        uint64_t pullSize;
        if(argc != 8)
        {
            std::cout << ARG_ERR << std::endl;
            exit(EXIT_FAILURE);
        }
        sscanf(argv[5],"%d",&pullEvery);
        sscanf(argv[6],"%lu",&pullSize);
        sscanf(argv[7],"%zu",&mvs);
        LocalConnector::ConnectMap cm;
        LocalConnector lcTab[peersCnt];
        ReqPullGossipConnector rpgcTab[peersCnt];
        lcTab[0] = LocalConnector(randAddr(addrLen),&cm);
        rpgcTab[0] = ReqPullGossipConnector(lcTab,pullEvery,pullSize,mvs,0);
        for(size_t i = 1; i < peersCnt; ++i)
        {
            boost::dynamic_bitset<> addr;
            do
            {
                addr = randAddr(addrLen);
            }
            while(cm.map.find(addr) != cm.map.end());
            lcTab[i] = LocalConnector(addr,&cm);
            rpgcTab[i] = ReqPullGossipConnector(lcTab+i,pullEvery,pullSize,mvs,0);
            rpgcTab[i].injectPeer(rpgcTab[0].getAddress());
        }
        for(size_t i = 0; i < peersCnt; ++i)
        {
            rpgcTab[i].activate();
        }
        std::this_thread::sleep_for(std::chrono::seconds(rTime));
        for(size_t i = 0; i < peersCnt; ++i)
        {
            rpgcTab[i].inactivate();
        }
        std::cout << "Local views:" << std::endl;
        for(size_t i = 0; i < peersCnt; ++i)
        {
            std::cout << rpgcTab[i].getAddress() << ":" << rpgcTab[i].viewSize() << std::endl;
            for(auto& x : rpgcTab[i].getPeers())
            {
                std::cout << x << ' ';
            }
            std::cout << '\n' << std::endl;
        }
    }
    else
    {
        std::cout << ARG_ERR << std::endl;
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
