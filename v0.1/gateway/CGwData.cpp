#include "CGwData.h"

CGwData* CGwData::gwData_ = NULL;

CGwData::CGwData()
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