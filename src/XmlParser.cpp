#include "XmlParser.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>

XmlParser::XmlParser( const std::string& inputFile )
{
    xmlDocument.LoadFile( inputFile.c_str() );
}
std::optional< std::string > XmlParser::isError() const
{
    return xmlDocument.Error() ? std::optional< std::string >( xmlDocument.ErrorStr() ) : std::nullopt;
}

void XmlParser::parseXml()
{
    tinyxml2::XMLElement* deviceRoot = xmlDocument.FirstChildElement( "device" );
    if( deviceRoot == nullptr )
        return;
    //Set Schema Version
    deviceInfo.schemaVersion = deviceRoot->Attribute( "schemaVersion" );

    //Set attributes
    setDeviceInfoAttrib( deviceRoot, "name", deviceInfo.name );
    setDeviceInfoAttrib( deviceRoot, "version", deviceInfo.version );
    setDeviceInfoAttrib( deviceRoot, "resetValue", deviceInfo.resetValue );

    // deviceInfo.printDeviceInfo();

    //Iterate over all peripherals and append them to peripherals
    tinyxml2::XMLElement* peripheralsRoot = deviceRoot->FirstChildElement( "peripherals" );
    if( peripheralsRoot != nullptr ) {
        for( tinyxml2::XMLNode* peripheralRoot = peripheralsRoot->FirstChild(); peripheralRoot;
             peripheralRoot = peripheralRoot->NextSibling() ) {
            //Parse only "peripheral" node
            if( std::string( peripheralRoot->Value() ) != "peripheral" ) {
                SPDLOG_WARN("Register node has value {}", peripheralRoot->Value());
                continue;
            }

            peripherals.push_back( parsePeripheral( peripheralRoot->ToElement()) );
        }
    }
}

void XmlParser::setDeviceInfoAttrib( tinyxml2::XMLElement* deviceRoot,
    const char* name,
    std::string& field ) const
{
    tinyxml2::XMLElement* deviceEntry = deviceRoot->FirstChildElement( name );
    field = deviceEntry ? std::string( deviceEntry->GetText() ) : noValue;
}

void XmlParser::setDeviceInfoAttrib( tinyxml2::XMLElement* deviceRoot,
    const char* name,
    unsigned int& field ) const
{
    tinyxml2::XMLElement* deviceEntry = deviceRoot->FirstChildElement( name );
    if(deviceEntry)
    {
        std::string text = deviceEntry->GetText();
        if(text.find("0x") != std::string::npos)
        {
            field = std::stoul( deviceEntry->GetText(), 0, 16 );
        }
        else
        {
            field = std::stoul( deviceEntry->GetText() );
        }
    }
    else
    {
        field = 0;
    }
}
void XmlParser::setDeviceInfoAttrib( tinyxml2::XMLElement* deviceRoot,
    const char* name,
    EAccess& field ) const
{
    tinyxml2::XMLElement* deviceEntry = deviceRoot->FirstChildElement( name );
    if( deviceEntry == nullptr ) {
        field = EAccess::ReadWrite;
        return;
    }
    const std::string text = deviceEntry->GetText();
    if( text == "read-only" ) {
        field = EAccess::ReadOnly;
    }
    else if( text == "write-only" ) {
        field = EAccess::WriteOnly;
    }
    else if( text == "read-write" ) {
        field = EAccess::ReadWrite;
    }
    else {
        SPDLOG_WARN("Wrong field for access: {}", text);
    }
}
Peripheral XmlParser::parsePeripheral( tinyxml2::XMLElement* peripheralRoot ) const
{
    //Check if peripheral is derived from previous one
    const char* attribute = peripheralRoot->Attribute( "derivedFrom" );
    const bool isDerived = attribute != nullptr;
    const std::string derivedFrom = isDerived ? attribute : "";

    Peripheral peripheral;
    if( isDerived == true ) {
        //Find the base peripheral and copy it to the new one
        auto crit = [&]( auto& periph ) { return periph.name == derivedFrom; };
        auto resultIt = std::find_if( peripherals.begin(), peripherals.end(), crit );
        if( resultIt <= peripherals.end() ) {
            peripheral = Peripheral( *resultIt );
        }
        else {
            SPDLOG_WARN("Couldn't find peripheral {}", derivedFrom);
        }
    }
    setDeviceInfoAttrib( peripheralRoot, "name", peripheral.name );
    setDeviceInfoAttrib( peripheralRoot, "baseAddress", peripheral.baseAddress );
    if( isDerived == false ) {
        setDeviceInfoAttrib( peripheralRoot, "description", peripheral.description );
        setDeviceInfoAttrib( peripheralRoot, "groupName", peripheral.groupName );
        peripheral.addressBlock = parseAddressBlock( peripheralRoot->FirstChildElement( "addressBlock" ) );
        if(peripheral.addressBlock.size == 0)
        {
            SPDLOG_INFO("{} has no addressBlock", peripheral.name);
        }

        //Iterate over all registers and append them to peripheral
        tinyxml2::XMLElement* registersRoot = peripheralRoot->FirstChildElement( "registers" );
        if( registersRoot != nullptr ) {
            for( tinyxml2::XMLNode* registerRoot = registersRoot->FirstChild(); registerRoot;
                 registerRoot = registerRoot->NextSibling() ) {
                // Parse only "register/cluster" node
                if ( std::string( registerRoot->Value() ) == "cluster" ) {
                    // Parse register with dim, dimIncrement, dimIndex
                    unsigned int dim{1};
                    unsigned int dimIncrement{0};
                    std::string dimIndex{};
                    std::vector<std::string> dimIndexV{};
                    std::string dimName{};
                    std::string dimDescription{};
                    setDeviceInfoAttrib( registerRoot->ToElement(), "dim", dim );
                    setDeviceInfoAttrib( registerRoot->ToElement(), "dimIncrement", dimIncrement );
                    setDeviceInfoAttrib( registerRoot->ToElement(), "dimIndex", dimIndex );
                    setDeviceInfoAttrib( registerRoot->ToElement(), "name", dimName );
                    setDeviceInfoAttrib( registerRoot->ToElement(), "description", dimDescription );

                    if(dim > 1) {
                        // dimIndex like 0-1,5,7-8, need paser to 0,1,5,7,8
                        // then push it into dimIndexV
                        std::stringstream ss(dimIndex);
                        std::string item;
                        while (std::getline(ss, item, ',')) {
                            size_t dashPos = item.find('-');
                            if (dashPos != std::string::npos) {
                                int start = std::stoi(item.substr(0, dashPos));
                                int end = std::stoi(item.substr(dashPos + 1));
                                for (int i = start; i <= end; ++i) {
                                    dimIndexV.push_back(std::to_string(i));
                                }
                            } else {
                                dimIndexV.push_back(item);
                            }
                        }
                    }
                    else {
                        dimIndexV.push_back("");
                    }

                    for( tinyxml2::XMLNode* dimRegisterRoot = registerRoot->FirstChildElement( "register" ); dimRegisterRoot;
                         dimRegisterRoot = dimRegisterRoot->NextSibling() ) {

                        auto reg = parseRegister( dimRegisterRoot->ToElement() );

                        reg.dimName = dimName;
                        reg.dimDescription = dimDescription;
                        reg.dim = dim;
                        reg.dimIncrement = dimIncrement;
                        reg.dimIndex = dimIndexV;

                        peripheral.registers.push_back( reg );
                    }
                }
                else if( std::string( registerRoot->Value() ) != "register" ) {
                    SPDLOG_WARN("Peripheral {} has register value {}", peripheral.name, registerRoot->Value());
                    continue;
                }
                else {
                    peripheral.registers.push_back(parseRegister(registerRoot->ToElement()));
                }
            }
        }
    }
    // peripheral.display();
    return peripheral;
}

AddressBlock XmlParser::parseAddressBlock( tinyxml2::XMLElement* addressBlockRoot ) const
{
    AddressBlock addressBlock{};
    if( addressBlockRoot != nullptr ) {
        setDeviceInfoAttrib( addressBlockRoot, "offset", addressBlock.offset );
        setDeviceInfoAttrib( addressBlockRoot, "size", addressBlock.size );
    }
    else {
        SPDLOG_DEBUG("addressBlockRoot is nullptr");
    }
    return addressBlock;
}

Register XmlParser::parseRegister( tinyxml2::XMLElement* registerRoot ) const
{
    Register registe{};
    setDeviceInfoAttrib( registerRoot, "name", registe.name );
    setDeviceInfoAttrib( registerRoot, "description", registe.description );
    setDeviceInfoAttrib( registerRoot, "addressOffset", registe.addressOffset );
    setDeviceInfoAttrib( registerRoot, "size", registe.size );
    setDeviceInfoAttrib( registerRoot, "access", registe.registerAccess );
    setDeviceInfoAttrib( registerRoot, "resetValue", registe.resetValue );

    //Iterate over all fields and append them to registe
    tinyxml2::XMLElement* fieldsRoot = registerRoot->FirstChildElement( "fields" );
    if( fieldsRoot != nullptr ) {
        for( tinyxml2::XMLNode* fieldRoot = fieldsRoot->FirstChild(); fieldRoot;
             fieldRoot = fieldRoot->NextSibling() ) {
            //Parse only "field" node
            if( std::string( fieldRoot->Value() ) != "field" ) {
                SPDLOG_WARN("Field node has value {}", fieldRoot->Value());
                continue;
            }
            registe.fields.push_back( parseField( fieldRoot->ToElement() ) );
        }
    }
    return registe;
}

Field XmlParser::parseField( tinyxml2::XMLElement* fieldRoot ) const
{
    Field field;
    setDeviceInfoAttrib( fieldRoot, "name", field.name );
    setDeviceInfoAttrib( fieldRoot, "description", field.description );
    setDeviceInfoAttrib( fieldRoot, "bitOffset", field.bitOffset );
    setDeviceInfoAttrib( fieldRoot, "bitWidth", field.bitWidth );
    setDeviceInfoAttrib( fieldRoot, "access", field.fieldAccess );

    // paser all enumeratedValues and process for read,write
    tinyxml2::XMLElement* enumeratedValuesRoot = fieldRoot->FirstChildElement( "enumeratedValues" );
    if( enumeratedValuesRoot != nullptr ) {
        //Parse name and usage then multi enumeratedValue field
        std::string usage;
        Enum enumeratedValues{};

        setDeviceInfoAttrib( enumeratedValuesRoot->ToElement(), "name", enumeratedValues.name );
        setDeviceInfoAttrib( enumeratedValuesRoot->ToElement(), "usage", usage );

        if( usage == "read" ) {
            enumeratedValues.usage = EnumUsage::Read;
        }
        else if( usage == "write" ) {
            enumeratedValues.usage = EnumUsage::Write;
        }
        else {
            enumeratedValues.usage = EnumUsage::ReadWrite;
        }

        //Iterate over all enumeratedValue and append them to field
        for( tinyxml2::XMLNode* valueRoot = enumeratedValuesRoot->FirstChildElement( "enumeratedValue" ); valueRoot;
             valueRoot = valueRoot->NextSibling() ) {

            if(valueRoot != nullptr) {
                //Parse only "name" node
                if (std::string(valueRoot->Value()) != "enumeratedValue") {
                    SPDLOG_WARN("enumeratedValue node has value {}", valueRoot->Value());
                    continue;
                }

                EnumValue enumeratedValue{};
                setDeviceInfoAttrib(valueRoot->ToElement(), "name", enumeratedValue.name);
                setDeviceInfoAttrib(valueRoot->ToElement(), "description", enumeratedValue.description);
                setDeviceInfoAttrib(valueRoot->ToElement(), "value", enumeratedValue.value);

                enumeratedValues.values.push_back(enumeratedValue);
            }
        }


        field.enums.push_back(enumeratedValues);
    }

    return field;
}
