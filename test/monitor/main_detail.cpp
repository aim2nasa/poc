#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

template<class T>
class monitor
{
public:
    template<typename ...Args>
    monitor(Args&&... args) : m_cl(std::forward<Args>(args)...)
    {
        std::cout<<"monitor()"<<std::endl;
    }

    ~monitor()
    {
        std::cout<<"~monitor()"<<std::endl;
    }

    struct monitor_helper
    {
        monitor_helper(monitor* mon) : m_mon(mon), m_ul(mon->m_lock)
        {
            std::cout<<"monitor_helper()"<<std::endl;
        }
        T* operator->()
        {
            std::cout<<"T* operator->"<<std::endl;
            return &m_mon->m_cl;
        }
        monitor* m_mon;
        std::unique_lock<std::mutex> m_ul;
    };

    monitor_helper operator->()
    {
        std::cout<<"monitor_helper operator->"<<std::endl;
        return monitor_helper(this);
    }
    monitor_helper ManuallyLock()
    {
        std::cout<<"ManuallyLock()"<<std::endl;
        return monitor_helper(this);
    }
    T& GetThreadUnsafeAccess()
    {
        std::cout<<"GetThreadUnsafeAccess()"<<std::endl;
        return m_cl;
    }

private:
    T           m_cl;
    std::mutex  m_lock;
};

int main(int argc, char *argv[])
{
    monitor<std::vector<int>> threadSafeVector {5};

    threadSafeVector->push_back(0);

    std::cout<<"end of main"<<std::endl;
    return 0;
}
