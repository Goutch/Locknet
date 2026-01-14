#ifdef USE_STEAMWORKS
#include "SteamService.h"

#include "steam/steam_api.h"


namespace locknet {
    SteamService::SteamService() {
        if (!SteamAPI_Init()) {
            printf("An error occurred while initializing SteamAPI.\n");
        }
    }
}
#endif