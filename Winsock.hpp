#pragma once

#include <stdexcept>
#include <WinSock2.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

namespace OlympusWebServer
{
  class Winsock
  {
  public: // methods

    static bool Accept(SOCKET socket, sockaddr_in address, SOCKET& newSocket)
    {
      auto addressSize = static_cast<int>(sizeof(address));
      newSocket = accept(socket, (sockaddr*) &address, &addressSize);
      return newSocket != INVALID_SOCKET;
    }

    static bool Bind(SOCKET socket, sockaddr_in address)
    {
      return bind(socket, (sockaddr*) &address, sizeof(address)) != SOCKET_ERROR;
    }

    static SOCKET CreateTcpSocket()
    {
      return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }

    static bool DestroySocket(SOCKET socket)
    {
      return shutdown(socket, SD_BOTH) != SOCKET_ERROR && closesocket(socket) != SOCKET_ERROR;
    }

    static sockaddr_in GetLoopbackAddress()
    {
      auto loopbackAddress = sockaddr_in();
      loopbackAddress.sin_family = AF_INET;
      loopbackAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

      return loopbackAddress;
    }

    static sockaddr_in GetLoopbackAddress(unsigned short port)
    {
      auto loopback = GetLoopbackAddress();
      loopback.sin_port = htons(port);

      return loopback;
    }

    static Winsock& Initialize()
    {
      static Winsock instance;
      return instance;
    }

    static bool IoctlSocket(SOCKET socket, bool blocking)
    {
      auto blockingMode = blocking ? 0ul : 1ul;
      return ::ioctlsocket(socket, FIONBIO, &blockingMode) != SOCKET_ERROR;
    }

    static bool Listen(SOCKET socket)
    {
      static const int backlog = 32;
      return ::listen(socket, backlog) != SOCKET_ERROR;
    }

    template <std::size_t BufferLength>
    static int Receive(SOCKET socket, char (&buffer)[BufferLength])
    {
      return recv(socket, buffer, BufferLength, 0);
    }

    static int Send(SOCKET socket, std::string const& data)
    {
      return send(socket, data.data(), data.size(), 0);
    }

  private: // methods

    Winsock()
    {
      auto winsockData = WSADATA();
      auto result = WSAStartup(WINSOCK_VERSION, &winsockData);
      if (result != 0)
      {
        throw std::runtime_error("Winsock.Winsock - WSAStartup failed");
      }
    }

    ~Winsock()
    {
      if (WSACleanup() == SOCKET_ERROR)
      {
        throw std::runtime_error("Winsock.~Winsock - WSACleanup failed");
      }
    }
  };

} // namespace OlympusWebServer