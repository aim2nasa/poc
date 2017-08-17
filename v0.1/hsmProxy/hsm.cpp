// hsm.cpp 
#include <node.h>
#include "CHsmProxy.h"
#include <string.h>

namespace demo {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Value;
using v8::String;
using v8::Local;
using v8::Object;

CHsmProxy hsm;

const char* ToCString(const String::Utf8Value& value)
{
  return *value ? *value : "<string conversion failed>";
}

void Init(const FunctionCallbackInfo<Value>& args)
{
  printf("Init\n");
  Isolate* isolate = args.GetIsolate();

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
  args.GetReturnValue().Set((uint32_t)hsm.slotID());
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "init", Init);
  NODE_SET_METHOD(exports, "init2", Init2);
  NODE_SET_METHOD(exports, "slotId", slotId);
}

NODE_MODULE(addon, init)

}  // namespace
