/*
 * Copyright 2013 Cameron Yule.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <iostream>
#include <string>

#include "fbx_importer.h"
#include "fbx_parser.h"
#include "fbx_exporter.h"

#define FBX2JSON_MAJOR "0"
#define FBX2JSON_MINOR "1"
#define FBX2JSON_PATCH "0"

void usage(std::string prog)
{
  std::cerr << prog << ": missing arguments" << std::endl << std::endl;
  std::cerr << "USAGE: " << prog;
  std::cerr << "[FBX inputFile] [JSON outputFile]" << std::endl;
}

void version()
{
  std::cout << FBX2JSON_MAJOR << ".";
  std::cout << FBX2JSON_MINOR << ".";
  std::cout << FBX2JSON_PATCH;
  std::cout << std::endl;
}

bool parse_arguments(int argc, char** argv)
{
  int c;

  while((c = getopt(argc, argv, "v")) != -1) {
    switch(c) {
      case 'v':
        version();
        return false;
        break;
    }
  }

  if (argc < 3) {
    usage(argv[0]);
    return false;
  }
  
  return true;
}

int main(int argc, char** argv)
{
  if (parse_arguments(argc, argv)) {
    std::string input = argv[1];
    std::string output = argv[2];

    // Initialise the FBX SDK and import our FBX file
    Fbx2Json::Importer importer = Fbx2Json::Importer();
    importer.import(input);

    // Bake component parts of FBX into raw data for export
    Fbx2Json::FbxParser parser = Fbx2Json::FbxParser();
    // parser.parse(importer.GetScene());

    // Output JSON-formatted raw data
    Fbx2Json::FbxExporter exporter = Fbx2Json::FbxExporter();
    // exporter.export();

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}
