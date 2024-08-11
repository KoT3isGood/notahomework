#pragma once
#include "../networking/nhwnetworking.h"

/*
Networking thread Plugin
Description:
This plugin will create a thread that will create a loop which waits for calls

Adds:
	CreateNetworkThread()
	DestroyNetworkThread()
*/
#ifdef __cplusplus
extern "C" {
#endif

	void CreateNetworkThread();
	void DestroyNetworkThread();
	void GetAllClients(uint32_t* amount, void** clients);

#ifdef __cplusplus
}
#endif