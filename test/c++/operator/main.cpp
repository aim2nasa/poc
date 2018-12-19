#include <iostream>

class CTest{
public:
    void test() {
        std::cout<<"CT::test"<<std::endl;
    }
};

class mon
{
public:
    struct helper
    {
        helper(mon *mon):m_mon(mon){}

        CTest* operator->() {
            std::cout<<"helper::->"<<std::endl;
            return &m_mon->m_ct;
        }
        CTest* test() {
            std::cout<<"helper::test"<<std::endl;
            return &m_mon->m_ct;
        }

        mon *m_mon;
    };

    helper operator->() {
        std::cout<<"mon::->"<<std::endl;
        return helper(this);
    }

    helper test() {
        std::cout<<"mon::test"<<std::endl;
        return helper(this);
    }

private:
    CTest m_ct;
};

int main(int argc, char *argv[])
{
    mon m;

    std::cout<<"1.Test using operator->()"<<std::endl;
    m->test();

    std::cout<<"2.Test using test()"<<std::endl;
    m.test();

    std::cout<<"3. To be identical calling with #1(to reach to CTest::test() methond)"<<std::endl;
    m.test().test()->test();

    std::cout<<"end of main"<<std::endl;
    return 0;
}
