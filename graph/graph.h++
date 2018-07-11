
#include <cstddef>
#include <valarray>
#include <boost/numeric/ublas/matrix.hpp>
#include "../peer_sampling/hierarchical/hierarchical.h++"

#ifndef PEERS_GRAPH
class PeersGraph
{
    boost::numeric::ublas::matrix<double> W;
    
public:
    PeersGraph(void);
    
    PeersGraph(const PeersGraph&);
    
    PeersGraph(HierarchicalGossipConnector[], size_t);
    
    ~PeersGraph(void);

    PeersGraph& operator=(const PeersGraph&);
    
    std::valarray<unsigned> inDegs(void) const;
    
    std::valarray<unsigned> outDegs(void) const;
    
    std::valarray<unsigned> totDegs(void) const;
    
    std::valarray<unsigned> biDegs(void) const;
    
    std::valarray<unsigned> degs(void) const;
    
    std::valarray<double> inWeights(void) const;
    
    std::valarray<double> outWeights(void) const;
    
    std::valarray<double> totWeights(void) const;
    
    std::valarray<double> biWeights(void) const;
    
    std::valarray<double> weights(void) const;
    
    double avgIODeg(void) const;
    
    double avgTotDeg(void) const;
    
    double avgBiDeg(void) const;
    
    double avgDeg(void) const;
    
    double avgIOWeight(void) const;
    
    double avgTotWeight(void) const;
    
    double avgBiWeight(void) const;
    
    double avgWeight(void) const;
    
    std::valarray<double> bidirShares(void) const;
    
    double bidirShare(void) const;
    
    std::valarray<int> difDegs(void) const;
    
    std::valarray<double> difWeights(void) const;
    
    double avgAbsDifDeg(void) const;
    
    double avgAbsDifWeight(void) const;
    
    std::valarray<double> locUwClustCoefs(void) const;
    
    std::valarray<double> locClustCoefs(void) const;
    
    double avgUwClustCoef(void) const;
    
    double avgClustCoef(void) const;
    
    unsigned diameter(void) const;
    
    double avgShortPathLength(void) const;
};
#define PEERS_GRAPH
#endif
