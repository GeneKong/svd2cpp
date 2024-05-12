#include "FileBuilder.hpp"
#include "Builders.hpp"

FileBuilder::FileBuilder(const cxxopts::ParseResult& results_,
                         const DeviceInfo& deviceInfo_,
                         const std::vector< Peripheral >& peripherals_ )
    : results( results_ )
    , deviceInfo( deviceInfo_ )
    , peripherals( peripherals_ )
{
}

void FileBuilder::setupBuilders()
{
    // 1. prebuild Peripherals for check same periph define with different name
    std::unordered_map<size_t , std::string> hashcodeMap;
    std::unordered_map< std::string, std::string> periphNames;

    for( auto& peripheral : peripherals ) {
        auto builder = std::make_unique<PeripheralBuilder>(peripheral, peripheral.name);
        builder->preBuild();
        fmt::println("{} 0x{:08x}", builder->getPeripheralName(), builder->getHashCode());
        if(!hashcodeMap.contains(builder->getHashCode())) {
            hashcodeMap[builder->getHashCode()] = builder->getPeripheralName();
            periphNames[builder->getPeripheralName()] = builder->getPeripheralName();
        }
        else {
            periphNames[builder->getPeripheralName()] = hashcodeMap[builder->getHashCode()];
        }
    }


    // 2. do real code generated...
    builders.push_back(std::make_unique<NSBeginBuilder>());
    builders.push_back( std::make_unique< FieldDefineBuilder >() );
    for( auto& peripheral : peripherals ) {
        builders.push_back( std::make_unique< PeripheralBuilder >( peripheral, periphNames[peripheral.name]) );
    }
    builders.push_back( std::make_unique< FunctionsBuilder >() );
    builders.push_back( std::make_unique<NSEnduilder>());
}

void FileBuilder::build()
{
    for( auto& builder : builders ) {
        builder->buildTemplate( outputStream );
    }
    for( auto& builder : builders ) {
        builder->buildNormal( outputStream );
    }
}

const std::stringstream& FileBuilder::getStream() const
{
    return outputStream;
}
