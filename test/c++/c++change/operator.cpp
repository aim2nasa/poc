#include <iostream>
#include <vector>

using namespace std;

//Want to know the behaviour of operator -> changed as c++ compiler changed from c++ to c++11 
//Conclusion: the behaviour is not changed

template<class T>
class monitor
{
public:
    monitor()
    {
        cout<<"monitor()"<<endl;
    }

    struct monitor_helper
    {
        monitor_helper(monitor* mon) : m_mon(mon)
        {
            cout<<"monitor_helper()"<<endl;
        }
        T* operator->() { 
            cout<<"T* operator->()"<<endl;
            return &m_mon->m_cl;
        }
        monitor* m_mon;
    };

    monitor_helper operator->() {
        cout<<"monitor_helper operator->()"<<endl;
        return monitor_helper(this);
    }
    monitor_helper ManuallyLock() {
        cout<<"ManuallyLock()"<<endl;
        return monitor_helper(this);
    }
    T& GetThreadUnsafeAccess() {
        cout<<"GetThreadUnsafeAccess()"<<endl;
        return m_cl;
    }

private:
    T           m_cl;
};

int main(int argc, char *argv[])
{
    monitor< std::vector<int> > threadSafeVector;

    threadSafeVector->push_back(0);

    std::cout<<"end of main"<<std::endl;
    return 0;
}
