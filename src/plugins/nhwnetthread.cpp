#include "nhwnetthread.h"
#include <thread>
#include <vector>

bool shouldRunNetthread = true;
std::thread thread;
std::vector<void*> clientStorage;


void NetworkingThread() {
	while (shouldRunNetthread) {
		void* client = ConnectClients();
		if (client) {
			SendMessage(client, "Hello from the server");
			clientStorage.push_back(client);
		};
	};
};
void CreateNetworkThread()
{
	thread = std::thread(NetworkingThread);
}

void DestroyNetworkThread()
{
	shouldRunNetthread = false;
	thread.join();
}

void GetAllClients(uint32_t* amount, void** clients)
{
	*amount = clientStorage.size();
	if (clients==nullptr) {
		return;
	}
	*clients = clientStorage.data();
}
