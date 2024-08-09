#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
	void CreateServer(uint16_t port);
	void ConnectToServer(const char* ip);
	unsigned char* RecieveAsClient();
	unsigned char* RecieveAsServer();
	void SendToServer(unsigned char* message);
	void SendToClient(unsigned char* message);
	void UpdateServer();
	void UpdateClient();

#ifdef __cplusplus
}
#endif