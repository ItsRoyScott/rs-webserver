#pragma once

#include <array>
#include <functional>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "TcpSocket.hpp"
#include <vector>

namespace OlympusWebServer
{
  class WebServer
  {
  private: // data

    std::vector<TcpSocket> clients;
    unsigned short port;
    TcpSocket socket;

  public: // data

    std::function<HttpResponse(HttpRequest)> PostResponse;

  public: // methods

    WebServer(WebServer&& b)
    {
      *this = std::move(b);
    }

    WebServer& operator=(WebServer&& b)
    {
      clients = std::move(b.clients);
      port = b.port;
      socket = std::move(socket);

      return *this;
    }

    explicit WebServer(unsigned short port_ = 8800) :
      port(port_)
    {
      static const auto maxOpenAttempts = 100;
      while (!socket.Open(true, port, false) && port < port_ + maxOpenAttempts)
      {
        ++port;
      }
    }

    HttpResponse HandleRequest(HttpRequest request)
    {
      return HttpResponse();
    }

    bool IsRunning() const
    {
      return socket.IsOpen() && socket.IsListening();
    }

    void Update()
    {
      if (!socket.IsOpen())
      {
        return;
      }

      // Accept a new client.
      auto client = TcpSocket();
      if (socket.Accept(client, false))
      {
        clients.push_back(std::move(client));
      }

      auto clientsToRemove = std::vector<std::size_t>();

      for (auto i = 0u; i < clients.size(); ++i)
      {
        auto& client = clients[i];

        // Remove a client if it is no longer open.
        if (!client.IsOpen())
        {
          clientsToRemove.push_back(i);
          continue;
        }

        // Receive the request from the client.
        auto requestString = client.Receive();
        if (requestString.empty())
        {
          continue;
        }
        auto request = HttpRequest(std::move(requestString));

        // Send a continue if it is requested.
        if (request.GetHttpVersion() == 1.1f && request["Expect"] == "100-continue")
        {
          client.Send(HttpResponse(HttpStatus::Continue).GetFormattedResponse());
        }

        // Process a response for the request.
        client.Send(HandleRequest(std::move(request)).GetFormattedResponse());
      }

      // Destroy all clients that should be removed.
      for (auto rit = clientsToRemove.rbegin(); rit != clientsToRemove.rend(); ++rit)
      {
        clients.erase(clients.begin() + *rit);
      }
    }
  };
} // namespace OlympusWebServer