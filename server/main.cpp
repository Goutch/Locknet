#include <mutex>
#include <string>
#include "iostream"
#include "Server.h"
#include "queue"

using namespace locknet;

std::queue<std::string> inputQueue;
std::mutex queueMutex;
Server *server;

void inputThread() {
	std::string input;
	while (true) {
		std::getline(std::cin, input);
		queueMutex.lock();
		inputQueue.push(input);
		queueMutex.unlock();
		if (input == "stop") {
			break;
		}
	}
}

void on_receive(void *data, int client_id) {
	//printf("Application: Received custom packet from client#%i:%s", client_id, (char *) data);
}

int main() {
	ServerInfo server_info{};
	server_info.max_channels = 4;
	server_info.port = 1234;
	server_info.max_clients = 3;


	std::thread inputThreadObj(inputThread);
	server = new Server(server_info);

	server->on_receive_event.subscribe(on_receive);
	server->start();
	while (true) {
		if (queueMutex.try_lock()) {
			if (inputQueue.empty()) {
				queueMutex.unlock();
			} else {
				std::string input = inputQueue.front();
				inputQueue.pop();
				queueMutex.unlock();
				if (input == "stop") {
					break;
				}
				if (input[0] == '-' && input[1] == 'k') {
					server->disconnectClient(std::stoi(input.substr(3, 1)), DISCONNECT_INFO::DISCONNECT_INFO_KICKED);
				}
			}
		}
		server->pollEvents(2);
	}
	server->stop();
	inputThreadObj.join();
	delete server;
	return 0;
}
