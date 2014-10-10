#pragma once

#include <string>

namespace OlympusWebServer
{
  namespace HttpDataType
  {
    enum Value
    {
      Json,
      Html
    };
  }

  namespace HttpMethod
  {
    enum Value
    {
      Get,
      Put,
      Post,
      Delete,
      Unknown
    };
  }

  namespace HttpStatus
  {
    enum Value
    {
      Continue = 100,
      Ok = 200,
      Created = 201,
      BadRequest = 400,
      Unauthorized = 401,
      Forbidden = 403,
      NotFound = 404,
      ServerError = 500,
      ServiceUnavailable = 503
    };
  }

  inline std::string ToString(HttpStatus::Value status)
  {
    switch (status)
    {
    case HttpStatus::Continue:            return "100 Continue";
    case HttpStatus::Ok:                  return "200 OK";
    case HttpStatus::Created:             return "201 Created";
    case HttpStatus::BadRequest:          return "400 Bad Request";
    case HttpStatus::Unauthorized:        return "401 Unauthorized";
    case HttpStatus::Forbidden:           return "403 Forbidden";
    case HttpStatus::NotFound:            return "404 Not Found";
    case HttpStatus::ServerError:         return "500 Server Error";
    case HttpStatus::ServiceUnavailable:  return "503 Service Unavailable";
    default:                              return 0;
    }
  }
} // namespace OlympusWebServer