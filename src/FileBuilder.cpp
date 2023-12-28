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
    builders.push_back(std::make_unique<NSBeginBuilder>());
    builders.push_back( std::make_unique< FieldDefineBuilder >() );
    for( auto& peripheral : peripherals ) {
        builders.push_back( std::make_unique< PeripheralBuilder >( peripheral ) );
    }
    builders.push_back( std::make_unique< FunctionsBuilder >() );
    builders.push_back( std::make_unique<NSEnduilder>());
}

void FileBuilder::build()
{
    for( auto& builder : builders ) {
        builder->build( outputStream );
    }
}

const std::stringstream& FileBuilder::getStream() const
{
    return outputStream;
}
