
#include <string>
#include "Server.h"
#include "stdio.h"

namespace LockNet {
	Server::Server(const ServerInfo &info) {
		this->info = info;
	}

	void Server::start() {
		if (enet_initialize() != 0) {
			fprintf(stderr, "An error occurred while initializing ENet.\n");
		}
		ENetAddress address;
		address.host = ENET_HOST_ANY;
		address.port = info.port;
		enet_handle = enet_host_create(&address /* the address to bind the server host to */,
									   info.max_clients     /* allow up to 32 clients and/or outgoing connections */,
									   info.max_channels      /* allow up to 2 channels to be used, 0 and 1 */,
									   0      /* assume any amount of incoming bandwidth */,
									   0      /* assume any amount of outgoing bandwidth */);
		if (enet_handle == NULL) {
			fprintf(stderr, "An error occurred while trying to create an ENet server host.");
		}

		pollEvents();
	}

	int i = 0;
	std::string frame_counter = std::to_string(i);

	void Server::pollEvents() {
		ENetEvent event;
		i++;
		frame_counter = std::to_string(i);
		PacketInfo packet_info{};
		packet_info.mode = PACKET_MODE_RELIABLE_ORDERED;
		packet_info.channel = 0;
		packet_info.data = static_cast<const void *>(frame_counter.c_str());
		packet_info.lenght = frame_counter.length();
		send(packet_info);

		while (enet_host_service(enet_handle, &event, 1000) > 0) {

			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					printf("Client#%i connected from  %d.%d.%d.%d:%u\n",
						   event.peer->connectID,
						   event.peer->address.host & 0xFF,
						   (event.peer->address.host >> 8) & 0xFF,
						   (event.peer->address.host >> 16) & 0xFF,
						   (event.peer->address.host >> 24) & 0xFF,
						   event.peer->address.port);
					break;
				case ENET_EVENT_TYPE_RECEIVE:
					printf("A packet of length %u containing %s was received from Client#%i %s on channel %u.\n",
						   event.peer->connectID,
						   (unsigned int) event.packet->dataLength,
						   event.packet->data,
						   (char *) event.peer->data,
						   event.channelID);

					/* Clean up the packet now that we're done using it. */
					enet_packet_destroy(event.packet);

					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					printf("%s disconnected.\n", event.peer->data);
					/* Reset the peer's client information. */
					event.peer->data = NULL;
			}
		}

	}

	void Server::stop() {
		enet_host_destroy(enet_handle);
		enet_deinitialize();
	}

	bool Server::send(const PacketInfo &packet_info) {
		for (int i = 0; i < enet_handle->connectedPeers; ++i) {
			ENetPacket *packet = enet_packet_create(packet_info.data, packet_info.lenght, packet_info.mode);
			if (enet_peer_send(&enet_handle->peers[i], packet_info.channel, packet) < 0) {
				printf("error: failed to send packet to client[%i].\n", i);
				enet_packet_destroy(packet);
				return false;
			}
		}

		return true;
	}
}

