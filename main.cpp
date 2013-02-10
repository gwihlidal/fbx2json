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

void usage(char* prog)
{
    std::cerr << prog << ": missing arguments" << std::endl;
	fprintf(stderr, "\nUSAGE: %s [FBX inputFile] [JSON outputFile] \n", prog);
}

int main(int argc, char** argv)
{
    if (argc < 3) {
		usage(argv[0]);
		return EXIT_FAILURE;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    
    return EXIT_SUCCESS;
}