#ifndef __COLLECTOR_H__
#define __COLLECTOR_H__

class Collector{
public:
	Collector();
	~Collector();

    int init(const char* ip,int port);
    int collect(const void* buf,unsigned int len,int flags=0);

    int sock_;
};

#endif
