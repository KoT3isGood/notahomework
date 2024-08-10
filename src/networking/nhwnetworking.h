#pragma once
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif
	void CreateServer(uint16_t port);

	// Returns pointer to accepted client, only 1 per iteration
	void* ConnectClients();

	// Sends message to the client
    void SendMessage(void* client, const char* message);

	typedef void(*RecieveMessageCallback)(const char*, uint32_t size);
	void SetMessageCallbackFunction(RecieveMessageCallback callback);

	// Server calls will call message set in SetMessageCallbackFunction
	void MessageCallback(const char* message, uint32_t size);

#ifdef __cplusplus
}
#endif