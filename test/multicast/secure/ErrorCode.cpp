#include "ErrorCode.h"

const char* errToMsg(int code)
{
    switch(code){
    case OK:
        return "OK";    
    case ERROR_SOCKET:
        return "ERROR_SOCKET";    
    case ERROR_BIND:
        return "ERROR_BIND";    
    case ERROR_LISTEN:
        return "ERROR_LISTEN";    
    case ERROR_RECEIVE:
        return "ERROR_RECEIVE";    
    case ERROR_CONNECT:
        return "ERROR_CONNECT";    
    default:
        return "UNDEFINED";
    }
}
