#include <stdexcept>
#include "Client.h"
#include "enet/enet.h"

namespace locknet {
	Client::Client(const ClientInfo &info) {
		this->info = info;
		if (enet_initialize() != 0) {
			printf("An error occurred while initializing ENet.");
		}
	}

	Client::~Client() {
		disconnect();
		atexit(enet_deinitialize);
	}

	bool Client::disconnect() {
		if (connected) {
			enet_peer_disconnect(server_handle, 0);
			enet_host_destroy(enet_handle);
		}
		return true;
	}

	bool Client::connect(int timeout_ms) {
		if (connected) {
			printf("Client is already connected\n");
			return true;
		}
		connected = false;
		enet_handle = enet_host_create(NULL /* create a client host */,
		                               1 /* only allow 1 outgoing connection */,
		                               info.max_channels /* allow up 2 channels to be used, 0 and 1 */,
		                               0 /* assume any amount of incoming bandwidth */,
		                               0 /* assume any amount of outgoing bandwidth */);
		if (enet_handle == NULL) {
			throw std::runtime_error("An error occurred while trying to create an ENet client host.");
			return false;
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

		} else {
			/* Either the 5 seconds are up or a disconnect event was */
			/* received. Reset the peer in the event the 5 seconds   */
			/* had run out without any significant event.            */
			enet_peer_reset(server_handle);
			printf("Connection to %s failed.\n", info.server_ip.c_str());
			return false;
		}

		connected = true;
		return true;
	}

	std::string ipToString(int ip) {
		std::string ip_str;
		ip_str += std::to_string(ip & 0xFF);
		ip_str += ".";
		ip_str += std::to_string((ip >> 8) & 0xFF);
		ip_str += ".";
		ip_str += std::to_string((ip >> 16) & 0xFF);
		ip_str += ".";
		ip_str += std::to_string((ip >> 24) & 0xFF);
		return ip_str;
	}

	void Client::processLocknetPacket(uint8_t *data) {
		LOCKNET_PACKET_TYPE type = *reinterpret_cast<LOCKNET_PACKET_TYPE *>(data);
		switch (type) {
			case LOCKNET_PACKET_TYPE::LOCKNET_PACKET_TYPE_CONNECTION_SUCCESS_RESPONSE: {
				ConnectSuccessResponseHeader header = *reinterpret_cast<ConnectSuccessResponseHeader *>(data);
				id = header.client_id;
				on_connect_success.invoke();
				break;
			}
			case LOCKNET_PACKET_TYPE::LOCKNET_PACKET_TYPE_CLIENT_CONNECTED: {
				ClientConnectedHeader header = *reinterpret_cast<ClientConnectedHeader *>(data);
				on_client_connected.invoke(header.client_id);
				break;
			}
			case LOCKNET_PACKET_TYPE::LOCKNET_PACKET_TYPE_BROADCAST: {
				BroadcastHeader broadcast_header = *reinterpret_cast<BroadcastHeader *>(data);
				data += sizeof(BroadcastHeader);
				on_receive.invoke(broadcast_header.sender_client_id, data);
				break;
			}
			case LOCKNET_PACKET_TYPE::LOCKNET_PACKET_TYPE_CLIENT_DISCONECTED: {
				DisconnectedHeader header = *reinterpret_cast<DisconnectedHeader *>(data);
				on_client_disonnected.invoke(header.client_id);
				break;
			}
			default:
				printf("Locknet: Unhandled packet type\n");
				break;
		}
	}

	void Client::pollEvents() {
		ENetEvent event;
		while (enet_host_service(enet_handle, &event, 0) > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					throw std::runtime_error("Client should not receive connect event after connection is established.");
					break;
				case ENET_EVENT_TYPE_RECEIVE:
					processLocknetPacket(event.packet->data);
					enet_packet_destroy(event.packet);
					break;

				case ENET_EVENT_TYPE_DISCONNECT: {
					connected = false;
					DISCONNECT_INFO disconnect_reason = (DISCONNECT_INFO) (event.data);

					on_disconnect.invoke(disconnect_reason);

					/* Reset the peer's client information. */
					event.peer->data = NULL;
				}
			}
		}
	}

	bool Client::send(const LocknetPacket &packet_info) {
		ENetPacket *packet = enet_packet_create(packet_info.data, packet_info.length, packet_info.mode);
		if (enet_peer_send(server_handle, packet_info.channel, packet) < 0) {
			printf("error: failed to send packet to server.\n");
			enet_packet_destroy(packet);
			return false;
		}
		return true;
	}

	int32_t Client::getID() const {
		return id;
	}

	bool Client::broadcast(LocknetPacket &packet) {
		BroadcastRequestHeader header = {};
		header.sender_client_id = id;
		header.data_length = packet.length;

		std::string str = (char *) packet.data;

		char *packet_data = (char *) malloc(sizeof(BroadcastRequestHeader) + packet.length);

		memcpy(packet_data, &header, sizeof(BroadcastRequestHeader));
		memcpy(packet_data + sizeof(BroadcastRequestHeader), packet.data, packet.length);

		LocknetPacket broadcast_packet = {};
		broadcast_packet.channel = 0;
		broadcast_packet.data = packet_data;
		broadcast_packet.length = sizeof(BroadcastRequestHeader) + packet.length;
		broadcast_packet.mode = packet.mode;

		send(broadcast_packet);

		free(packet_data);

		return true;
	}

	bool Client::isConnected() {
		return connected;
	}


}

