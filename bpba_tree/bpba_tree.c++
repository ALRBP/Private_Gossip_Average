
#include <chrono>
#include <random>
#include "bpba_tree.h++"


BpbaTree::BpbaTree(void)
{
    root = nullptr;
    size = 0;
    addrLen = 0;
    deterThld = 0;
    keepThld = 0;
}
 
BpbaTree::BpbaTree(const BpbaTree& t)
{
    size = t.size;
    addrLen = t.addrLen;
    deterThld = t.deterThld;
    keepThld = t.keepThld;
    elems = t.elems;
    if(size > 0)
    {
        root = copyTree(t.root,t.deterLeaves,t.keepLeaves);
        root->parent = nullptr;
    }
    else
    {
        root = nullptr;
    }
}

BpbaTree::BpbaTree(size_t addrBits, size_t deterLimit, size_t keepLimit)
{
    root = nullptr;
    size = 0;
    addrLen = addrBits;
    deterThld = deterLimit;
    keepThld = keepLimit;
}

BpbaTree::~BpbaTree(void)
{
    if(size > 0)
    {
        freeTree(root);
    }
}

BpbaTree& BpbaTree::operator=(const BpbaTree& t)
{
    if(size > 0)
    {
        freeTree(root);
        deterLeaves.clear();
        keepLeaves.clear();
    }
    size = t.size;
    addrLen = t.addrLen;
    deterThld = t.deterThld;
    keepThld = t.keepThld;
    elems = t.elems;
    if(size > 0)
    {
        root = copyTree(t.root,t.deterLeaves,t.keepLeaves);
        root->parent = nullptr;
    }
    else
    {
        root = nullptr;
    }
    return *this;
}

void BpbaTree::insert(const boost::dynamic_bitset<>& addr)
{
    if(size == 0)
    {
        root = new Node{addr,addrLen,nullptr,nullptr,nullptr};
        size = 1;
        deterLeaves.insert(root);
        keepLeaves.insert(root);
    }
    else
    {
        Node* node = root;
        while(((addr ^ node->subnet) >> (addrLen-node->mask)).none())
        {
            if(node->mask == addrLen)
            {
                return;
            }
            if(((addr << node->mask) >> (addrLen-1)).any())
            {
                node = node->one;
            }
            else
            {
                node = node->zero;
            }
        }
        Node* splitNode = new Node{node->subnet,node->mask,node,node->zero,node->one};
        Node* newNode = new Node{addr,addrLen,node,nullptr,nullptr};
        ++size;
        if(node->mask < addrLen)
        {
            node->zero->parent = splitNode;
            node->one->parent = splitNode;
        }
        for(node->mask = 0; !((addr ^ node->subnet)[addrLen-node->mask-1]); ++node->mask);
        if(node->mask < keepThld)
        {
            if(splitNode->mask >= keepThld)
            {
                keepLeaves.erase(node);
                keepLeaves.insert(splitNode);
            }
            keepLeaves.insert(newNode);
            if(node->mask < deterThld)
            {
                if(splitNode->mask >= deterThld)
                {
                    deterLeaves.remove(node);
                    deterLeaves.insert(splitNode);
                }
                deterLeaves.insert(newNode);
            }
        }
        if(addr[addrLen-node->mask-1])
        {
            node->zero = splitNode;
            node->one = newNode;
        }
        else
        {
            node->zero = newNode;
            node->one = splitNode;
        }
    }
    elems.insert(addr);
}

void BpbaTree::remove(const boost::dynamic_bitset<>& addr)
{
    if((size == 1) && (addr == root->subnet))
    {
        size = 0;
        deterLeaves.remove(root);
        keepLeaves.erase(root);
        delete root;
    }
    else if(size > 1)
    {
        Node* node = root;
        if(((addr ^ node->subnet) >> (addrLen-node->mask)).none())
        {
            while(node->mask < addrLen)
            {
                if(((addr << node->mask) >> (addrLen-1)).any())
                {
                    node = node->one;
                }
                else
                {
                    node = node->zero;
                }
                if(((addr ^ node->subnet) >> (addrLen-node->mask)).any())
                {
                    return;
                }
            }
            --size;
            Node* deleteNode = node;
            Node* fusionNode;
            if(node == node->parent->zero)
            {
                fusionNode = node->parent->one;
            }
            else
            {
                fusionNode = node->parent->zero;
            }
            node = node->parent;
            if(node->mask < keepThld)
            {
                keepLeaves.erase(deleteNode);
                if(fusionNode->mask >= keepThld)
                {
                    keepLeaves.erase(fusionNode);
                    keepLeaves.insert(node);
                }
                if(node->mask < deterThld)
                {
                    deterLeaves.remove(deleteNode);
                    if(fusionNode->mask >= deterThld)
                    {
                        deterLeaves.remove(fusionNode);
                        deterLeaves.insert(node);
                    }
                }
            }
            node->subnet = fusionNode->subnet;
            node->mask = fusionNode->mask;
            if(node->mask < addrLen)
            {
                fusionNode->zero->parent = node;
                fusionNode->one->parent = node;
            }
            node->zero = fusionNode->zero;
            node->one = fusionNode->one;
            delete deleteNode;
            delete fusionNode;
        }
    }
    elems.erase(addr);
}

bool BpbaTree::find(const boost::dynamic_bitset<>& addr) const
{
    if(size == 0 || ((addr ^ root->subnet) >> (addrLen-root->mask)).any())
    {
        return false;
    }
    Node* node = root;
    while(node->mask < addrLen)
    {
        if(((addr << node->mask) >> (addrLen-1)).any())
        {
            node = node->one;
        }
        else
        {
            node = node->zero;
        }
        if(((addr ^ node->subnet) >> (addrLen-node->mask)).any())
        {
            return false;
        }
    }
    return true;
}

bool BpbaTree::findProba(const boost::dynamic_bitset<>& addr) const
{
    if(size == 0 || ((addr ^ root->subnet) >> (addrLen-root->mask)).any())
    {
        return false;
    }
    Node* node = root;
    std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::bernoulli_distribution randGen(0.5);
    while(node->mask < addrLen)
    {
        if((node->mask >= deterThld) && randGen(gen))
        {
            return false;
        }
        if(((addr << node->mask) >> (addrLen-1)).any())
        {
            node = node->one;
        }
        else
        {
            node = node->zero;
        }
        if(((addr ^ node->subnet) >> (addrLen-node->mask)).any())
        {
            return false;
        }
    }
    return true;
}

boost::dynamic_bitset<> BpbaTree::rand(void) const
{
    Node* node = deterLeaves.rand();
    if(node == nullptr)
    {
        return boost::dynamic_bitset<>();
    }
    std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::bernoulli_distribution randGen(0.5);
    while(node->mask < addrLen)
    {
        node = (randGen(gen))?node->one:node->zero;
    }
    return node->subnet;
}

std::unordered_set<boost::dynamic_bitset<>> BpbaTree::randSet(size_t s) const
{
    std::unordered_set<Node*> nodeSet = deterLeaves.randSet(s);
    std::unordered_set<boost::dynamic_bitset<>> addrSet;
    std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::bernoulli_distribution randGen(0.5);
    for(auto node : nodeSet)
    {
        while(node->mask < addrLen)
        {
            node = (randGen(gen))?node->one:node->zero;
        }
        addrSet.insert(node->subnet);
    }
    return addrSet;
}

void BpbaTree::clean(void)
{
    std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::bernoulli_distribution randGen(0.5);
    for(auto& base : keepLeaves)
    {
        if(base->mask < addrLen)
        {
            Node* node;
            if(randGen(gen))
            {
                clearTree(base->zero);
                node = base->one;
            }
            else
            {
                clearTree(base->one);
                node = base->zero;
            }
            while(node->mask < addrLen)
            {
                if(randGen(gen))
                {
                    clearTree(node->zero);
                    node = node->one;
                }
                else
                {
                    clearTree(node->one);
                    node = node->zero;
                }
                delete node->parent;
            }
            base->subnet = node->subnet;
            base->mask = addrLen;
            delete node;
        }
    }
}

void BpbaTree::setDeterThld(size_t deterLimit)
{
    if(deterLimit > deterThld)
    {
        for(auto& node : deterLeaves)
        {
            if(node->mask < deterLimit)
            {
                deterLeaves.remove(node);
                getDeterLeaves(node->zero,deterLimit);
                getDeterLeaves(node->one,deterLimit);
            }
        }
    }
    else if(deterLimit < deterThld)
    {
        for(auto node : deterLeaves)
        {
            if((node->parent != nullptr) && (node->parent->mask >= deterLimit))
            {
                deterLeaves.remove(node);
                do
                {
                    node = node->parent;
                }
                while((node->parent != nullptr) && (node->parent->mask >= deterLimit));
                deterLeaves.insert(node);
            }
        } 
    }
    deterThld = deterLimit;
}

void BpbaTree::setKeepThld(size_t keepLimit)
{
    if(keepLimit > keepThld)
    {
        for(auto& node : keepLeaves)
        {
            if(node->mask < keepLimit)
            {
                keepLeaves.erase(node);
                getKeepLeaves(node->zero,keepLimit);
                getKeepLeaves(node->one,keepLimit);
            }
        }
    }
    else if(keepLimit < keepThld)
    {
        for(auto node : keepLeaves)
        {
            if((node->parent != nullptr) && (node->parent->mask >= keepLimit))
            {
                keepLeaves.erase(node);
                do
                {
                    node = node->parent;
                }
                while((node->parent != nullptr) && (node->parent->mask >= keepLimit));
                keepLeaves.insert(node);
            }
        } 
    }
    keepThld = keepLimit;
}

size_t BpbaTree::getAddrLen(void) const noexcept
{
    return addrLen;
}

size_t BpbaTree::getDeterThld(void) const noexcept
{
    return deterThld;
}

size_t BpbaTree::getKeepThld(void) const noexcept
{
    return keepThld;
}

size_t BpbaTree::getSize(void) const noexcept
{
    return size;
}

size_t BpbaTree::getDeterSize(void) const noexcept
{
    return deterLeaves.size();
}

size_t BpbaTree::getKeepSize(void) const noexcept
{
    return keepLeaves.size();
}

bool BpbaTree::isEmpty(void) const noexcept
{
    return (size == 0);
}

BpbaTree::const_iterator BpbaTree::begin(void) const noexcept
{
    return elems.cbegin();
}

BpbaTree::const_iterator BpbaTree::end(void) const noexcept
{
    return elems.cend();
}

std::unordered_set<boost::dynamic_bitset<>> BpbaTree::getElems(void) const
{
    return elems;
}

std::unordered_set<boost::dynamic_bitset<>> BpbaTree::getDeterElems(void) const
{
    std::unordered_set<boost::dynamic_bitset<>> ans;
    for(auto& x : deterLeaves)
    {
        if(x->mask == addrLen)
        {
            ans.insert(x->subnet);
        }
    }
    return ans;
}

std::unordered_set<boost::dynamic_bitset<>> BpbaTree::getKeepElems(void) const
{
    std::unordered_set<boost::dynamic_bitset<>> ans;
    for(auto& x : keepLeaves)
    {
        if(x->mask == addrLen)
        {
            ans.insert(x->subnet);
        }
    }
    return ans;
}

std::unordered_set<boost::dynamic_bitset<>> BpbaTree::getRandElems(void) const
{
    std::unordered_set<boost::dynamic_bitset<>> ans;
    for(auto& x : keepLeaves)
    {
        if((x->mask == addrLen) && (!deterLeaves.find(x)))
        {
            ans.insert(x->subnet);
        }
    }
    return ans;
}

std::unordered_set<boost::dynamic_bitset<>> BpbaTree::getTempElems(void) const
{
    std::unordered_set<boost::dynamic_bitset<>> keep, ans;
    for(auto& x : keepLeaves)
    {
        if(x->mask == addrLen)
        {
            keep.insert(x->subnet);
        }
    }
    for(auto& x : elems)
    {
        if(keep.find(x) == keep.end())
        {
            ans.insert(x);
        }
    }
    return ans;
}
    
std::unordered_map<boost::dynamic_bitset<>,double> BpbaTree::elemsProba(void) const
{
    std::unordered_map<boost::dynamic_bitset<>,double> ans;
    if(root != nullptr)
    {
        elemsProbaReq(root,1.0,ans);
    }
    return ans;
}
    
std::unordered_map<boost::dynamic_bitset<>,double> BpbaTree::elemsKeep(void) const
{
    std::unordered_map<boost::dynamic_bitset<>,double> ans;
    if(root != nullptr)
    {
        elemsKeepReq(root,1.0,ans);
    }
    return ans;
}

void BpbaTree::getDeterLeaves(Node* orig, size_t deterLimit)
{
    if(orig->mask >= deterLimit)
    {
        deterLeaves.insert(orig);
    }
    else
    {
        getDeterLeaves(orig->zero,deterLimit);
        getDeterLeaves(orig->one,deterLimit);
    }
}

void BpbaTree::getKeepLeaves(Node* orig, size_t keepLimit)
{
    if(orig->mask >= keepLimit)
    {
        keepLeaves.insert(orig);
    }
    else
    {
        getKeepLeaves(orig->zero,keepLimit);
        getKeepLeaves(orig->one,keepLimit);
    }
}

BpbaTree::Node* BpbaTree::copyTree(Node* source, const RandomSet<Node*>& dtlv, const std::unordered_set<Node*>& kplv)
{
    Node* ans = new Node;
    ans->subnet = source->subnet;
    ans->mask = source->mask;
    if(source->mask < addrLen)
    {
        ans->zero = copyTree(source->zero,dtlv,kplv);
        ans->one = copyTree(source->one,dtlv,kplv);
        ans->zero->parent = ans;
        ans->one->parent = ans;
    }
    else
    {
        ans->zero = nullptr;
        ans->one = nullptr;
    }
    if(dtlv.find(source))
    {
        deterLeaves.insert(ans);
    }
    if(kplv.find(source) != kplv.end())
    {
        keepLeaves.insert(ans);
    }
    return ans;
}

void BpbaTree::clearTree(Node* orig)
{
    deterLeaves.remove(orig);
    keepLeaves.erase(orig);
    if(orig->mask < addrLen)
    {
        clearTree(orig->zero);
        clearTree(orig->one);
    }
    else
    {
        elems.erase(orig->subnet);
        --size;
    }
    delete orig;
}

void BpbaTree::freeTree(Node* orig)
{
    if(orig->mask < addrLen)
    {
        clearTree(orig->zero);
        clearTree(orig->one);
    }
    delete orig;
}

void BpbaTree::elemsProbaReq(const Node* node, double proba, std::unordered_map<boost::dynamic_bitset<>,double>& ans) const
{
    if(node->mask == addrLen)
    {
        ans.emplace(node->subnet,proba);
    }
    else
    {
        if(node->mask >= deterThld)
        {
            elemsProbaReq(node->zero,proba/2.0,ans);
            elemsProbaReq(node->one,proba/2.0,ans);
        }
        else
        {
            elemsProbaReq(node->zero,proba,ans);
            elemsProbaReq(node->one,proba,ans);
        }
    }
}

void BpbaTree::elemsKeepReq(const Node* node, double proba, std::unordered_map<boost::dynamic_bitset<>,double>& ans) const
{
    if(node->mask == addrLen)
    {
        ans.emplace(node->subnet,proba);
    }
    else
    {
        if(node->mask >= keepThld)
        {
            elemsKeepReq(node->zero,proba/2.0,ans);
            elemsKeepReq(node->one,proba/2.0,ans);
        }
        else
        {
            elemsKeepReq(node->zero,proba,ans);
            elemsKeepReq(node->one,proba,ans);
        }
    }
}
