#pragma once
#include <string>

class IRequest
{
public:
   enum class RequestType
   {
      Http,
      Command,
      Grpc,
      Unknown
   };

   virtual ~IRequest() = default;
   virtual RequestType type() const = 0;

    // Core components
    virtual std::string method() const = 0;
    virtual std::string uri() const = 0;
    virtual std::string httpVersion() const = 0;

    // Headers / Body
    virtual std::string getHeader(const std::string& name) const = 0;
    virtual std::string body() const = 0;

    // Parameters
    virtual std::string getQueryParam(const std::string& name) const = 0;
    virtual std::string getPathParam(const std::string& name) const = 0;

    // Metadata
    virtual std::string remoteAddress() const = 0;

    // Command-line form (for non-HTTP input)
    virtual std::string get() const = 0;

};