#include <stdexcept>
#include "Client.h"
//#include <steam/steam_api.h>

namespace locknet {
    Client::Client(const ClientInfo &info) {
        this->info = info;

        //if (!SteamAPI_Init()) {
           // printf("An error occurred while initializing SteamAPI.\n");
        //}
    }


    Client::~Client() {
        disconnect();
    }

    bool Client::disconnect() {
        if (connected) {
            //todo:disconnect here
        }
        return true;
    }

    bool Client::connect(int timeout_ms) {
        if (connected) {
            printf("Client is already connected\n");
            return true;
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
        switch (info.service_type) {
            case SERVICE_TYPE_NONE:
                break;
            case SERVICE_TYPE_STEAM:
                break;
        }
        /*
           ENetEvent event;
           while (enet_host_service(enet_handle, &event, 0) > 0) {
               switch (event.type) {
                   case ENET_EVENT_TYPE_CONNECT:
                       throw std::runtime_error(
                           "Client should not receive connect event after connection is established.");
                       break;
                   case ENET_EVENT_TYPE_RECEIVE:
                       processLocknetPacket(event.packet->data);
                       enet_packet_destroy(event.packet);
                       break;

                   case ENET_EVENT_TYPE_DISCONNECT: {
                       connected = false;
                       DISCONNECT_INFO disconnect_reason = (DISCONNECT_INFO) (event.data);

                       on_disconnect.invoke(disconnect_reason);

                       //Reset the peer's client information.
        event.peer->data = NULL;*/
    }

    bool Client::send(const LocknetPacket &packet_info) {
        switch (info.service_type) {
            case SERVICE_TYPE_STEAM:
                return false;
                break;
            case SERVICE_TYPE_NONE:
                return true;
                break;
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
