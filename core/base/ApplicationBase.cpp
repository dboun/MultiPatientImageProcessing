#include "ApplicationBase.h"

void ApplicationBase::SetUid(long uid)
{
	this->uid = uid;
}

long ApplicationBase::GetUid()
{
	return uid;
}

void ApplicationBase::EmitProgressUpdateForDebugging(int progress)
{
	emit ProgressUpdateUI(uid, QString("Debugging"), progress);
}