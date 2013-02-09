//
//  main.cpp
//  fbx2json
//
//  Created by Cameron Yule on 09/02/2013.
//  Copyright (c) 2013 Cameron Yule. All rights reserved.
//

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <string>
using namespace std;

const std::string VERSION = "1.0";

namespace
{
    const size_t ERROR_IN_COMMAND_LINE = 1;
    const size_t SUCCESS = 0;
    const size_t ERROR_UNHANDLED_EXCEPTION = 2;    
}

int main(int argc, char** argv)
{
    try
    {
        std::string input;
        std::string output;
        
        std::string appName = boost::filesystem::basename(argv[0]);
        std::string appVersion = appName + " " + VERSION;
        
        po::options_description desc("Options");
        desc.add_options()
        ("help", "Print help messages")
        ("input", po::value<std::string>(&input)->required(), "Input FBX file")
        ("output", po::value<std::string>(&output)->required(), "Output JSON file");
        
        po::positional_options_description positionalOptions;
        positionalOptions.add("input", 1);
        positionalOptions.add("output", 1);
        
        po::variables_map vm;

        try
        {
            po::store(po::command_line_parser(argc, argv).options(desc)
                      .positional(positionalOptions).run(),
                      vm);
            
            if ( vm.count("help")  )
            {
                std::cout << appVersion << std::endl << std::endl
                << "USAGE: " << appName << " input.fbx output.json" << std::endl << std::endl
                << desc << std::endl;
                return SUCCESS;
            }
            
            po::notify(vm);
        }
        catch(po::error& e)
        {
            std::cerr << appVersion << std::endl << std::endl;
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
            std::cerr << "USAGE: " << appName << " input.fbx output.json" << std::endl << std::endl;
            std::cerr << desc << std::endl;
            return ERROR_IN_COMMAND_LINE;
        }
        
        std::cout << "Required Positional, input: " << input
        << " output: " << output << std::endl;
        
        // application code here //
        
    }
    catch(std::exception& e)
    {
        std::cerr << "Unhandled Exception " << e.what() << ", application will now exit" << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;
    } 
    
    return SUCCESS;
}