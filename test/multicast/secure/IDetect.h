#ifndef __IDETECT_H__
#define __IDETECT_H__

class IDetect{
public:
    virtual ~IDetect(){}
    virtual void onUnauthorizedPubData(const char *data,size_t size,const char* errMsg)=0;
    virtual void onFraudData(const char *data,size_t size)=0;
    virtual void onReplayData(const char *data,size_t size,int visitCount,int orfer,size_t qsize)=0;
    virtual void onWrongSequenceData(const char *data,size_t size,unsigned int frameNumber,int frameNumDiff)=0;
    virtual void onVerifiedData(const char *data,size_t size,unsigned int frameNumber)=0;
};

#endif
