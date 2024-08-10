#pragma once
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif
	void CreateServer(uint16_t port);

	// Returns pointer to accepted client, only 1 per iteration
	void* ConnectClients();

	// Sends message to the client
    void SendMessage(void* client);

#ifdef __cplusplus
}
#endif