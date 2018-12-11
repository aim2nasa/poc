#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

template<class T>
class monitor
{
public:
    template<typename ...Args>
    monitor(Args&&... args) : m_cl(std::forward<Args>(args)...){}

    struct monitor_helper
    {
        monitor_helper(monitor* mon) : m_mon(mon), m_ul(mon->m_lock) {}
        T* operator->() { return &m_mon->m_cl;}
        monitor* m_mon;
        std::unique_lock<std::mutex> m_ul;
    };

    monitor_helper operator->() { return monitor_helper(this); }
    monitor_helper ManuallyLock() { return monitor_helper(this); }
    T& GetThreadUnsafeAccess() { return m_cl; }

private:
    T           m_cl;
    std::mutex  m_lock;
};

int main(int argc, char *argv[])
{
    monitor<std::vector<int>> threadSafeVector {5};

    threadSafeVector->push_back(0);
    threadSafeVector->push_back(1);
    threadSafeVector->push_back(2);

    // Create a bunch of threads that hammer the vector
    std::vector<std::thread> threads;
    for(int i=0; i<16; ++i)
    {
        threads.push_back(std::thread([&]()
        {
            for(int i=0; i<1024; ++i)
            {
                threadSafeVector->push_back(i);
            }
        }));
    }

    // You can explicitely take a lock then call multiple functions
    // without the overhead of a relock each time. The 'lock handle'
    // destructor will unlock the lock correctly. This is necessary
    // if you want a chain of logically connected operations 
    {
        auto lockedHandle = threadSafeVector.ManuallyLock();
        if(!lockedHandle->empty())
        {
            lockedHandle->pop_back();
            lockedHandle->push_back(-3);
        }
    }

    for(auto& t : threads)
    {
        t.join();
    }

    // And finally access the underlying object in a raw fashion without a lock
    // Use with Caution!

    std::vector<int>& rawVector = threadSafeVector.GetThreadUnsafeAccess();
    rawVector.push_back(555);

    // Should be 16393 (5+3+16*1024+1)
    std::cout << threadSafeVector->size() << std::endl;

    std::cout<<"end of main"<<std::endl;
    return 0;
}
