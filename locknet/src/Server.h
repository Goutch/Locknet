
#pragma once

#include "enet/enet.h"
#include "vector"
#include "Packet.h"
#include "LocknetCore.h"
#include "Event.h"

using namespace utils;
namespace locknet {
	struct ServerInfo {
		int port;
		int max_clients;
		int max_channels;
	};

	struct ClientData {
		uint32_t id = UINT32_MAX;
		ENetPeer *peer = nullptr;
	};

	class LOCKNET_API Server {

	public:
		Event<void *, int> on_receive_event;
		Event<int> on_client_connect;
		Event<int> on_client_disconnect;
	private:
		ServerInfo info;
		ENetHost *enet_handle;

	protected:
		std::vector<ClientData> clients;
		std::vector<uint32_t> free_client_ids;
	public:
		Server(const ServerInfo &info);

		void start();

		void pollEvents(uint32_t timeout_ms);

		void stop();

		bool sendAll(const LocknetPacket &packet_info);

		bool send(const LocknetPacket &packet_info, uint32_t client_id);

		void disconnectClient(uint32_t client_id, uint32_t info);

	private:
		void onClientConnect(ENetEvent &event);

		void disconnectClient(ENetPeer *peer, DISCONNECT_INFO info);

		void processLocknetPacket(uint8_t *data);

		bool broadcast(const LocknetPacket &packet_info, uint32_t sender_client_id);
	};
}



