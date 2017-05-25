#include "CGwData.h"

CGwData* CGwData::gwData_ = NULL;

CGwData::CGwData()
:hGw_(CK_INVALID_HANDLE)
{

}

CGwData* CGwData::getInstance()
{
	if (gwData_ == NULL) gwData_ = new CGwData();
	return gwData_;
}

void CGwData::delInstance()
{
	delete gwData_;
	gwData_ = NULL;
}