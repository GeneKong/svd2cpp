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
        "namespace pac {{\n"
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
    ss << fmt::format(
        "template<typename _T = uint32_t, _T BaseAddr = 0x{:x}>\n"
        "class {} {{\n"
        "  public:\n",
        peripheral.baseAddress,
        toCamelCase(peripheral.name)
        );
        for( auto& registe : peripheral.registers ) {
        RegisterBuilder( registe, peripheral.baseAddress ).build( ss );
    }
    ss << "};\n\n";

}

void RegisterBuilder::build( std::stringstream& ss ) const
{
    ss << fmt::format(
        "    class {0} : public Register<_T, BaseAddr + {1}, AccessType::{2}> {{\n"
        "      public:\n"
        "        constexpr static _T RegisterAddr =  BaseAddr + {1};\n"
        "        constexpr static inline unsigned int address() {{ return RegisterAddr; }}\n",
        toCamelCase(registe.name),
        registe.addressOffset,
        registe.registerAccess.toString()
        );

    for( auto& field : registe.fields ) {
        FieldBuilder( field, getRegisterAddress() ).build( ss );
    }
    ss << fmt::format("    }} {};\n\n", registe.name);
}

unsigned int RegisterBuilder::getRegisterAddress() const
{
    return baseAddress + registe.addressOffset;
}

void FieldBuilder::build( std::stringstream& ss ) const
{
    ss << fmt::format(
    "        Filed<_T, RegisterAddr, {}, {}, AccessType::{}> {};\n",
    field.bitOffset,
    field.bitWidth,
    field.fieldAccess.toString(),
    field.name
    );
}

unsigned int FieldBuilder::getAddress() const
{
    return registerAddress;
}

void FunctionsBuilder::build( std::stringstream& ss ) const
{

}
