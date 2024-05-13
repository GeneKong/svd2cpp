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

void NSBeginBuilder::buildTemplate( std::stringstream& ss ) const
{
    ss << fmt::format( "#pragma once\n"
                       "#include \"RegBase.h\"\n"
                       "#include <optional>\n\n"
                       "namespace pac::tmpl {{\n" );
}

void NSEnduilder::buildTemplate( std::stringstream& ss ) const
{
    ss << fmt::format( "}};\n" );
}

void NSBeginBuilder::buildNormal( std::stringstream& ss ) const
{
    ss << fmt::format( "\n\nnamespace pac::nor {{\n" );
}

void NSEnduilder::buildNormal( std::stringstream& ss ) const
{
    ss << fmt::format( "}};\n" );
}

void FieldDefineBuilder::buildTemplate( std::stringstream& ss ) const
{
    // Do nothing...
}

void FieldDefineBuilder::buildNormal( std::stringstream& ss ) const
{
    // Do nothing...
}

void PeripheralBuilder::preBuild()
{
    this->buildTemplate( outputStream );

    // 获取 outputStream 的 hashcode
    std::stringstream newOutputStream;
    std::string line;
    int lineCount = 0;

    while (std::getline(outputStream, line)) {
        lineCount++;
        if (lineCount > 3) {
            newOutputStream << line << '\n';
        }
    }

    std::hash<std::string> hasher;
    hashcode = hasher(newOutputStream.str());
}

std::vector<std::string> fieldEnums {};
void PeripheralBuilder::buildTemplate( std::stringstream& ss ) const
{
    if(alias.compare(peripheral.name))
    {
        ss << fmt::format( "using {} = {}<uint32_t, 0x{:08x}>;\n",
            toCamelCase( peripheral.name ),
            toCamelCase( alias ),
            peripheral.baseAddress);
        return;
    }
    fieldEnums.clear();
    ss << fmt::format( "\ntemplate<typename _T = uint32_t, _T BaseAddr = 0x{:08x}>\n"
                       "class {} {{\n"
                       "  public:\n"
                       "  constexpr static _T BaseAddress = BaseAddr;\n",
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
                    RegisterBuilder( reg, peripheral.baseAddress).buildTemplate( ss );
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
                        RegisterBuilder( reg, peripheral.baseAddress).buildTemplate( ss );
                    }
                }
            }
        }
        else {
            RegisterBuilder( registe, peripheral.baseAddress).buildTemplate( ss );
        }
    }
    ss << "};\n\n";
}

void PeripheralBuilder::buildNormal( std::stringstream& ss ) const
{
    if(alias.compare(peripheral.name))
    {
        return;  // Do nothing for normal class defined...
    }

    fieldEnums.clear();

    ss << fmt::format( "\nclass {0} {{\n"
                       "  public:\n"
                       "    using PeriphClass = {0};\n"
                       "    const uint32_t base_addr_;\n"
                       "    explicit {0}(uint32_t baddr)\n"
                       "        : base_addr_(baddr)\n"
                       "    {{}}\n",
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
                    RegisterBuilder( reg, peripheral.baseAddress).buildNormal( ss );
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
                        RegisterBuilder( reg, peripheral.baseAddress).buildNormal( ss );
                    }
                }
            }
        }
        else {
            RegisterBuilder( registe, peripheral.baseAddress).buildNormal( ss );
        }
    }
    ss << "};\n\n";
}

void RegisterBuilder::buildTemplate( std::stringstream& ss ) const
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

    ss << fmt::format( "    class {0} : public TRegister<_T, BaseAddr + {1}, AccessType::{2}> {{\n"
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
                       "        constexpr auto chain_value() const {{\n"
                       "            return chain_init_;\n"
                       "        }}\n"
                       "        constexpr auto resetValue() const {{\n"
                       "            return CurrentRegister(0x{3:08x});\n"
                       "        }}\n",
        toCamelCase( registe.name ),
        registe.addressOffset,
        registe.registerAccess.toString(),
        registe.resetValue);

    // process all fields instance in register
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
                FieldBuilder( f, getRegisterAddress() ).buildTemplate( ss );
                field_array.push_back(f.name);
            }

            // process cluster registers
            snprintf( buff, 255, field.name.c_str(), "s" );
            ss << fmt::format("\n"
                              "        constexpr auto static {} = std::make_tuple({});\n", buff, fmt::join(field_array, ", "));
        }
        else {
            FieldBuilder( field, getRegisterAddress() ).buildTemplate( ss );
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

void RegisterBuilder::buildNormal( std::stringstream& ss ) const
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

    ss << fmt::format( "    class {0} : public Register<AccessType::{1}> {{\n"
                       "      public:\n"
                       "        using CurrentRegister = class {0};\n\n"
                       "        {0}() = delete;\n"
                       "        auto load() const {{\n"
                       "            uint32_t v =  *(volatile uint32_t *)(addr_);\n"
                       "            return CurrentRegister(addr_, v, 0, 32);\n"
                       "        }}\n"
                       "        constexpr auto chain_value() const {{\n"
                       "            return chain_init_;\n"
                       "        }}\n"
                       "        auto resetValue() const {{\n"
                       "            return CurrentRegister(addr_, 0x{2:08x}, 0, 32);\n"
                       "        }}\n",
        toCamelCase( registe.name ),
        registe.registerAccess.toString(),
        registe.resetValue);

    // process all fields instance in register
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
                FieldBuilder( f, getRegisterAddress() ).buildNormal( ss );
                field_array.push_back(f.name);
            }

            // process cluster registers
            snprintf( buff, 255, field.name.c_str(), "s" );
            ss << fmt::format("\n"
                               "        constexpr auto static {} = std::make_tuple({});\n", buff, fmt::join(field_array, ", "));
        }
        else {
            FieldBuilder( field, getRegisterAddress() ).buildNormal( ss );
        }
    }

    ss << fmt::format(

          "        void store() const {{\n"
          "            *(volatile uint32_t *)(addr_) = chain_init_;\n"
          "        }}\n\n"
          "      protected:\n"
          "        constexpr explicit {0}(uint32_t addr): \n"
          "            Register<AccessType::{1}>(addr, 0, 32), chain_init_ {{0}} {{}}\n"
          "        constexpr explicit {0}(uint32_t addr, uint32_t v, uint8_t offset, uint8_t width): \n"
          "            Register<AccessType::{1}>(addr, offset, width), chain_init_ {{v}} {{}}\n\n"
          "        friend PeriphClass;\n"
          "        const uint32_t chain_init_;\n",
        toCamelCase( registe.name ),
        registe.registerAccess.toString());
    ss << fmt::format( "    }};\n", registe.name );
    ss << fmt::format( "    {0} {1}() const {{ return {0}(base_addr_ + 0x{2:x}); }}; \n\n", toCamelCase( registe.name ), toLowerCase( registe.name ), registe.addressOffset);
}

unsigned int RegisterBuilder::getRegisterAddress() const
{
    return baseAddress + registe.addressOffset;
}

void FieldBuilder::buildTemplate( std::stringstream& ss ) const
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

void FieldBuilder::buildNormal( std::stringstream& ss ) const
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
        ss << fmt::format( "        constexpr auto {}(std::optional<{}> v) const\n"
                           "        {{\n"
                           "            constexpr static Filed<uint32_t, 0x{:08x}, {}, {}, AccessType::{}, {}, {}> {} {{}};\n"
                           "            if(v.has_value()) {{\n"
                           "                return CurrentRegister(addr_, {}.evalSet(chain_init_, v, {}, {}));\n"
                           "            }} else {{\n"
                           "                return CurrentRegister(addr_, chain_init_, {}, {});\n"
                           "            }}\n"
                           "        }}\n",
                           toLowerCase(field.name),
                           write_E,
                           getAddress(),
                           field.bitOffset,
                           field.bitWidth,
                           field.fieldAccess.toString(),
                           read_E,
                           write_E,
                           toUpperCase(field.name),
                           toUpperCase(field.name),
                           field.bitOffset,
                           field.bitWidth,
                           field.bitOffset,
                           field.bitWidth
        );
    }
    else {
        ss << fmt::format( "        constexpr auto {}(std::optional<uint32_t> v) const\n"
                           "        {{\n"
                           "            constexpr static Filed<uint32_t, 0x{:08x}, {}, {}, AccessType::{}> {} {{}};\n"
                           "            if(v.has_value()) {{\n"
                           "                return CurrentRegister(addr_, {}.evalSet(chain_init_, v.value()), {}, {});\n"
                           "            }} else {{\n"
                           "                return CurrentRegister(addr_, chain_init_, {}, {});\n"
                           "            }}\n"
                           "        }}\n",
                           toLowerCase(field.name),
                           getAddress(),
                           field.bitOffset,
                           field.bitWidth,
                           field.fieldAccess.toString(),
                           toUpperCase(field.name),
                           toUpperCase(field.name),
                           field.bitOffset,
                           field.bitWidth,
                           field.bitOffset,
                           field.bitWidth
                           );
    }
}

unsigned int FieldBuilder::getAddress() const
{
    return registerAddress;
}

void FunctionsBuilder::buildTemplate( std::stringstream& ss ) const {
    // Noncompliant - method is empty
}

void FunctionsBuilder::buildNormal( std::stringstream& ss ) const {
    // Noncompliant - method is empty
}
