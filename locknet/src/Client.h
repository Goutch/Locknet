#pragma once

#include "string"
#include "Event.h"
#include "Packet.h"
#include "LocknetCore.h"
#include "vector"

using namespace utils;

namespace locknet {
    class Service;

    enum LOCKNET_SERVICE_TYPE {
        SERVICE_TYPE_LOCAL,
        SERVICE_TYPE_STEAM
    };

    struct ClientInfo {
        LOCKNET_SERVICE_TYPE service_type = SERVICE_TYPE_LOCAL;
    };

    class LOCKNET_API Client {
        ClientInfo info;
        uint32_t id;
        bool connected = false;
        Service *service;

    public:
        Event<uint32_t, void *> on_receive; //When server sends a custom packet to this client
        Event<> on_connect_success; //When server accepted this client connection and assigned an id
        Event<uint32_t> on_client_connected; //When this or any other client connect to server
        Event<uint32_t> on_client_disonnected; //When any other client disconnect from server
        Event<DISCONNECT_INFO> on_disconnect; //When server disconnects this client

        void pollEvents();

        int32_t getID() const;

        Client(const ClientInfo &info);

        ~Client();

        bool connect(int timeout_ms);

        bool broadcast(LocknetPacket &packet);

        bool disconnect();

        bool send(const LocknetPacket &packet_info);

        void processLocknetPacket(uint8_t *data);

        bool isConnected();
    };
}
