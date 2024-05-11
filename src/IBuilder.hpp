#pragma once

#include <sstream>

struct IBuilder
{
    virtual void buildTemplate( std::stringstream&) const = 0;
    virtual void buildNormal( std::stringstream&) const = 0;
    virtual ~IBuilder() = default;
};
