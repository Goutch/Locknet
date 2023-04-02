#include "enet/enet.h"

#pragma once
namespace LockNet {

	enum PACKET_MODE {
		PACKET_MODE_UNRELIABLE_UNORDERED = ENET_PACKET_FLAG_UNSEQUENCED,
		PACKET_MODE_UNRELIABLE_ORDERED = 0,
		PACKET_MODE_RELIABLE_ORDERED = ENET_PACKET_FLAG_RELIABLE,
	};

	enum PACKET_TYPE {
		LOCKNET_PACKET_TYPE_CONNECT, // Client send this to the sever when it establishes a connection to send the client info
		LOCKNET_PACKET_TYPE_CONNECT_RESPONSE, // Server send this to the client when it receives a connect packet
		LOCKNET_PACKET_TYPE_DISCONNECT_REQUEST, // Client send this to the server when it wants to disconnect
		LOCKNET_PACKET_TYPE_DISCONECTED, // Server send this to all clients when a client disconnected
		LOCKNET_PACKET_TYPE_FRAME_COMMANDS, // Client send this to the sever when at the end of a frame
		LOCKNET_PACKET_TYPE_FRAME_UPDATE, // Server sends this to all clients when all clients have sent their commands for this frame so the client can process all commands from all clients
		LOCKNET_PACKET_TYPE_CUSTOM // Custom packet type
	};


	struct PacketInfo {
		int channel;
		int lenght;
		const void *data;
		PACKET_MODE mode;
	};
}