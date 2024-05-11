#pragma once

#include "IBuilder.hpp"
#include "Peripheral.hpp"

#include <sstream>

struct NSBeginBuilder : public IBuilder
{
    void buildTemplate( std::stringstream& ss ) const final;
    void buildNormal( std::stringstream& ss ) const final;
};

struct NSEnduilder : public IBuilder
{
    void buildTemplate( std::stringstream& ss ) const final;
    void buildNormal( std::stringstream& ss ) const final;
};

struct FieldDefineBuilder : public IBuilder
{
    void buildTemplate( std::stringstream& ss ) const final;
    void buildNormal( std::stringstream& ss ) const final;
};

struct PeripheralBuilder : public IBuilder
{
    explicit PeripheralBuilder( const Peripheral& peripheral_ , std::string alias_ = "")
        : peripheral( peripheral_ )
        , alias(alias_)
    {
    }

    void buildTemplate( std::stringstream& ss ) const final;
    void buildNormal( std::stringstream& ss ) const final;

    // Add pre-compile for check same periph define with different name
    void preBuild();

    size_t getHashCode()
    {
        return hashcode;
    }

    std::string getPeripheralName()
    {
        return peripheral.name;
    }

private:
    const Peripheral& peripheral;
    const std::string alias;
    std::stringstream outputStream;
    size_t hashcode;
};

struct RegisterBuilder : public IBuilder
{
    RegisterBuilder( const Register& register_, const unsigned int baseAddress_ )
        : registe( register_ )
        , baseAddress( baseAddress_ )
    {
    }

    void buildTemplate( std::stringstream& ss ) const final;
    void buildNormal( std::stringstream& ss ) const final;


    unsigned int getRegisterAddress() const;

private:
    const Register& registe;
    const unsigned int baseAddress;
};

struct FieldBuilder : public IBuilder
{
    FieldBuilder( const Field& field_, const unsigned int registerAddress_ )
        : field( field_ )
        , registerAddress( registerAddress_ )
    {
    }
    void buildTemplate( std::stringstream& ss ) const final;
    void buildNormal( std::stringstream& ss ) const final;


    unsigned int getAddress() const;

private:
    const Field& field;
    const unsigned int registerAddress;
};

struct FunctionsBuilder : public IBuilder
{
    void buildTemplate( std::stringstream& ss ) const final;
    void buildNormal( std::stringstream& ss ) const final;
};
