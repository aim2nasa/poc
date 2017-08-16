// hsm.cpp 
#include <node.h>
#include "CHsmProxy.h"

namespace demo {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Value;
using v8::String;
using v8::Local;
using v8::Object;

CHsmProxy hsm;

void Init(const FunctionCallbackInfo<Value>& args) {
  printf("Init\n");
  Isolate* isolate = args.GetIsolate();

  String::Utf8Value cmd(args[0]);
  std::string str = std::string(*cmd);
  int nRtn = hsm.init(str.c_str());

  args.GetReturnValue().Set(nRtn);
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "init", Init);
}

NODE_MODULE(addon, init)

}  // namespace
