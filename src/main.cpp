#include "FileBuilder.hpp"
#include "XmlParser.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/sinks/wincolor_sink.h>

#include <cxxopts.hpp>
#include <argparse/argparse.hpp>

#include <iostream>
#include <fstream>

int main( int argc, char** argv )
{
    #if defined(__linux__) || defined(__linux)
    auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern("[storage] [%^%l%$] %s:%# %v");
    #else
    auto console_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern("[%^%l%$] %s:%# %v");
    #endif

    std::shared_ptr<spdlog::logger> logger = nullptr;
    logger.reset(new spdlog::logger("svd2cpp", { console_sink }));
    logger->set_level(spdlog::level::debug);

    spdlog::register_logger(logger);
    spdlog::flush_every(std::chrono::seconds(5));
    spdlog::set_default_logger(logger);

    // Create and configure options for the program
    cxxopts::Options options( "svd2cpp", "Parser from svd files to C++ header" );
    options.add_options()(
        "i, input", "File with .svd extention to be parsed", cxxopts::value< std::string >() )(
        "o, output", "Output file", cxxopts::value< std::string >() )( "h, help", "Print help" );

    std::string inputFile, outputFile;
    auto result = options.parse( argc, argv );
    try {
        if( result.count( "help" ) ) {
            std::cout << options.help() << std::endl;
            return 0;
        }
        if( result.count( "input" ) != 1 ) {
            std::cout << "Missing input file!" << std::endl;
            return 1;
        }
        if( result.count( "output" ) != 1 ) {
            std::cout << "Missing output file!" << std::endl;
            return 1;
        }
        inputFile = result["input"].as< std::string >();
        outputFile = result["output"].as< std::string >();
        // std::cout << "Input: " << inputFile << "\tOutput: " << outputFile << std::endl;
    }
    catch( const std::exception& ex ) {
        std::cout << ex.what() << std::endl;
        return 2;
    }
    //Try to parse the file
    XmlParser xmlParser( inputFile );
    if( auto err = xmlParser.isError() ) {
        std::cout << "There was an error while reading " << inputFile << ":" << std::endl
                  << *err << std::endl;
        return 3;
    }
    xmlParser.parseXml();
    FileBuilder classBuilder(result, xmlParser.getDeviceInfo(), xmlParser.getPeripherals() );
    classBuilder.setupBuilders();
    classBuilder.build();

    /* OutputFile oFile(outputFile); */
    /* std::cout << (oFile.save(classBuilder.getStream()) ? "Successfuly created " : "Failed to create ") << outputFile << std::endl; */
    std::ofstream oFile(outputFile);
    oFile << classBuilder.getStream().str() << std::endl;
    oFile.close();

    return 0;
}
