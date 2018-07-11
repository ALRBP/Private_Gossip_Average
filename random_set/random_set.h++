
#include <cstddef>
#include <chrono>
#include <random>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#ifndef RAND_SET
template<class T>
class RandomSet
{
public:
    using const_iterator = typename std::vector<T>::const_iterator;
    
private:
    std::vector<T> idxd;
    
    std::unordered_map<T,size_t> idxs;
    
public:
    RandomSet(void)
    {
        return;
    }
    
    RandomSet(const RandomSet<T>& s)
    {
        idxd = s.idxd;
        idxs = s.idxs;
    }
    
    RandomSet(const std::unordered_set<T>& s)
    {
        idxd.reserve(s.size());
        idxs.reserve(s.size());
        size_t i = 0;
        for(auto& x : s)
        {
            idxd[i] = x;
            idxs.insert(x,i);
            ++i;
        }
    }
    
    ~RandomSet(void)
    {
        return;
    }

    RandomSet<T>& operator=(const RandomSet<T>& s)
    {
        idxd = s.idxd;
        idxs = s.idxs;
        return *this;
    }
    
    const_iterator begin(void) const noexcept
    {
        return idxd.cbegin();
    }
    
    const_iterator end(void) const noexcept
    {
        return idxd.cend();
    }
    
    void insert(const T& x)
    {
        if(idxs.find(x) == idxs.end())
        {
            idxs.emplace(x,idxd.size());
            idxd.push_back(x);
        }
    }
    
    void remove(const T& x)
    {
        if(idxs.find(x) != idxs.end())
        {
            idxd[idxs[x]] = idxd.back();
            idxs[idxd.back()] = idxs[x];
            idxd.pop_back();
            idxs.erase(x);
        }
    }
    
    bool find(const T& x) const
    {
        return (idxs.find(x) != idxs.end());
    }
    
    bool isEmpty(void) const noexcept
    {
        return idxd.empty();
    }
    
    size_t size(void) const noexcept
    {
        return idxd.size();
    }
    
    size_t capacity(void) const noexcept
    {
        return idxd.capacity();
    }
    
    void reserve(size_t s)
    {
        idxd.reserve(s);
        idxs.reserve(s);
    }
    
    void clear(void)
    {
        idxd.clear();
        idxs.clear();
    }
    
    T rand(void) const
    {
        if(idxd.empty())
        {
            return T();
        }
        std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<size_t> randGen(0,idxd.size()-1);
        return idxd[randGen(gen)];
    }
    
    std::unordered_set<T> randSet(size_t s) const
    {
        std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<size_t> randGen(0,idxd.size()-1);
        if(s >= idxd.size())
        {
            return std::unordered_set<T>(idxd.begin(),idxd.end());
        }
        std::unordered_set<T> ans;
        ans.reserve(s);
        if(s > idxd.size()/2)
        {
            std::unordered_set<size_t> ids;
            ids.reserve(idxd.size()-s);
            while(ids.size() < (idxd.size()-s))
            {
                ids.insert(randGen(gen));
            }
            for(size_t i = 0; i < idxd.size(); ++i)
            {
                if(ids.find(i) == ids.end())
                {
                    ans.insert(idxd[i]);
                }
            }
        }
        else
        {
            ans.reserve(s);
            while(ans.size() < s)
            {
                ans.insert(idxd[randGen(gen)]);
            }
        }
        return ans;
    }
    
    std::unordered_set<T> toSet(void) const
    {
        std::unordered_set<T> ans(idxs.size());
        for(auto& x : idxs)
        {
            ans.insert(x.first);
        }
        return ans;
    }
};
#define RAND_SET
#endif
