
#include <unordered_set>
#include <unordered_map>
#include <boost/dynamic_bitset.hpp>
#include "graph.h++"


PeersGraph::PeersGraph(void)
{
    return;
}
 
PeersGraph::PeersGraph(const PeersGraph& t)
{
    W = t.W;
}

PeersGraph::PeersGraph(HierarchicalGossipConnector tab[], size_t num)
{
    W = boost::numeric::ublas::matrix<double>(num,num);
    for(size_t i = 0; i < num; ++i)
    {
        for(size_t j = 0; j < num; ++j)
        {
            W.insert_element(i,j,0.0);
        }
    }
    std::unordered_map<boost::dynamic_bitset<>,size_t> index(num);
    for(size_t i = 0; i < num; ++i)
    {
        index.emplace(tab[i].getAddress(),i);
    }
    for(size_t i = 0; i < num; ++i)
    {
        std::unordered_map<boost::dynamic_bitset<>,double> peers = tab[i].outEdges();
        for(auto& p : peers)
        {
            W.insert_element(i,index[p.first],p.second);
        }
    }
}

PeersGraph::~PeersGraph(void)
{
    return;
}

PeersGraph& PeersGraph::operator=(const PeersGraph& t)
{
    W = t.W;
    return *this;
}

std::valarray<unsigned> PeersGraph::inDegs(void) const
{
    std::valarray<unsigned> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(j,i) > 0.0)
            {
                ++ans[i];
            }
        }
    }
    return ans;
}

std::valarray<unsigned> PeersGraph::outDegs(void) const
{
    std::valarray<unsigned> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(i,j) > 0.0)
            {
                ++ans[i];
            }
        }
    }
    return ans;
}

std::valarray<unsigned> PeersGraph::totDegs(void) const
{
    std::valarray<unsigned> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(i,j) > 0.0)
            {
                ++ans[i];
            }
            if(W(j,i) > 0.0)
            {
                ++ans[i];
            }
        }
    }
    return ans;
}

std::valarray<unsigned> PeersGraph::biDegs(void) const
{
    std::valarray<unsigned> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if((W(i,j) > 0.0) && (W(j,i) > 0.0))
            {
                ++ans[i];
            }
        }
    }
    return ans;
}

std::valarray<unsigned> PeersGraph::degs(void) const
{
    std::valarray<unsigned> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if((W(i,j) > 0.0) || (W(j,i) > 0.0))
            {
                ++ans[i];
            }
        }
    }
    return ans;
}

std::valarray<double> PeersGraph::inWeights(void) const
{
    std::valarray<double> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            ans[i]+= W(j,i);
        }
    }
    return ans;
}

std::valarray<double> PeersGraph::outWeights(void) const
{
    std::valarray<double> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            ans[i]+= W(i,j);
        }
    }
    return ans;
}

std::valarray<double> PeersGraph::totWeights(void) const
{
    std::valarray<double> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            ans[i]+= W(i,j);
            ans[i]+= W(j,i);
        }
    }
    return ans;
}

std::valarray<double> PeersGraph::biWeights(void) const
{
    std::valarray<double> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if((W(i,j) > 0.0) && (W(j,i) > 0.0))
            {
                ans[i]+= (W(i,j)+W(j,i))/2;
            }
        }
    }
    return ans;
}

std::valarray<double> PeersGraph::weights(void) const
{
    std::valarray<double> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if((W(i,j) > 0.0) || (W(j,i) > 0.0))
            {
                ans[i]+= (W(i,j)+W(j,i))/2;
            }
        }
    }
    return ans;
}

double PeersGraph::avgIODeg(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(i,j) > 0.0)
            {
                ans+= 1.0;
            }
        }
    }
    ans/= W.size1();
    return ans;
}

double PeersGraph::avgTotDeg(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(i,j) > 0.0)
            {
                ans+= 2.0;
            }
        }
    }
    ans/= W.size1();
    return ans;
}

double PeersGraph::avgBiDeg(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if((W(i,j) > 0.0) && (W(j,i) > 0.0))
            {
                ans+= 1.0;
            }
        }
    }
    ans/= W.size1();
    return ans;
}

double PeersGraph::avgDeg(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if((W(i,j) > 0.0) || (W(j,i) > 0.0))
            {
                ans+= 1.0;
            }
        }
    }
    ans/= W.size1();
    return ans;
}

double PeersGraph::avgIOWeight(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            ans+= W(i,j);
        }
    }
    ans/= W.size1();
    return ans;
}

double PeersGraph::avgTotWeight(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            ans+= W(i,j);
        }
    }
    ans*= 2;
    ans/= W.size1();
    return ans;
}

double PeersGraph::avgBiWeight(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if((W(i,j) > 0.0) && (W(j,i) > 0.0))
            {
                ans+= (W(i,j)+W(j,i))/2;
            }
        }
    }
    ans*= 2;
    ans/= W.size1();
    return ans;
}

double PeersGraph::avgWeight(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if((W(i,j) > 0.0) || (W(j,i) > 0.0))
            {
                ans+= (W(i,j)+W(j,i))/2;
            }
        }
    }
    ans/= W.size1();
    return ans;
}

std::valarray<double> PeersGraph::bidirShares(void) const
{
    std::valarray<double> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        unsigned l = 0;
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if((W(i,j) > 0.0) && (W(j,i) > 0.0))
            {
                ++l;
                ans[i]+= 1.0;
            }
            else if((W(i,j) > 0.0) || (W(j,i) > 0.0))
            {
                ++l;
            }
        }
        if(l != 0)
        {
            ans[i]/= l;
        }
    }
    return ans;
}

double PeersGraph::bidirShare(void) const
{
    double ans = 0.0;
    unsigned l = 0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if((W(i,j) > 0.0) && (W(j,i) > 0.0))
            {
                ++l;
                ans+= 1.0;
            }
            else if((W(i,j) > 0.0) || (W(j,i) > 0.0))
            {
                ++l;
            }
        }
    }
    if(l != 0)
    {
        ans/= l;
    }
    return ans;
}

std::valarray<int> PeersGraph::difDegs(void) const
{
    std::valarray<int> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(i,j) > 0.0)
            {
                ++ans[i];
            }
            if(W(j,i) > 0.0)
            {
                --ans[i];
            }
        }
    }
    return ans;
}

std::valarray<double> PeersGraph::difWeights(void) const
{
    std::valarray<double> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        for(size_t j = 0; j < W.size2(); ++j)
        {
            ans[i]+= W(i,j);
            ans[i]-= W(j,i);
        }
    }
    return ans;
}

double PeersGraph::avgAbsDifDeg(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        int di = 0;
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(i,j) > 0.0)
            {
                ++di;
            }
            if(W(j,i) > 0.0)
            {
                --di;
            }
        }
        ans+= abs(di);
    }
    ans/= W.size1();
    return ans;
}

double PeersGraph::avgAbsDifWeight(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        double wi = 0;
        for(size_t j = 0; j < W.size2(); ++j)
        {
            wi+= W(i,j);
            wi-= W(j,i);
        }
        ans+= abs(wi);
    }
    ans/= W.size1();
    return ans;
}

std::valarray<double> PeersGraph::locUwClustCoefs(void) const
{
    std::valarray<double> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        unsigned div = 0;
        std::unordered_set<size_t> neibi, neibo;
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(i,j) > 0.0)
            {
                neibo.insert(j);
            }
            if(W(j,i) > 0.0)
            {
                neibi.insert(j);
            }
        }
        for(auto& j : neibo)
        {
            for(auto& k : neibi)
            {
                if(k != j)
                {
                    if(W(j,k) > 0.0)
                    {
                        ans+= 1.0;
                    }
                    div++;
                }
            }
        }
        if(div != 0)
        {
            ans/= div;
        }
    }
    return ans;
}

std::valarray<double> PeersGraph::locClustCoefs(void) const
{
    std::valarray<double> ans(W.size1());
    for(size_t i = 0; i < W.size1(); ++i)
    {
        double div = 0;
        std::unordered_set<size_t> neibi, neibo;
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(i,j) > 0.0)
            {
                neibo.insert(j);
            }
            if(W(j,i) > 0.0)
            {
                neibi.insert(j);
            }
        }
        for(auto& j : neibo)
        {
            for(auto& k : neibi)
            {
                if(k != j)
                {
                    if(W(j,k) > 0.0)
                    {
                        ans+= (W(i,j)+W(k,i));
                    }
                    div+= (W(i,j)+W(k,i));
                }
            }
        }
        if(div != 0)
        {
            ans/= div;
        }
    }
    return ans;
}

double PeersGraph::avgUwClustCoef(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        unsigned num = 0, den = 0;
        std::unordered_set<size_t> neibi, neibo;
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(i,j) > 0.0)
            {
                neibo.insert(j);
            }
            if(W(j,i) > 0.0)
            {
                neibi.insert(j);
            }
        }
        for(auto& j : neibo)
        {
            for(auto& k : neibi)
            {
                if(k != j)
                {
                    if(W(j,k) > 0.0)
                    {
                        num++;
                    }
                    den++;
                }
            }
        }
        if(den != 0)
        {
            ans+= (num/den);
        }
    }
    ans/= W.size1();
    return ans;
}

double PeersGraph::avgClustCoef(void) const
{
    double ans = 0.0;
    for(size_t i = 0; i < W.size1(); ++i)
    {
        double num = 0.0, den = 0.0;
        std::unordered_set<size_t> neibi, neibo;
        for(size_t j = 0; j < W.size2(); ++j)
        {
            if(W(i,j) > 0.0)
            {
                neibo.insert(j);
            }
            if(W(j,i) > 0.0)
            {
                neibi.insert(j);
            }
        }
        for(auto& j : neibo)
        {
            for(auto& k : neibi)
            {
                if(k != j)
                {
                    if(W(j,k) > 0.0)
                    {
                        num+= (W(i,j)+W(k,i));
                    }
                    den+= (W(i,j)+W(k,i));
                }
            }
        }
        if(den != 0.0)
        {
            ans+= (num/den);
        }
    }
    ans/= W.size1();
    return ans;
}

unsigned PeersGraph::diameter(void) const
{
    boost::numeric::ublas::matrix<double> B = W+boost::numeric::ublas::identity_matrix<double>(W.size1()), M = boost::numeric::ublas::identity_matrix<double>(W.size1());
    unsigned pn, c = W.size1(), k = 0;
    do
    {
        pn = c;
        c = 0;
        M = prod(M,B);
        for(size_t i = 0; i < W.size1(); ++i)
        {
            for(size_t j = 0; j < W.size2(); ++j)
            {
                if(M(i,j) > 0.0)
                {
                    c++;
                }
            }
        }
        k++;
    }
    while(c>pn);
    return k-1;
}

double PeersGraph::avgShortPathLength(void) const
{
    boost::numeric::ublas::matrix<double> B = W+boost::numeric::ublas::identity_matrix<double>(W.size1()), M = boost::numeric::ublas::identity_matrix<double>(W.size1());
    double ans = 0;
    unsigned pn, c = W.size1(), k = 1;
    do
    {
        pn = c;
        c = 0;
        M = prod(M,B);
        for(size_t i = 0; i < W.size1(); ++i)
        {
            for(size_t j = 0; j < W.size2(); ++j)
            {
                if(M(i,j) > 0.0)
                {
                    c++;
                }
            }
        }
        ans+= k*(c-pn);
        k++;
    }
    while(c>pn);
    ans/= (W.size1()*(W.size2()-1));
    return ans;
}
