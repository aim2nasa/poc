// hsm.cpp 
#include <node.h>
#include <node_buffer.h>
#include "CHsmProxy.h"
#include <string.h>
#include <nan.h>

namespace demo {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Value;
using v8::String;
using v8::Local;
using v8::Object;

CHsmProxy hsm;
unsigned long hKey = 0;
unsigned long encryptedDataLen = 0;
unsigned long gDataLen = 0;
std::vector<unsigned char> encData;
std::vector<unsigned char> gDecData;

const char* ToCString(const String::Utf8Value& value)
{
  return *value ? *value : "<string conversion failed>";
}

void Init(const FunctionCallbackInfo<Value>& args)
{
  printf("Init\n");
  String::Utf8Value cmd(args[0]);
  std::string str = std::string(*cmd);
  int nRtn = hsm.init(str.c_str());

  args.GetReturnValue().Set(nRtn);
}

void Init2(const FunctionCallbackInfo<Value>& args)
{
  printf("Init2\n");
  String::Utf8Value strLabel(args[0]);
  const char* label = ToCString(strLabel);

  String::Utf8Value strSoPin(args[1]);
  const char* soPin = ToCString(strSoPin);

  String::Utf8Value strUserPin(args[2]);
  const char* userPin = ToCString(strUserPin);

  String::Utf8Value strEmptySlot(args[3]);
  const char* emptySlot = ToCString(strEmptySlot);

  bool bEmptySlot = true;
  if(strcmp(emptySlot,"false")==0) bEmptySlot=false;

  int nRtn = hsm.init(label,soPin,userPin,bEmptySlot);
  args.GetReturnValue().Set(nRtn);
}

void slotId(const FunctionCallbackInfo<Value>& args)
{
  printf("slotId\n");
  args.GetReturnValue().Set((uint32_t)hsm.slotID());
}

void findKey(const FunctionCallbackInfo<Value>& args)
{
  String::Utf8Value strLabel(args[0]);
  const char* label = ToCString(strLabel);

  unsigned int labelSize = args[1]->NumberValue();

  int nRtn = hsm.findKey(label,labelSize,hKey);
  args.GetReturnValue().Set(nRtn);
}

void getFoundKey(const FunctionCallbackInfo<Value>& args)
{
  args.GetReturnValue().Set((uint32_t)hKey);
}

void setEnv(const FunctionCallbackInfo<Value>& args)
{
  String::Utf8Value strName(args[0]);
  const char* name = ToCString(strName);

  String::Utf8Value strValue(args[1]);
  const char* value = ToCString(strValue);

  int overwrite = args[2]->NumberValue();

  int nRtn = hsm.setenv(name,value,overwrite);
  args.GetReturnValue().Set(nRtn);
}

void encryptInit(const FunctionCallbackInfo<Value>& args)
{
  String::Utf8Value strMechType(args[0]);
  const char* mt = ToCString(strMechType);

  unsigned long hArgKey = args[1]->NumberValue();

  int nRtn;
  if(strcmp(mt,"AES_CBC_PAD")==0)
    nRtn = hsm.encryptInit(CHsmProxy::AES_CBC_PAD,hArgKey);
  else if(strcmp(mt,"AES_CBC")==0)
    nRtn = hsm.encryptInit(CHsmProxy::AES_CBC,hArgKey);
  else if(strcmp(mt,"AES_ECB")==0)
    nRtn = hsm.encryptInit(CHsmProxy::AES_ECB,hArgKey);
  else{
    args.GetReturnValue().Set(-1);
    return;
  }

  args.GetReturnValue().Set(nRtn);
}

void encrypt(const FunctionCallbackInfo<Value>& args) {
  unsigned long dataLen = args[1]->NumberValue();
  Local<Object> bufferObj = args[0]->ToObject();
  char *data = node::Buffer::Data(bufferObj);

  printf("* 1.encrypt: data:%s,dataLen=%lu\n",data,dataLen);

  int nRtn;
  nRtn = hsm.encrypt((unsigned char*)data,dataLen,NULL,&encryptedDataLen);
  printf("* 2.encrypt: nRtn=%d,encryptedDataLen=%lu\n",nRtn,encryptedDataLen);

  encData.resize(encryptedDataLen);
  nRtn = hsm.encrypt((unsigned char*)data,dataLen,&encData.front(),&encryptedDataLen);
  if(nRtn!=0) args.GetReturnValue().Set(nRtn);
  printf("* 3.encrypt: nRtn=%d\n",nRtn);

  printf("* 4.encrypt: encData.data = %s, encData.Size=%d\n",(char*)encData.data(),encData.size());

  //Nan 버퍼 사용
  args.GetReturnValue().Set(Nan::NewBuffer((char*)encData.data(),encData.size()).ToLocalChecked());
  printf("* 5.encrypt: encData.data = %s, encData.Size=%d\n",(char*)encData.data(),encData.size());
}

void decryptInit(const FunctionCallbackInfo<Value>& args)
{
  String::Utf8Value strMechType(args[0]);
  const char* mt = ToCString(strMechType);

  unsigned long hArgKey = args[1]->NumberValue();

  int nRtn;
  if(strcmp(mt,"AES_CBC_PAD")==0)
    nRtn = hsm.decryptInit(CHsmProxy::AES_CBC_PAD,hArgKey);
  else if(strcmp(mt,"AES_CBC")==0)
    nRtn = hsm.decryptInit(CHsmProxy::AES_CBC,hArgKey);
  else if(strcmp(mt,"AES_ECB")==0)
    nRtn = hsm.decryptInit(CHsmProxy::AES_ECB,hArgKey);
  else{
    args.GetReturnValue().Set(-1);
    return;
  }

  args.GetReturnValue().Set(nRtn);
}

void decrypt(const FunctionCallbackInfo<Value>& args) {
  unsigned long encryptedDataLen = args[1]->NumberValue();
  Local<Object> bufferObj = args[0]->ToObject();
  char *encryptedData = node::Buffer::Data(bufferObj);

  printf("* 1.decrypt: encryptedData:%s,encryptedDataLen=%lu\n",encryptedData,encryptedDataLen);

  int nRtn;
  nRtn = hsm.decrypt((unsigned char*)encryptedData,encryptedDataLen,NULL,&gDataLen);
  printf("* 2.decrypt: nRtn=%d,DataLen=%lu\n",nRtn,gDataLen);

  gDecData.resize(gDataLen);
  nRtn = hsm.decrypt((unsigned char*)encryptedData,encryptedDataLen,&gDecData.front(),&gDataLen);
  if(nRtn!=0) args.GetReturnValue().Set(nRtn);
  printf("* 3.decrypt: nRtn=%d\n",nRtn);

  printf("* 4.decrypt: decData.data = %s, decData.Size=%d\n",(char*)gDecData.data(),gDecData.size());

  //Nan 버퍼 사용
  args.GetReturnValue().Set(Nan::NewBuffer((char*)gDecData.data(),gDecData.size()).ToLocalChecked());
  printf("* 5.decrypt: decData.data = %s, decData.Size=%d\n",(char*)gDecData.data(),gDecData.size());
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "init", Init);
  NODE_SET_METHOD(exports, "init2", Init2);
  NODE_SET_METHOD(exports, "slotId", slotId);
  NODE_SET_METHOD(exports, "findKey", findKey);
  NODE_SET_METHOD(exports, "getFoundKey", getFoundKey);
  NODE_SET_METHOD(exports, "setEnv", setEnv);
  NODE_SET_METHOD(exports, "encryptInit", encryptInit);
  NODE_SET_METHOD(exports, "encrypt", encrypt);
  NODE_SET_METHOD(exports, "decryptInit", decryptInit);
  NODE_SET_METHOD(exports, "decrypt", decrypt);
}

NODE_MODULE(addon, init)

}  // namespace
