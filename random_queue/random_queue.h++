
#include <cstddef>
#include <chrono>
#include <random>
#include <queue>
#include "../random_set/random_set.h++"

#ifndef RAND_QUEUE
template<class T>
class RandomQueue
{
public:
    using const_iterator = typename RandomSet<T>::const_iterator;
    
private:
    size_t ms;
    
    RandomSet<T> elems;
    
    std::queue<T> q;
    
public:
    RandomQueue(void)
    {
        ms = 0;
        return;
    }
    
    RandomQueue(const RandomQueue<T>& s)
    {
        ms = s.ms;
        elems = s.elems;
        q = s.q;
    }
    
    RandomQueue(size_t s)
    {
        ms = s;
    }
    
    ~RandomQueue(void)
    {
        return;
    }

    RandomQueue<T>& operator=(const RandomQueue<T>& s)
    {
        ms = s.ms;
        elems = s.elems;
        q = s.q;
        return *this;
    }
    
    const_iterator begin(void) const noexcept
    {
        return elems.begin();
    }
    
    const_iterator end(void) const noexcept
    {
        return elems.end();
    }
    
    void insert(const T& x)
    {
        if(!elems.find(x))
        {
            elems.insert(x);
            q.push(x);
            if(q.size() > ms)
            {
                elems.remove(q.front());
                q.pop();
            }
        }
        else
        {
            std::queue<T> tq;
            while(q.size() > 0)
            {
                if(q.front() != x)
                {
                    tq.push(q.front());
                }
                q.pop();
                
            }
            q = tq;
            q.push(x);
        }
            
    }
    
    void remove(const T& x)
    {
        elems.remove(x);
        std::queue<T> tq;
        while(q.size() > 0)
        {
            if(q.front() != x)
            {
                tq.push(q.front());
            }
            q.pop();
            
        }
        q = tq;
    }
    
    bool find(const T& x) const
    {
        return elems.find();
    }
    
    bool isEmpty(void) const noexcept
    {
        return elems.isEmpty();
    }
    
    size_t size(void) const noexcept
    {
        return elems.size();
    }
    
    size_t maxSize(void) const noexcept
    {
        return ms;
    }
    
    void clear(void)
    {
        elems.clear();
        q = std::queue<T>();
    }
    
    T rand(void) const
    {
        return elems.rand();
    }
    
    std::unordered_set<T> randSet(size_t s) const
    {
        return elems.randSet(s);
    }
    
    std::unordered_set<T> toSet(void) const
    {
        return elems.toSet();
    }
};
#define RAND_QUEUE
#endif
