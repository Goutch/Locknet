#include "LockNet.h"
#include "string"

using namespace LockNet;

void startClient() {
	ClientInfo clientInfo{};
	Client client(clientInfo);
	if (client.connect(5000)) {
		while (true) {
			client.pollEvents();
		}
	}
}

void startServer() {
	Server server(ServerInfo{1234, 32, 2});
	server.start();

	while (true) {
		server.pollEvents();
	}
}

int main() {
	startClient();
	return 0;
}
