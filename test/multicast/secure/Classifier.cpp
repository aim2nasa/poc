#include "Classifier.h"
#include <string.h>
#include "IDetect.h"

Classifier::Classifier()
:prevFrameDefined_(false),prevFrameNumber_(0),tagSize_(0),detect_(NULL)
{
}

Classifier::~Classifier()
{
}

vcRtn Classifier::getVisitCount(std::vector<messageCount>& q,const char *buff,unsigned int buffSize)
{
    int i=1;
    vcRtn v;
    for(std::vector<messageCount>::iterator it = q.begin(); it != q.end(); ++it){
        if(memcmp((*it).body,buff,buffSize)==0) {
            v.visitCount = (*it).visitCount;
            v.order = i;
            (*it).visitCount++;
            return v;
        }
        i++;
    }
    return v;
}

int Classifier::getFrameNumDiff(unsigned int frameNumber)
{
    if(!prevFrameDefined_) return 1;
    return (frameNumber - prevFrameNumber_);
}

void Classifier::setKeys(int size,int key,int iv)
{
    Bob_.size_ = size;
    Bob_.key_ = new byte[Bob_.size_];
    memset(Bob_.key_,key,Bob_.size_);
    memset(Bob_.iv_,iv,CryptoPP::AES::BLOCKSIZE);
}

void Classifier::setDetect(IDetect *p)
{
    detect_ = p;
}

const char* Classifier::errToMsg(kind askRtn)
{
    const char *pRtn= 0;
    switch(askRtn){
    case initErr:
        pRtn = "init error";
        break;
    case unexpected:
        pRtn = "uexpected error";
        break;
    case verified:
        pRtn = "verified";
        break;
    case sequence:
        pRtn = "sequence warning";
        break;
    case replay:
        pRtn = "replay attack";
        break;
    case fraud:
        pRtn = "fraud attack";
        break;
    case unAuthPub:
        pRtn = "unauthorized publishing attack";
        break;
    default:
        break;
    }
    return pRtn;
}

void Classifier::init(int tagSize,std::string adata,int keySize,int key,int iv,IDetect *p)
{
    tagSize_ = tagSize;
    adata_ = adata;
    setKeys(keySize,key,iv);
    setDetect(p);
    d_.SetKeyWithIV(Bob_.key_,Bob_.size_,Bob_.iv_);
}

Classifier::kind Classifier::ask(const char* buf, size_t size)
{
    if(Bob_.size_==0) return initErr;

    int rtn;
    std::string recoveredText;
    if((rtn=Bob_.decrypt(d_,tagSize_,adata_,std::string(buf,size),recoveredText))!=DECRYPT_OK){
        if(detect_) detect_->onUnauthorizedPubData(std::string(buf,size).c_str(),size,Node::errToStr(rtn).c_str());
        return unAuthPub;
    }else{
        vcRtn v = getVisitCount(q_,buf,size);
        unsigned int frameNumber;
        memcpy(&frameNumber,recoveredText.c_str(),sizeof(frameNumber));

        if(v.visitCount<0) {
            if(detect_) detect_->onFraudData(buf,size);
            return fraud;
        }else if(v.visitCount>0){
            if(detect_) detect_->onReplayData(buf,size,v.visitCount,v.order,q_.size());
            return replay;
        }else{
            assert(v.visitCount==0);

            int frameNumDiff = getFrameNumDiff(frameNumber);
            prevFrameNumber_ = frameNumber;
            prevFrameDefined_ = true;

            if(frameNumDiff!=1) {
                if(detect_) detect_->onWrongSequenceData(buf,size,frameNumber,frameNumDiff);
                return sequence;
            }else{
                if(detect_) detect_->onVerifiedData(buf,size,frameNumber);
                return verified;
            }
        }
    }
    return unexpected;
}
