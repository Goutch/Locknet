
#pragma once
#include "Packet.h"
namespace LockNet {
	struct ServerInfo {
		int port;
		int max_clients;
		int max_channels;
		int frame_time_ms = 1000;
	};


	class Server {
		ServerInfo info;
		ENetHost *enet_handle;
	public:
		Server(const ServerInfo& info);

		void start();
		void pollEvents();
		void stop();

		bool send(const PacketInfo& packet_info);
	};
}



