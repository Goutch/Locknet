#pragma once

#include "string"
#include "enet/enet.h"
#include "Packet.h"

namespace LockNet {
	struct ClientInfo {
		int max_channels = 2;
		int server_port = 1234;
		std::string server_ip = "127.0.0.1";
	};

	class Client {
		ENetHost *enet_handle;
		ENetPeer *server_handle;
		ClientInfo info;
	public:
		void pollEvents();

		Client(const ClientInfo &info);

		~Client();

		bool connect(int timeout_ms);

		bool send(const PacketInfo &packet_info);
	};

}

