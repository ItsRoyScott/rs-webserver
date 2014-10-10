#pragma once

#include "Winsock.hpp"

namespace OlympusWebServer
{
  class TcpSocket
  {
  private: // data

    bool isBlocking;
    bool isListening;
    SOCKET socket;

  public: // methods

    TcpSocket() :
      isBlocking(false),
      isListening(false),
      socket(0)
    {
    }

    TcpSocket(TcpSocket&& b)
    {
      *this = std::move(b);
    }

    TcpSocket& operator=(TcpSocket&& b)
    {
      isBlocking = b.isBlocking;
      isListening = b.isListening;
      socket = b.socket;

      b.isBlocking = false;
      b.isListening = false;
      b.socket = 0;

      return *this;
    }

    ~TcpSocket()
    {
      Close();
    }

    bool Accept(TcpSocket& connection, bool blocking = true) const
    {
      if (!IsListening())
      {
        throw std::runtime_error("TcpSocket.Accept - Called on a socket that does not have listening mode enabled");
      }

      auto newSocket = SOCKET(0);
      auto address = sockaddr_in();
      auto tryAgain = bool();

      do
      {
        tryAgain = false;
        if (!Winsock::Accept(socket, address, newSocket))
        {
          switch (WSAGetLastError())
          {
          case WSAEWOULDBLOCK:
            //if (!IsBlocking())
            //{
            //  throw std::runtime_error("TcpSocket.Accept - Received would-block error for non-blocking socket");
            //}
            return false;

          case WSAECONNRESET:
            tryAgain = true;
            break;

          default:
            return false;
          }
        }
      } while (tryAgain);

      if (connection.IsOpen())
      {
        connection.Close();
      }

      connection.socket = newSocket;
      connection.isListening = false;

      if (!Winsock::IoctlSocket(connection.socket, blocking))
      {
        throw std::runtime_error("TcpSocket.Accept - Unable to set blocking mode on new connection");
      }
      connection.isBlocking = blocking;

      return true;
    }

    void Close()
    {
      if (!IsOpen())
      {
        return;
      }

      if (!Winsock::DestroySocket(socket))
      {
        throw std::runtime_error("TcpSocket.Close - Error shutting down socket");
      }
    }

    bool IsBlocking() const
    {
      return isBlocking;
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#endif

    bool IsConnected() const
    {
      if (!IsOpen())
      {
        return false;
      }

      auto timeout = timeval();
      timeout.tv_sec = 0;
      timeout.tv_usec = 1000 * 100;

      fd_set writeSet;
      FD_ZERO(&writeSet);
      FD_SET(socket, &writeSet);

      int result = select(0, NULL, &writeSet, NULL, &timeout);
      switch (result)
      {
      case SOCKET_ERROR:
        throw std::runtime_error("TcpSocket.IsConnected - Error calling select");

      case 0:
        return false;

      default:
        if (FD_ISSET(socket, &writeSet))
        {
          return true;
        }
        else
        {
          return false;
        }
      }
    }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    bool IsListening() const
    {
      return isListening;
    }

    bool IsOpen() const 
    { 
      return socket != 0; 
    }

    bool Open(bool listen = false, unsigned short port = 0, bool blocking = true)
    {
      if (IsOpen()) 
      {
        throw std::runtime_error("TcpSocket.Open - Open called on socket that is already open");
      }

      Winsock::Initialize();

      auto newSocket = Winsock::CreateTcpSocket();
      if (newSocket == INVALID_SOCKET)
      {
        throw std::runtime_error("TcpSocket.Open - Failed to create a TCP socket");
      }

      auto loopback = Winsock::GetLoopbackAddress(port);
      if (!Winsock::Bind(newSocket, loopback))
      {
        throw std::runtime_error("TcpSocket.Open - Failed to bind the socket to the given address");
      }

      if (listen && !Winsock::Listen(newSocket))
      {
        throw std::runtime_error("TcpSocket.Open - Failed to set socket to listening mode");
      }
      isListening = listen;

      if (!Winsock::IoctlSocket(newSocket, blocking))
      {
        auto message = std::ostringstream();
        message << "TcpSocket.Open - Failed to set socket blocking mode - Error: " << WSAGetLastError();
        throw std::runtime_error(message.str());
      }
      isBlocking = blocking;

      socket = newSocket;

      return true;
    }

    std::string Receive()
    {
      if (!IsOpen())
      {
        throw std::runtime_error("TcpSocket.Receive - Called on a closed/invalid socket");
      }

      static const auto bufferSize = 64u * 1024u;
      char buffer[bufferSize];

      auto recvResult = Winsock::Receive(socket, buffer);
      if (recvResult == SOCKET_ERROR)
      {
        switch (WSAGetLastError())
        {
        case WSAEWOULDBLOCK:
          return std::string();

        case WSAENOTCONN:
        case WSAENETRESET:
        case WSAESHUTDOWN:
        case WSAECONNABORTED:
        case WSAETIMEDOUT:
        case WSAECONNRESET:
          Close();
          return std::string();

        default:
          return std::string();
          //throw std::runtime_error("TcpSocket.Receive - Unable to receive data over socket");
        }
      }

      if (recvResult == 0) // graceful close
      {
        Close();
      }

      return std::string(buffer, recvResult);
    }

    bool Send(std::string const& data)
    {
      if (data.empty())
      {
        return false;
      }
      if (!IsOpen())
      {
        throw std::runtime_error("TcpSocket.Send - Called on a closed/invalid socket");
      }
      if (!IsConnected())
      {
        throw std::runtime_error("TcpSocket.Send - Called on a socket that is not connected");
      }

      auto sendResult = Winsock::Send(socket, data);
      if (sendResult == SOCKET_ERROR)
      {
        auto error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK)
        {
          throw std::runtime_error("TcpSocket.Send - Unable to send data over socket");
        }
        return false;
      }

      if (static_cast<unsigned>(sendResult) != data.size())
      {
        return false;
      }

      return true;
    }
  };
} // namespace OlympusWebServer