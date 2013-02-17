//
//  main.cpp
//  fbx2json
//
//  Created by Cameron Yule on 09/02/2013.
//  Copyright (c) 2013 Cameron Yule. All rights reserved.
//

#include <iostream>
#include <string>
using namespace std;

#define FBXSDK_NEW_API true
#include <fbxsdk.h>

#include "SceneContext.h"

void ExitFunction();

SceneContext * gSceneContext;

void usage(std::string prog)
{
    std::cerr << prog << ": missing arguments" << std::endl;
    fprintf(stderr, "\nUSAGE: %s [FBX inputFile] [JSON outputFile] \n", prog.c_str());
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        argv[1] = "/Users/cameronyule/projects/personal/fbx2json/samples/drillbugtest.fbx";
        argv[2] = "out.js";
//        usage(argv[0]);
//        return EXIT_FAILURE;
    }
    
    // Set exit function to destroy objects created by the FBX SDK.
    atexit(ExitFunction);

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    
    gSceneContext = new SceneContext(inputFile.c_str());

    return EXIT_SUCCESS;
}

// Function to destroy objects created by the FBX SDK.
void ExitFunction()
{
    delete gSceneContext;
}