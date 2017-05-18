#include "CCtrlProxy.h"

CCtrlProxy::CCtrlProxy()
{

}

int CCtrlProxy::open(void *)
{
	ACE_TRACE("CCtrlProxy::open");
	ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::open\n"));
	return 0;
}

int CCtrlProxy::handle_input(ACE_HANDLE handle)
{
	ACE_TRACE("CCtrlProxy::handle_input");
	ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::handle_input\n"));
	return 0;
}

int CCtrlProxy::handle_output(ACE_HANDLE handle)
{
	ACE_TRACE("CCtrlProxy::handle_output");
	ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::handle_output\n"));
	return 0;
}

int CCtrlProxy::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
	ACE_TRACE("CCtrlProxy::handle_close");
	ACE_DEBUG((LM_INFO, "(%t) CCtrlProxy::handle_close\n"));
	return 0;
}