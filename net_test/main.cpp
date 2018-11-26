#include "net_proto_handshake.h"
#include "net_proto_game_client.h"
#include "net_proto_game_server.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "winmm.lib")



void init_client(NetHost& host)
{
#ifdef CLIENT
    auto handshake = new NetProtocolHandshake;
    auto game = new NetProtocolGameClient;

    handshake->bind(host);
    game->bind_reliable(host);
    game->bind_unreliable(host);

    host.listen(1, NetAddress::any);
    bool ok = host.connect(NetAddress::ipv4("127.0.0.1", 1200));
    M_ASSERT_MSG(ok, "Cannot launch host");
#endif
}


void init_server(NetHost& host)
{
#ifdef SERVER
    using namespace std::placeholders;

    auto handshake = new NetProtocolHandshake;
    auto game = new NetProtocolGameServer;

    handshake->bind(host);
    game->level = "test_scene";
    game->bind_reliable(host);
    game->bind_unreliable(host);
    handshake->onConnected += std::bind(&NetProtocolGameServer::onConnected, game, _1);

    bool ok = host.listen(32, NetAddress::ipv4("127.0.0.1", 1200));
    M_ASSERT_MSG(ok, "Cannot launch host");

    printf("> server started\n");
#endif
}


int main()
{
    enet_initialize();
    {
        NetHost client("Client");
        NetHost server("Server");

        init_client(client);
        init_server(server);
        while (true) {
            client.update();
            server.update();
            Sleep(100);
        }
    }
    enet_deinitialize();
}