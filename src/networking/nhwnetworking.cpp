#include "nhwnetworking.h"

RecieveMessageCallback networkCallback;

void SetMessageCallbackFunction(RecieveMessageCallback callback)
{
	networkCallback = callback;
}

void MessageCallback(const char* message, uint32_t size)
{
	networkCallback(message, size);
}
