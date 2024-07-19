#include "nhw.h"

NhwInstanceInfo nhwInstanceInfo = {};

void nhwCreateInstance(NhwInstanceInfo createInfo)
{
	nhwInstanceInfo = createInfo;
}

void nhwDestroyInstance()
{
}

void nhwLog(const char* msg)
{
	if (nhwInstanceInfo.logCallback) {
		nhwInstanceInfo.logCallback(msg);
	}
}

void nhwResetLog()
{
	if (nhwInstanceInfo.logResetCallback) {
		nhwInstanceInfo.logResetCallback();
	}
}
