#pragma once

#include "IRequest.hpp"
#include "IResponse.hpp"
#include <memory>

class IRequestHandler
{
public:
    virtual ~IRequestHandler() = default;
    virtual std::unique_ptr<IResponse> handleRequest(const IRequest& request) = 0;
};