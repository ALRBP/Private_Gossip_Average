
#include <cstddef>
#include <unordered_set>
#include <boost/dynamic_bitset.hpp>
#include "../helpers/helpers.h++"
#include "../random_set/random_set.h++"

#ifndef BPBA_TREE
class BpbaTree
{
    struct Node
    {
        boost::dynamic_bitset<> subnet;
        
        size_t mask;
        
        Node* parent;
        
        Node* zero;
        
        Node* one;
    };
    
public:
    using const_iterator = typename std::unordered_set<boost::dynamic_bitset<>>::const_iterator;
    
private:
    Node* root;
    
    size_t size;
    
    size_t addrLen;
    
    size_t deterThld;
    
    size_t keepThld;
    
    RandomSet<Node*> deterLeaves;
    
    std::unordered_set<Node*> keepLeaves;
    
    std::unordered_set<boost::dynamic_bitset<>> elems;
    
public:
    BpbaTree(void);
    
    BpbaTree(const BpbaTree&);
    
    BpbaTree(size_t, size_t, size_t);
    
    ~BpbaTree(void);

    BpbaTree& operator=(const BpbaTree&);
    
    void insert(const boost::dynamic_bitset<>&);
    
    void remove(const boost::dynamic_bitset<>&);
    
    bool find(const boost::dynamic_bitset<>&) const;
    
    bool findProba(const boost::dynamic_bitset<>&) const;
    
    boost::dynamic_bitset<> rand(void) const;
    
    std::unordered_set<boost::dynamic_bitset<>> randSet(size_t) const;
    
    void clean(void);
    
    void setDeterThld(size_t);
    
    void setKeepThld(size_t);
    
    size_t getAddrLen(void) const noexcept;
    
    size_t getDeterThld(void) const noexcept;
    
    size_t getKeepThld(void) const noexcept;
    
    size_t getSize(void) const noexcept;
    
    size_t getDeterSize(void) const noexcept;
    
    size_t getKeepSize(void) const noexcept;
    
    bool isEmpty(void) const noexcept;
    
    const_iterator begin(void) const noexcept;
    
    const_iterator end(void) const noexcept;
    
    std::unordered_set<boost::dynamic_bitset<>> getElems(void) const;
    
    std::unordered_set<boost::dynamic_bitset<>> getDeterElems(void) const;
    
    std::unordered_set<boost::dynamic_bitset<>> getKeepElems(void) const;
    
    std::unordered_set<boost::dynamic_bitset<>> getRandElems(void) const;
    
    std::unordered_set<boost::dynamic_bitset<>> getTempElems(void) const;
    
    std::unordered_map<boost::dynamic_bitset<>,double> elemsProba(void) const;
    
    std::unordered_map<boost::dynamic_bitset<>,double> elemsKeep(void) const;
    
private:
    void getDeterLeaves(Node*, size_t);
    
    void getKeepLeaves(Node*, size_t);
    
    Node* copyTree(Node*, const RandomSet<Node*>&, const std::unordered_set<Node*>&);
    
    void clearTree(Node*);
    
    void freeTree(Node*);
    
    void elemsProbaReq(const Node*, double, std::unordered_map<boost::dynamic_bitset<>,double>&) const;
    
    void elemsKeepReq(const Node*, double, std::unordered_map<boost::dynamic_bitset<>,double>&) const;
};
#define BPBA_TREE
#endif
