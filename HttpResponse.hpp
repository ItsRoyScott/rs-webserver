#pragma once

#include "HttpTypes.hpp"
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace OlympusWebServer
{
  class HttpResponse
  {
  private: // data

    std::string data;
    HttpDataType::Value dataType;
    std::string fullResponse;
    float httpVersion;
    std::unordered_map<std::string, std::string> params;
    HttpStatus::Value status;

  public: // methods

    HttpResponse() :
      data("<html><heading>Hi there! I'm a web server for Olympus. :)</heading></html>"),
      dataType(HttpDataType::Html),
      status(HttpStatus::Ok)
    {
      FormatResponse();
    }

    HttpResponse(HttpResponse&& b)
    {
      *this = std::move(b);
    }

    HttpResponse& operator=(HttpResponse&& b)
    {
      data = std::move(b.data);
      dataType = b.dataType;
      fullResponse = std::move(b.fullResponse);
      httpVersion = b.httpVersion;
      params = std::move(b.params);
      status = b.status;

      return *this;
    }

    HttpResponse(
      std::string data_,
      HttpDataType::Value dataType_ = HttpDataType::Json,
      HttpStatus::Value status_ = HttpStatus::Ok) :
        dataType(dataType_),
        httpVersion(1.1f),
        status(status_)
    {
      FormatResponse();
    }

    explicit HttpResponse(HttpStatus::Value status_) :
      dataType(HttpDataType::Html),
      httpVersion(1.1f),
      status(status_)
    {
      FormatResponse();
    }

    std::string GetFormattedResponse() const
    {
      return fullResponse;
    }

  private: // methods

    void FormatResponse()
    {
      auto ostream = std::ostringstream();

      // Write the header.
      ostream << "HTTP/" << (httpVersion == 1.0f ? "1.0" : "1.1") << " ";
      ostream << ToString(status);
      for (auto it = params.begin(); it != params.end(); ++it)
      {
        auto& param = *it;
        ostream << std::endl << param.first << ": " << param.second;
      }
      ostream << std::endl;

      ostream << "Content-Type: ";
      switch (dataType)
      {
      case HttpDataType::Html: 
        ostream << "text/html;"; 
        break;
      case HttpDataType::Json:
        ostream << "application/json";
        break;
      default:
        throw std::runtime_error("HttpResponse.FormatResponse - Unknown HttpDataType being used");
      }

      ostream << "charset=utf-8" << std::endl;
      ostream << "Content-Length: " << data.size();

      ostream << std::endl << std::endl << std::move(data);
      fullResponse = ostream.str();
    }
  };
} // namespace OlympusWebServer