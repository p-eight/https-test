#pragma once
#include <memory>
#include <string>
#include "Core/IRequest.hpp"
#include "HttpRequest.hpp"
#include "CommandRequest.hpp"
#include <vector>

class RequestFactory
{
public:
   static std::unique_ptr<IRequest> parse(std::string_view raw)
   {
      if (isHttp(raw))
      {
         return std::make_unique<HttpRequest>(std::string(raw));
      }
      return std::make_unique<CommandRequest>(std::string(raw));
   }
private:
   static bool isHttp(std::string_view raw)
   {
      static const std::vector<std::string> httpMethods = {
            "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH"
        };

      for (const auto& method : httpMethods) {
            if (raw.starts_with(method + " ")) {
                return true;
            }
        }
        return false;
   }
};