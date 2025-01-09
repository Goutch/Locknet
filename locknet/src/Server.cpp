
#include <string>
#include <iostream>
#include "Server.h"
#include "stdio.h"

namespace locknet {
	Server::Server(const ServerInfo &info) {
		this->info = info;
		free_client_ids.reserve(info.max_clients);
		for (int i = 0; i < info.max_clients; i++) {
			free_client_ids.push_back((info.max_clients - 1) - i);
		}
		clients.resize(info.max_clients, {});
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

		pollEvents(0);
	}


	void Server::onClientConnect(ENetEvent &event) {

		if (free_client_ids.empty()) {
			//Server full
			disconnectClient(event.peer, DISCONNECT_INFO_SERVER_FULL);
			return;
		}
		uint32_t id = free_client_ids.back();
		clients[id] = {id, event.peer};
		event.peer->data = &clients[id];
		free_client_ids.pop_back();

		ConnectSuccessResponseHeader header{};
		header.client_id = id;
		LocknetPacket connect_success_packet{};
		connect_success_packet.mode = PACKET_MODE_RELIABLE_ORDERED;
		connect_success_packet.channel = 0;
		connect_success_packet.data = &header;
		connect_success_packet.length = sizeof(ConnectSuccessResponseHeader);

		send(connect_success_packet, id);

		ClientConnectedHeader client_connected_header{};
		client_connected_header.client_id = id;
		LocknetPacket client_connected_packet{};
		client_connected_packet.mode = PACKET_MODE_RELIABLE_ORDERED;
		client_connected_packet.channel = 0;
		client_connected_packet.data = &client_connected_header;
		client_connected_packet.length = sizeof(ClientConnectedHeader);

		sendAll(client_connected_packet);

		on_client_connect.invoke(id);
	}

	void Server::processLocknetPacket(uint8_t *data) {
		LOCKNET_PACKET_TYPE type = *reinterpret_cast<LOCKNET_PACKET_TYPE *>(data);
		switch (type) {
			case LOCKNET_PACKET_TYPE::LOCKNET_PACKET_TYPE_BROADCAST_REQUEST: {
				BroadcastRequestHeader header = *reinterpret_cast<BroadcastRequestHeader *>(data);
				printf("Locknet: Broadcasst request from client#%i\n", header.sender_client_id);

				data += sizeof(BroadcastRequestHeader);

				LocknetPacket packet_info = {};
				packet_info.channel = 0;
				packet_info.data = data;
				packet_info.length = header.data_length;
				packet_info.mode = PACKET_MODE_RELIABLE_ORDERED;

				broadcast(packet_info, header.sender_client_id);

				on_receive_event.invoke(data, header.sender_client_id);
				break;
			}

			default:
				printf("Locknet: Unhandled packet type\n");
				break;
		}
	}

	void Server::disconnectClient(ENetPeer *peer, DISCONNECT_INFO info) {
		enet_peer_disconnect(peer, info);
	}


	void Server::pollEvents(uint32_t timeout_ms) {
		ENetEvent event;

		while (enet_host_service(enet_handle, &event, timeout_ms) > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					printf("Enet: Client connect request from  %d.%d.%d.%d:%u\n",
					       event.peer->address.host & 0xFF,
					       (event.peer->address.host >> 8) & 0xFF,
					       (event.peer->address.host >> 16) & 0xFF,
					       (event.peer->address.host >> 24) & 0xFF,
					       (uint32_t) event.peer->address.port);

					onClientConnect(event);
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_RECEIVE: {
					ClientData *clientData = static_cast<ClientData *>(event.peer->data);
					printf("Enet: Received packet from client#%i:%s\n", clientData->id, reinterpret_cast<char *>(event.packet->data));
					processLocknetPacket(event.packet->data);
					enet_packet_destroy(event.packet);
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT: {
					ClientData *client_data = static_cast<ClientData *>(event.peer->data);
					printf("Enet: Client#%i disconnected.\n", client_data->id);

					on_client_disconnect.invoke(client_data->id);
					clients[client_data->id] = {UINT32_MAX, nullptr};
					free_client_ids.push_back(client_data->id);


					DisconnectedHeader header{};
					header.client_id = client_data->id;

					LocknetPacket packet{};
					packet.channel = 0;
					packet.mode = PACKET_MODE_RELIABLE_ORDERED;
					packet.length = sizeof(DisconnectedHeader);
					packet.data = &header;

					sendAll(packet);
				}

			}
		}

	}

	void Server::stop() {
		enet_host_destroy(enet_handle);
		enet_deinitialize();
	}


	bool Server::sendAll(const LocknetPacket &packet_info) {
		for (int i = 0; i < clients.size(); ++i) {
			if (clients[i].peer != nullptr) {
				send(packet_info, i);
			}
		}
		return true;
	}

	bool Server::send(const LocknetPacket &packet_info, uint32_t client_id) {
		ENetPacket *packet = enet_packet_create(packet_info.data, packet_info.length, packet_info.mode);
		if (enet_peer_send(clients[client_id].peer, packet_info.channel, packet) < 0) {
			printf("error: failed to send packet to client[%i].\n", client_id);
			enet_packet_destroy(packet);
			return false;
		}
		return true;
	}

	bool Server::broadcast(const LocknetPacket &packet_info, uint32_t sender_client_id) {

		BroadcastHeader header = {};
		header.sender_client_id = sender_client_id;
		header.data_length = packet_info.length;

		size_t packet_size = sizeof(BroadcastHeader) + header.data_length;


		char *packet_data = (char *) malloc(packet_size);
		memcpy(packet_data, &header, sizeof(BroadcastHeader));
		memcpy(packet_data + sizeof(BroadcastHeader), packet_info.data, header.data_length);

		LocknetPacket broadcast_packet = {};
		broadcast_packet.channel = 0;
		broadcast_packet.data = packet_data;
		broadcast_packet.length = packet_size;
		broadcast_packet.mode = packet_info.mode;

		sendAll(broadcast_packet);

		free(packet_data);

		return true;
	}

	void Server::disconnectClient(uint32_t client_id, uint32_t info) {
		disconnectClient(clients[client_id].peer, (DISCONNECT_INFO) info);
	}


}

