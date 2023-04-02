//
// Created by User on 3/6/2023.
//

#include "Client.h"
#include "enet/enet.h"
#include "stdio.h"

namespace LockNet {
	Client::Client(const ClientInfo &info) {
		this->info = info;
		if (enet_initialize() != 0) {
			fprintf(stderr, "An error occurred while initializing ENet.\n");
		}
	}

	Client::~Client() {
		enet_host_destroy(enet_handle);
		atexit(enet_deinitialize);
	}

	bool Client::connect(int timeout_ms) {
		enet_handle = enet_host_create(NULL /* create a client host */,
									   1 /* only allow 1 outgoing connection */,
									   info.max_channels /* allow up 2 channels to be used, 0 and 1 */,
									   0 /* assume any amount of incoming bandwidth */,
									   0 /* assume any amount of outgoing bandwidth */);
		if (enet_handle == NULL) {
			fprintf(stderr,
					"An error occurred while trying to create an ENet client host.\n");
			exit(EXIT_FAILURE);
		}

		ENetAddress address;
		ENetEvent event;
		enet_address_set_host(&address, info.server_ip.c_str());
		address.port = info.server_port;
		server_handle = enet_host_connect(enet_handle, &address, info.max_channels, 0);

		if (server_handle == NULL) {
			fprintf(stderr,
					"No available peers for initiating an ENet connection.\n");
			return false;
		}
		if (enet_host_service(enet_handle, &event, timeout_ms) > 0 &&
			event.type == ENET_EVENT_TYPE_CONNECT) {
			printf("Connection to %s:%i succeeded.\n", info.server_ip.c_str(), info.server_port);
			return true;
		} else {
			/* Either the 5 seconds are up or a disconnect event was */
			/* received. Reset the peer in the event the 5 seconds   */
			/* had run out without any significant event.            */
			enet_peer_reset(server_handle);
			printf("Connection to %s failed.", info.server_ip.c_str());
			return false;
		}
	}

	void Client::pollEvents() {
		ENetEvent event;
		while (enet_host_service(enet_handle, &event, 10) > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					printf("error: received a connection event from %d.%d.%d.%d:%u.\n",
						   event.peer->address.host & 0xFF,
						   (event.peer->address.host >> 8) & 0xFF,
						   (event.peer->address.host >> 16) & 0xFF,
						   (event.peer->address.host >> 24) & 0xFF,
						   event.peer->address.port);
					break;
				case ENET_EVENT_TYPE_RECEIVE:
					printf("A packet of length %u containing %s was received from %d.%d.%d.%d on channel %u.\n",
						   (unsigned int) event.packet->dataLength,
						   (unsigned char*) event.packet->data,
						   event.peer->address.host & 0xFF,
						   (event.peer->address.host >> 8) & 0xFF,
						   (event.peer->address.host >> 16) & 0xFF,
						   (event.peer->address.host >> 24) & 0xFF,
						   //(char *) event.peer->data,
						   event.channelID);
					enet_packet_destroy(event.packet);

					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					printf("%s disconnected.\n", (char *) event.peer->data);
					/* Reset the peer's client information. */
					event.peer->data = NULL;
			}
		}
	}

	bool Client::send(const PacketInfo &packet_info) {
		ENetPacket *packet = enet_packet_create(packet_info.data, packet_info.lenght, packet_info.mode);
		if (enet_peer_send(server_handle, packet_info.channel, packet) < 0) {
			printf("error: failed to send packet to server.\n");
			enet_packet_destroy(packet);
			return false;
		}
		return true;
	}

}

