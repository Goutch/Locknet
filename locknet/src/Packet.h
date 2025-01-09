#include "enet/enet.h"

#pragma once
namespace locknet {

	enum PACKET_MODE {
		PACKET_MODE_UNRELIABLE_UNORDERED = ENET_PACKET_FLAG_UNSEQUENCED,
		PACKET_MODE_UNRELIABLE_ORDERED = 0,
		PACKET_MODE_RELIABLE_ORDERED = ENET_PACKET_FLAG_RELIABLE,
	};

	struct LocknetPacket {
		int channel;
		int length;
		const void *data = nullptr;
		PACKET_MODE mode;
	};

	enum LOCKNET_PACKET_TYPE {
		LOCKNET_PACKET_TYPE_CONNECT, // Client send this to the sever when it establishes a connection to send the client info
		LOCKNET_PACKET_TYPE_CONNECTION_SUCCESS_RESPONSE, // Server send this to the client when it receives a connect packet
		LOCKNET_PACKET_TYPE_CLIENT_CONNECTED, // Server send this to all clients when a new client connected
		LOCKNET_PACKET_TYPE_CLIENT_DISCONECTED, // Server send this to all clients when a client disconnected
		LOCKNET_PACKET_TYPE_BROADCAST_REQUEST,// Client sends this to server to broadcast a packet to all other clients
		LOCKNET_PACKET_TYPE_BROADCAST, // Server sends this to all clients when a client sends a broadcast request

		LOCKNET_PACKET_TYPE_FRAME_INPUT, // Client send this to the sever when at the end of a frame
		LOCKNET_PACKET_TYPE_FRAME_READY, // Server sends this to all clients when all clients have sent their inputs for this frame so the client can process all Inputs from all clients
	};

	enum DISCONNECT_INFO {
		DISCONNECT_INFO_NONE,
		DISCONNECT_INFO_KICKED,
		DISCONNECT_INFO_SERVER_FULL,
	};

	struct ConnectSuccessResponseHeader {
		LOCKNET_PACKET_TYPE type = LOCKNET_PACKET_TYPE_CONNECTION_SUCCESS_RESPONSE;
		int32_t client_id;
	};
	struct ClientConnectedHeader {
		LOCKNET_PACKET_TYPE type = LOCKNET_PACKET_TYPE_CLIENT_CONNECTED;
		int32_t client_id;
	};

	struct DisconnectedHeader {
		LOCKNET_PACKET_TYPE type = LOCKNET_PACKET_TYPE_CLIENT_DISCONECTED;
		uint32_t client_id;
	};

	struct BroadcastRequestHeader {
		LOCKNET_PACKET_TYPE type = LOCKNET_PACKET_TYPE_BROADCAST_REQUEST;
		int32_t sender_client_id;
		uint32_t data_length;
	};

	struct BroadcastHeader {
		LOCKNET_PACKET_TYPE type = LOCKNET_PACKET_TYPE_BROADCAST;
		int32_t sender_client_id;
		uint32_t data_length;
	};

	struct FrameInputHeader {
		int32_t client_id;
		uint32_t data_length;
	};

	struct FrameReadyHeader {
		uint32_t data_length;
		uint32_t frame_id;
		uint32_t simulation_id;
	};
}