#include "Builders.hpp"
#include "fmt/format.h"

#include <bitset>
#include <cassert>
#include <fmt/core.h>
#include <iostream>
#include <algorithm>

std::string toCamelCase( const std::string& str, bool raw_keep = false)
{
    std::stringstream ss;
    bool capitalize = true;
    for( char c : str ) {
        if( c == ' ' || c == '_' ) {
            capitalize = true;
            continue;
        }
        if( capitalize ) {
            ss << static_cast< char >( std::toupper( c ) );
            capitalize = false;
        }
        else if(raw_keep) {
            ss << static_cast< char >( c ); // Keep
        }
        else {
            ss << static_cast< char >( std::tolower( c ) );
        }

        if( isdigit( c ) )
            capitalize = true;
    }
    // Check if the last character of the original string is an underscore
    if( !str.empty() && str.back() == '_' ) {
        // If so, append an underscore to the result string
        ss << '_';
    }
    return ss.str();
}

std::string toLowerCase(const std::string &str)
{
    std::stringstream ss;
    for( char c : str ) {
        ss << static_cast< char >( std::tolower( c ) );
    }
    if(ss.str() == "class" ||
       ss.str() == "or" ||
       ss.str() == "do" ||
       ss.str() == "asm" ||
       ss.str() == "for" ||
       ss.str() == "if" )
    {
        ss << "_";
    }
    return ss.str();
}

std::string toUpperCase(const std::string &str)
{
    std::stringstream ss;
    for( char c : str ) {
        ss << static_cast< char >( std::toupper( c ) );
    }
    return ss.str();
}

void NSBeginBuilder::build( std::stringstream& ss ) const
{
    ss << fmt::format( "#pragma once\n"
                       "#include \"RegBase.h\"\n\n"
                       "namespace pac {{\n" );
}

void NSEnduilder::build( std::stringstream& ss ) const
{
    ss << fmt::format( "}};\n" );
}

void FieldDefineBuilder::build( std::stringstream& ss ) const
{
    // Do nothing...
}

std::vector<std::string> fieldEnums {};
void PeripheralBuilder::build( std::stringstream& ss ) const
{
    fieldEnums.clear();
    ss << fmt::format( "template<typename _T = uint32_t, _T BaseAddr = 0x{:x}>\n"
                       "class {} {{\n"
                       "  public:\n",
        peripheral.baseAddress,
        toCamelCase( peripheral.name ) );
    for( auto& registe : peripheral.registers ) {
        if( registe.dim ) {
            if( registe.registers.empty() ) {
                // process regsister array...
                for( int i = 0; i < registe.dim; i++ ) {
                    Register reg{ registe };
                    char buff[256];
                    if( registe.name.find( '%' ) != std::string::npos ) {
                        snprintf( buff, 255, registe.name.c_str(), registe.dimIndex[i].c_str() );
                        reg.name = std::string( buff );
                    }
                    reg.addressOffset += reg.addressOffset * i;
                    RegisterBuilder( reg, peripheral.baseAddress).build( ss );
                }
            }
            else {
                // process cluster registers
                for( int i = 0; i < registe.dim; i++ ) {
                    for( auto& creg : registe.registers ) {
                        Register reg{ creg };
                        char buff[256];
                        if( registe.dimName.find( '%' ) != std::string::npos ) {
                            snprintf( buff, 255, registe.dimName.c_str(), registe.dimIndex[i].c_str() );
                            reg.name = std::string( buff ) + reg.name;
                        }
                        else
                        {
                            reg.name = registe.dimName + reg.name;
                        }
                        reg.addressOffset += reg.dimIncrement * i + reg.addressOffset;
                        RegisterBuilder( reg, peripheral.baseAddress).build( ss );
                    }
                }
            }
        }
        else {
            RegisterBuilder( registe, peripheral.baseAddress).build( ss );
        }
    }
    ss << "};\n\n";
}

void RegisterBuilder::build( std::stringstream& ss ) const
{
    // process all enum class first then field define
    for( auto& field : registe.fields ) {
        if( field.enums.size() > 0 ) {
            for( auto& e : field.enums ) {
                auto resultIt = std::find_if( fieldEnums.begin(), fieldEnums.end(), [&]( const auto& item ) {
                  return item == e.name;
                });
                if(resultIt == fieldEnums.end())
                {
                    ss << fmt::format( "    enum class {} {{\n", "E_" + e.name);
                    for( auto& v : e.values ) {
                        ss << fmt::format( "        {} = {},\n", toCamelCase(v.name, true), v.value );
                    }
                    ss << fmt::format( "    }};\n" );
                    fieldEnums.push_back(e.name);
                }
            }
        }
    }
    ss << "\n";

    ss << fmt::format( "    class {0} : public Register<_T, BaseAddr + {1}, AccessType::{2}> {{\n"
                       "      public:\n"
                       "        constexpr static _T RegisterAddr =  BaseAddr + {1};\n"
                       "        constexpr static unsigned int address() {{ return RegisterAddr; }}\n\n"
                       "        using CurrentRegister = class {0};\n\n"
                       "        constexpr {0}(): chain_init_ {{0}} {{}}\n"
                       "        constexpr explicit {0}(_T v): chain_init_ {{v}} {{}}\n"
                       "        constexpr auto load() const {{\n"
                       "            _T v =  *(volatile _T *)RegisterAddr;\n"
                       "            return CurrentRegister(v);\n"
                       "        }}\n"
                       "        constexpr auto resetValue() const {{\n"
                       "            return CurrentRegister({3});\n"
                       "        }}\n",
        toCamelCase( registe.name ),
        registe.addressOffset,
        registe.registerAccess.toString(),
        registe.resetValue);

    // process all field instance in register
    for( auto& field : registe.fields ) {
        if( field.dim ) {
            // process regsister array...
            std::vector<std::string> field_array{};
            char buff[256];
            for( int i = 0; i < field.dim; i++ ) {
                Field f{ field };
                if( field.name.find( '%' ) != std::string::npos ) {
                    snprintf( buff, 255, field.name.c_str(), field.dimIndex[i].c_str() );
                    f.name = std::string( buff );
                }
                f.bitOffset += f.dimIncrement * i;
                FieldBuilder( f, getRegisterAddress() ).build( ss );
                field_array.push_back(f.name);
            }

            // process cluster registers
            snprintf( buff, 255, field.name.c_str(), "s" );
            ss << fmt::format("\n"
                              "        constexpr auto static {} = std::make_tuple({});\n", buff, fmt::join(field_array, ", "));
        }
        else {
            FieldBuilder( field, getRegisterAddress() ).build( ss );
        }
    }

    ss << "        constexpr void store() const {\n"
          "            *(volatile _T *)RegisterAddr = chain_init_;\n"
          "        }\n\n"
          "      private:\n"
          "        const _T chain_init_;\n";
    ss << fmt::format( "    }};\n", registe.name );
    ss << fmt::format( "    constexpr static {0} {1} {{}}; \n\n", toCamelCase( registe.name ), registe.name);
}

unsigned int RegisterBuilder::getRegisterAddress() const
{
    return baseAddress + registe.addressOffset;
}

void FieldBuilder::build( std::stringstream& ss ) const
{
    // Need process enumlate value
    std::string write_E = "_T";
    std::string read_E = "_T";
    if(!field.enums.empty())
    {
        for( auto& e : field.enums ) {
            if(e.usage == EnumUsage::Read)
            {
                read_E = "E_" + e.name;
            }
            else if(e.usage == EnumUsage::Write)
            {
                write_E = "E_" + e.name;
            }
            else {
                write_E = "E_" + e.name;
                read_E = "E_" + e.name;
            }
        }
    }

    if(write_E != "_T" || read_E != "_T")
    {
        ss << fmt::format( "        constexpr static Filed<_T, RegisterAddr, {}, {}, AccessType::{}, {}, {}> {}{{}};\n"
                           "        constexpr auto {}({} v) const\n"
                           "        {{\n"
                           "            return CurrentRegister({}.evalSet(chain_init_, v));\n"
                           "        }}\n",
            field.bitOffset,
            field.bitWidth,
            field.fieldAccess.toString(),
            read_E,
            write_E,
            toUpperCase(field.name),
            toLowerCase(field.name),
            write_E,
            toUpperCase(field.name)
            );
    }
    else {
        ss << fmt::format( "        constexpr static Filed<_T, RegisterAddr, {}, {}, AccessType::{}> {}{{}};\n"
                           "        constexpr auto {}(_T v) const\n"
                           "        {{\n"
                           "            return CurrentRegister({}.evalSet(chain_init_, v));\n"
                           "        }}\n",
            field.bitOffset,
            field.bitWidth,
            field.fieldAccess.toString(),
            toUpperCase(field.name),
            toLowerCase(field.name),
                           toUpperCase(field.name));
    }
}

unsigned int FieldBuilder::getAddress() const
{
    return registerAddress;
}

void FunctionsBuilder::build( std::stringstream& ss ) const {
    // Noncompliant - method is empty
}
