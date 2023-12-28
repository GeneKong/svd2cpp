#include "Builders.hpp"

#include <bitset>
#include <iostream>
#include <fmt/core.h>

std::string toCamelCase(const std::string& str)
{
    std::stringstream ss;
    bool capitalize = true;
    for (char c : str)
    {
        if (c == ' ' || c == '_')
        {
            capitalize = true;
        }
        else if (capitalize)
        {
            ss << static_cast<char>(std::toupper(c));
            capitalize = false;
        }
        else
        {
            ss << static_cast<char>(std::tolower(c));
        }
    }
    // Check if the last character of the original string is an underscore
    if (!str.empty() && str.back() == '_')
    {
        // If so, append an underscore to the result string
        ss << '_';
    }
    return ss.str();
}

void NSBeginBuilder::build(std::stringstream &ss) const
{
    ss << fmt::format(
        "#pragma once\n"
        "#include \"RegBase.h\"\n\n"
        "namespace FEmbed {{\n"
        );
}

void NSEnduilder::build(std::stringstream& ss ) const
{
    ss << fmt::format("}};\n");
}

void FieldDefineBuilder::build( std::stringstream& ss ) const
{
    // Do nothing...
}

void PeripheralBuilder::build( std::stringstream& ss ) const
{
//    ss << "namespace " << peripheral.name << "{\n";
//    for( auto& registe : peripheral.registers ) {
//        RegisterBuilder( registe, peripheral.baseAddress ).build( ss );
//    }
//    ss << "}\n\n";
    ss <<   "template<typename _T = uint32_t, _T BaseAddr = 0x" << std::hex << peripheral.baseAddress << std::dec << ">\n"
            "class " << toCamelCase(peripheral.name) << " {\n"
            "  public:";
        for( auto& registe : peripheral.registers ) {
        RegisterBuilder( registe, peripheral.baseAddress ).build( ss );
    }
    ss << "};\n\n";

}

void RegisterBuilder::build( std::stringstream& ss ) const
{
    ss <<
    "    class " << toCamelCase(registe.name) << " : public Register<_T, BaseAddr + " << registe.addressOffset << "> {\n"
    "      public:\n"
    "        constexpr static _T RegisterAddr =  BaseAddr + " << registe.addressOffset << ";\n"
    "        constexpr static inline unsigned int address() { return RegisterAddr; }\n";

    for( auto& field : registe.fields ) {
        FieldBuilder( field, getRegisterAddress() ).build( ss );
    }
    ss <<
    "    } " << registe.name << ";\n\n";
}

unsigned int RegisterBuilder::getRegisterAddress() const
{
    return baseAddress + registe.addressOffset;
}

void FieldBuilder::build( std::stringstream& ss ) const
{
    ss <<
    "        Filed<_T, RegisterAddr, "<< field.bitOffset << ", "<< field.bitWidth <<"> " << field.name << ";\n";
}

unsigned int FieldBuilder::getAddress() const
{
    return registerAddress;
}

void FunctionsBuilder::build( std::stringstream& ss ) const
{

}
