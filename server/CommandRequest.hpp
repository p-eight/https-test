#pragma once
#include "Core/IRequest.hpp"

class CommandRequest : public IRequest
{
public:
   CommandRequest(const std::string raw_request)
   {
      
   };
   RequestType type() const { return RequestType::Grpc; };

    // Core components
    std::string method() const{ return "";};
    std::string uri() const{ return "";};
    std::string httpVersion() const{ return "";};

    // Headers / Body
    std::string getHeader(const std::string& name) const{ return "";};
    std::string body() const{ return "";};

    // Parameters
    std::string getQueryParam(const std::string& name) const{ return "";};
    std::string getPathParam(const std::string& name) const{ return "";};

    // Metadata
    std::string remoteAddress() const{ return "";};

    // Command-line form (for non-HTTP input)
    std::string get() const{ return "";};


private:
};