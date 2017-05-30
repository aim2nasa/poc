#include "CGwData.h"

CGwData* CGwData::gwData_ = NULL;

CGwData::CGwData()
:hGw_(CK_INVALID_HANDLE), token_(NULL), ctrlProxy_(NULL)
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