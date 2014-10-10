#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include "HttpTypes.hpp"
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace OlympusWebServer
{
  class HttpRequest
  {
  private: // data

    std::vector<std::string> collections;
    float httpVersion;
    HttpMethod::Value method;
    std::unordered_map<std::string, std::string> params;
    std::string path;
    std::string protocol;
    std::unordered_map<std::string, std::string> queries;
    std::string resource;

  public: // methods

    HttpRequest(HttpRequest&& b)
    {
      *this = std::move(b);
    }

    HttpRequest& operator=(HttpRequest&& b)
    {
      collections = std::move(b.collections);
      httpVersion = b.httpVersion;
      method = b.method;
      params = std::move(b.params);
      path = std::move(b.path);
      protocol = std::move(b.protocol);
      queries = std::move(b.queries);
      resource = std::move(b.resource);

      return *this;
    }

    HttpRequest(std::string request)
    {
      // Read the method, path, and protocol.
      auto istream = std::istringstream(std::move(request));
      auto methodString = std::string();
      istream >> methodString >> path >> protocol;
      if (methodString.empty() || path.empty() || protocol.empty())
      {
        return; // invalid request
      }

      std::transform(methodString.begin(), methodString.end(), methodString.begin(), std::toupper);
      httpVersion = std::stof(protocol.substr(protocol.find('/') + 1));

      method = ParseMethod(methodString);
      if (method == HttpMethod::Unknown)
      {
        return; // invalid request
      }

      auto queryStart = path.rfind('?');
      if (queryStart != std::string::npos)
      {
        ParseQueries(path.substr(queryStart + 1));
      }

      ParseCollections(path.substr(0, queryStart));
      ParseParameters(istream);
    }

    float GetHttpVersion() const
    {
      return httpVersion;
    }

    std::string operator[](std::string const& paramKey) const
    {
      auto it = params.find(paramKey);
      if (it == params.end())
      {
        return std::string();
      }

      return it->second;
    }

  private: // methods

    void ParseCollections(std::string currentPath)
    {
      auto collectionEnd = std::size_t();

      for (auto collectionStart = currentPath.find('/');
        collectionStart != std::string::npos;
        collectionStart = collectionEnd)
      {
        auto collectionNameStart = collectionStart + 1;
        auto collectionEnd = currentPath.find('/', collectionNameStart);
        if (collectionEnd == std::string::npos)
        {
          resource = currentPath.substr(collectionNameStart);
          break;
        }

        collections.push_back(currentPath.substr(collectionNameStart, collectionEnd - collectionNameStart));
        currentPath = currentPath.substr(collectionEnd);
      }
    }

    static HttpMethod::Value ParseMethod(std::string const& method)
    {
      if (method == "GET")
      {
        return HttpMethod::Get;
      }
      else if (method == "PUT")
      {
        return HttpMethod::Put;
      }
      else if (method == "POST")
      {
        return HttpMethod::Post;
      }
      else if (method == "DELETE")
      {
        return HttpMethod::Delete;
      }

      return HttpMethod::Unknown;
    }

    void ParseParameters(std::istringstream& istream)
    {
      // Read all parameters defined in the header.
      while (istream.good())
      {
        std::string key, value;

        istream >> key;
        if (key.empty())
        {
          break;
        }
        key.pop_back(); // removing ':'

        // Seek through non-alphanumeric text (usually whitespace).
        while (!std::isalnum(istream.peek()))
        {
          istream.ignore();
          if (!istream.good())
          {
            break;
          }
        }

        static const std::size_t bufferSize = 16u * 1024u;
        auto buffer = std::array<char, bufferSize>();
        istream.getline(buffer.data(), bufferSize);
        value = std::string(buffer.data(), static_cast<unsigned>(istream.gcount()));
        while (value.size() && !std::isalnum(value.back()))
        {
          value.pop_back();
        }

        auto kvPair = std::make_pair(std::move(key), std::move(value));
        params.emplace(std::move(kvPair));
      }
    }

    void ParseQueries(std::string queryString)
    {
      auto kvPairs = std::vector<std::string>();

      // Parse all key-value pairs in the query string.
      auto ampersand = queryString.find('&');
      while (ampersand != std::string::npos)
      {
        kvPairs.push_back(queryString.substr(0, ampersand));
        queryString = queryString.substr(ampersand + 1);
        ampersand = queryString.find('&');
      }

      // Make sure to save the last key-value pair that was parsed.
      kvPairs.push_back(queryString);

      // Read all key-value pairs as query fields.
      for (auto it = kvPairs.begin(); it != kvPairs.end(); ++it)
      {
        auto& kvPair = *it;
        auto equals = kvPair.find('=');
        if (equals == std::string::npos)
        {
          queries.emplace(std::make_pair(std::string(), kvPair));
          continue;
        }

        queries.emplace(std::make_pair(kvPair.substr(0, equals), kvPair.substr(equals + 1)));
      }
    }
  };
} // namespace OlympusWebServer