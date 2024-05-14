#include "Builders.hpp"
#include "fmt/format.h"

#include <bitset>
#include <cassert>
#include <fmt/core.h>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <map>

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

std::string removeTailDigit(const std::string &str)
{
    std::string result = str;
    while(result.back() >= '0' && result.back() <= '9')
    {
        result.pop_back();
    }
    return result;
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
    std::vector< std::unique_ptr< RegisterBuilder > > builders;

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
                    //RegisterBuilder( reg, peripheral.baseAddress).buildNormal( ss );
                    builders.push_back(std::make_unique<RegisterBuilder>( reg, peripheral.baseAddress));
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
                        //RegisterBuilder( reg, peripheral.baseAddress).buildNormal( ss );
                        builders.push_back(std::make_unique<RegisterBuilder>(reg, peripheral.baseAddress));
                    }
                }
            }
        }
        else {
            // prebuild: auto check register array
            //RegisterBuilder(registe, peripheral.baseAddress).buildNormal( ss );
            builders.push_back(std::make_unique<RegisterBuilder>(registe, peripheral.baseAddress));
        }
    }

    // generate build register source...
    struct RegisterInfo {
        uint32_t arrayOffMin;
        uint32_t arrayOffMax;
        uint32_t arrayNum;
        uint32_t base;
    };

    std::map<size_t , std::string> hashcodeMap;
    std::map<std::string, RegisterInfo> regInfoMap;
    std::map<std::string, std::string> regAlias;

    for(auto &build : builders) {
        build->preBuild();

//        fmt::println("{} 0x{:08x}", build->getName(), build->getHashCode());

        auto cname = build->getName();
        if(!hashcodeMap.contains(build->getHashCode())) {

            hashcodeMap[build->getHashCode()] = cname;
            regAlias[cname] = cname;
            regInfoMap[cname] = { build->getRegisterOffset(),
                                  build->getRegisterOffset(),
                                  1,
                                  build->getRegisterAddress() };
        }
        else {
            regAlias[cname] = hashcodeMap[build->getHashCode()];
            cname = hashcodeMap[build->getHashCode()];
            regInfoMap[cname].arrayOffMin = std::min(build->getRegisterOffset(), regInfoMap[cname].arrayOffMin);
            regInfoMap[cname].arrayOffMax = std::max(build->getRegisterOffset(), regInfoMap[cname].arrayOffMax);
            regInfoMap[cname].arrayNum ++;
        }
    }

    for(auto &build : builders) {
        build->buildNormal(ss);
    }

    // process array register
    for(auto &[reg, item] : regInfoMap) {
        if(item.arrayNum > 1) {
            std::string comment = "";
            for(auto [k, v] : regAlias)
            {
                if(std::equal(v.begin(), v.end(), reg.begin(), reg.end()))
                {
                    comment += k + ", ";
                }
            }
            ss << fmt::format( "    // {1}_array items are: {4}\n"
                               "    {0} {1}_array(size_t idx, std::optional<uint32_t> v = std::nullopt) const {{\n"
                               "         if (v.has_value())\n"
                               "             {0}(base_addr_ + 0x{2:x} + 0x{3:x}*idx, v.value(), 0, 32).store();\n"
                               "         return {0}(base_addr_ + 0x{2:x} + 0x{3:x}*idx).load(); \n"
                               "     }}; \n\n",
                               toCamelCase( reg ),
                               toLowerCase( removeTailDigit( reg ) ),
                               item.arrayOffMin,
                               (item.arrayOffMax - item.arrayOffMin)/(regInfoMap[reg].arrayNum - 1),
                               comment);
        }
    }

    ss << "};\n\n";
}

void RegisterBuilder::preBuild()
{
    auto reg = registe;
    reg.name = "CheckSame";
    reg.addressOffset = 0;
    RegisterBuilder(reg, baseAddress).buildTemplate( outputStream );
    std::hash<std::string> hasher;
    hashcode = hasher(outputStream.str());
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
                       "        }}\n"
                       "        auto zeroValue() const {{\n"
                       "            return CurrentRegister(addr_, 0, 0, 32);\n"
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
    ss << fmt::format( "    {0} {1}(std::optional<uint32_t> v = std::nullopt) const {{\n"
                       "         if (v.has_value())\n"
                       "             {0}(base_addr_ + 0x{2:x}, v.value(), 0, 32).store();\n"
                       "         return {0}(base_addr_ + 0x{2:x}).load(); \n"
                       "     }}; \n\n",
                       toCamelCase( registe.name ),
                       toLowerCase( registe.name ),
                       registe.addressOffset);
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
        ss << fmt::format( "        constexpr static Filed<_T, RegisterAddr, {0}, {1}, AccessType::{2}, {3}, {4}> {5}{{}};\n"
                           "        constexpr auto {6}({4} v) const\n"
                           "        {{\n"
                           "            return CurrentRegister({5}.evalSet(chain_init_, v));\n"
                           "        }}\n",
            field.bitOffset,
            field.bitWidth,
            field.fieldAccess.toString(),
            read_E,
            write_E,
            toUpperCase(field.name),
            toLowerCase(field.name)
            );
    }
    else {
        ss << fmt::format( "        constexpr static Filed<_T, RegisterAddr, {0}, {1}, AccessType::{2}> {3}{{}};\n"
                           "        constexpr auto {4}(_T v) const\n"
                           "        {{\n"
                           "            return CurrentRegister({3}.evalSet(chain_init_, v));\n"
                           "        }}\n",
            field.bitOffset,
            field.bitWidth,
            field.fieldAccess.toString(),
            toUpperCase(field.name),
            toLowerCase(field.name));
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
        ss << fmt::format( "        using {6} = Filed<uint32_t, 0x{0:08x}, {1}, {2}, AccessType::{3}, {4}, {5}>;\n"
                           "        constexpr auto {7}(std::optional<{5}> v = std::nullopt) const\n"
                           "        {{\n"
                           "            constexpr static {6} test{{}};\n"
                           "            if(v.has_value()) {{\n"
                           "                return CurrentRegister(addr_, test.evalSet(chain_init_, v, {1}, {2}));\n"
                           "            }} else {{\n"
                           "                return CurrentRegister(addr_, chain_init_, {1}, {2});\n"
                           "            }}\n"
                           "        }}\n",
                           getAddress(),
                           field.bitOffset,
                           field.bitWidth,
                           field.fieldAccess.toString(),
                           read_E,
                           write_E,
                           toUpperCase(field.name),
                           toLowerCase(field.name)
        );
    }
    else {
        ss << fmt::format(
                           "        using {4} = Filed<uint32_t, 0x{0:08x}, {1}, {2}, AccessType::{3}>;\n"
                           "        constexpr auto {5}(std::optional<uint32_t> v = std::nullopt) const\n"
                           "        {{\n"
                           "            constexpr static {4} test{{}};\n"
                           "            if(v.has_value()) {{\n"
                           "                return CurrentRegister(addr_, test.evalSet(chain_init_, v.value()), {1}, {2});\n"
                           "            }} else {{\n"
                           "                return CurrentRegister(addr_, chain_init_, {1}, {2});\n"
                           "            }}\n"
                           "        }}\n",
                           getAddress(),
                           field.bitOffset,
                           field.bitWidth,
                           field.fieldAccess.toString(),
                           toUpperCase(field.name),
                           toLowerCase(field.name)
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
