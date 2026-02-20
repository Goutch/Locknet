#include <queue>
#include <mutex>
#include "iostream"
#include "Client.h"
#include <semaphore>
#include <thread>

using namespace locknet;
using namespace HBE;
std::queue<std::string> inputQueue;
std::mutex queueMutex;
std::mutex endThreadMutex;
bool endThread = false;
Client *client;

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
        endThreadMutex.lock();
        if (endThread) {
            endThreadMutex.unlock();
            break;
        }
        endThreadMutex.unlock();
    }
}

void on_receive(uint32_t client_id, void *data) {
    char *data_str = (char *) data;
    printf("From client#%i Received:%s\n", client_id, data_str);
}

void on_client_connect(uint32_t client_id) {
    printf("Client#%i connected\n", client_id);
}

void on_connection_success() {
    printf("Connection success as Client#%u!\n", client->getID());
}

void on_client_disconnected(uint32_t client_id) {
    printf("Client#%i disconnected\n", client_id);
}

void on_disconnected(DISCONNECT_INFO reason) {
    std::string reason_str;
    switch (reason) {
        case DISCONNECT_INFO::DISCONNECT_INFO_NONE:
            reason_str = "None";
            break;
        case DISCONNECT_INFO::DISCONNECT_INFO_SERVER_FULL:
            reason_str = "Server Full";
            break;
        case DISCONNECT_INFO::DISCONNECT_INFO_KICKED:
            reason_str = "Kicked";
            break;
        default:
            reason_str = "Unknown";
            break;
    }
    printf("Disconnected from server reason: %s \n", reason_str.c_str());

    endThreadMutex.lock();
    endThread = true;
    endThreadMutex.unlock();
}

int main() {
    std::thread inputThreadObj(inputThread);
    ClientInfo client_info{};

    client = new Client(client_info);

	event_subscription_id on_receive_event_id{};
	event_subscription_id on_client_connected_event_id{};
	event_subscription_id on_disconnect_event_id{};
	event_subscription_id on_connect_event_id{};
	event_subscription_id on_client_disonnected_event_id{};

	client->on_receive.subscribe(on_receive_event_id, on_receive);
	client->on_client_connected.subscribe(on_client_connected_event_id, on_client_connect);
	client->on_disconnect.subscribe(on_disconnect_event_id, on_disconnected);
	client->on_connect_success.subscribe(on_connect_event_id, on_connection_success);
	client->on_client_disonnected.subscribe(on_client_disonnected_event_id, on_client_disconnected);
    if (!client->connect(5000)) {
        std::cout << "Failed to connect to server!" << std::endl;
        return 0;
    }

    while (client->isConnected()) {
        if (queueMutex.try_lock()) {
            if (inputQueue.empty()) {
                queueMutex.unlock();
            } else {
                std::string input = inputQueue.front();
                inputQueue.pop();
                queueMutex.unlock();

                std::string message_str = input;
                const char *message = message_str.c_str();
                LocknetPacket packet;
                packet.channel = 0;
                packet.mode = PACKET_MODE_RELIABLE_ORDERED;
                packet.data = message;
                packet.length = input.size();
                packet.length += 1; //for null terminator
                client->broadcast(packet);
            }
        }
        client->pollEvents();
    }

    client->disconnect();
    inputThreadObj.join();
    delete client;
    return 0;
}
