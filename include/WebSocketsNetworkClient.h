// Force-include this header for W5500 builds: -include WebSocketsNetworkClient.h.
// Ethernet 2.0.2 predates ESP32 Server::begin(uint16_t) and the three-argument
// client connect used by WebSockets 2.6.1. Keep this adapter in the project so
// dependency sources remain untouched. Framework/dependency translation units
// without the complete networking include path deliberately skip the adapter.
#if defined(__cplusplus) && defined(ARDUINO) && defined(IDLAP_AKTIF_W5500) && IDLAP_AKTIF_W5500
#if __has_include(<Ethernet.h>) && __has_include(<SPI.h>) && __has_include(<WebSockets.h>)
#ifndef IDLAP_WEBSOCKETS_NETWORK_ADAPTER_H
#define IDLAP_WEBSOCKETS_NETWORK_ADAPTER_H

#include <Ethernet.h>
#include <SPI.h>

class IdLapEthernetClient : public EthernetClient {
 public:
  IdLapEthernetClient() : EthernetClient() { configure(); }
  IdLapEthernetClient(const EthernetClient& other) : EthernetClient(other) { configure(); }
  using EthernetClient::connect;
  int connect(const char* host, uint16_t port, int32_t timeout) {
    setConnectionTimeout(limitTimeout(timeout));
    return EthernetClient::connect(host, port);
  }
  int connect(IPAddress ip, uint16_t port, int32_t timeout) {
    setConnectionTimeout(limitTimeout(timeout));
    return EthernetClient::connect(ip, port);
  }
 private:
  static uint16_t limitTimeout(int32_t timeout) {
    return static_cast<uint16_t>(timeout < 0 ? 0 : timeout > 65535 ? 65535 : timeout);
  }
  void configure() {
    setConnectionTimeout(50);
    setTimeout(50);
  }
};

class IdLapEthernetServer : public EthernetServer {
 public:
  explicit IdLapEthernetServer(uint16_t port) : EthernetServer(port) {}
  // Port is fixed by the base constructor, as required by Arduino Ethernet.
  void begin(uint16_t = 0) override { EthernetServer::begin(); }
  IdLapEthernetClient available() { return IdLapEthernetClient(EthernetServer::available()); }
  IdLapEthernetClient accept() { return IdLapEthernetClient(EthernetServer::accept()); }
};

// Bind both WSclient_t members and library implementation macros to the same
// concrete types. Load Ethernet/SPI first to keep their declarations untouched.
#define EthernetClient IdLapEthernetClient
#define EthernetServer IdLapEthernetServer
#include <WebSockets.h>
#undef EthernetClient
#undef EthernetServer
#undef WEBSOCKETS_NETWORK_CLASS
#undef WEBSOCKETS_NETWORK_SERVER_CLASS
#define WEBSOCKETS_NETWORK_CLASS IdLapEthernetClient
#define WEBSOCKETS_NETWORK_SERVER_CLASS IdLapEthernetServer

#endif
#endif
#endif
