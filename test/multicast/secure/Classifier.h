#ifndef __CLASSIFIER_H__
#define __CLASSIFIER_H__

#include <vector>
#include "messageCount.h"
#include <entity/node.h>

#define MAX_QUEUE 5

struct vcRtn{
    vcRtn():visitCount(-1),order(-1){}
    int visitCount;
    int order;
};

class IDetect;

class Classifier{
public:
	Classifier();
	~Classifier();

    enum kind{
        initErr=-2,
        unexpected=-1,
        verified=0,
        sequence=1,
        replay=2,
        fraud=3,
        unAuthPub=4
    };

    static const char* errToMsg(kind askRtn);
    void init(int tagSize,std::string adata,int keySize,int key,int iv,IDetect *p=NULL);
    kind ask(const char* buf, size_t size);
    std::vector<messageCount> q_;

private:
    void setKeys(int size,int key,int iv);
    void setDetect(IDetect *p);
    vcRtn getVisitCount(std::vector<messageCount>& q,const char *buff,unsigned int buffSize);
    int getFrameNumDiff(unsigned int frameNumber);

    int tagSize_;
    std::string adata_;
    CryptoPP::GCM<CryptoPP::AES>::Decryption d_;
    Node Bob_;
    IDetect *detect_;
    bool prevFrameDefined_;
    unsigned int prevFrameNumber_;
};

#endif
