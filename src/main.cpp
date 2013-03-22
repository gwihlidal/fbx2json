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

#include "FBXImporter.h"
#include "FBXParser.h"
#include "FBXExporter.h"

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

void parseArguments(int argc, char** argv)
{
  int index;
  int c;

  while ((c = getopt(argc, argv, "v")) != -1)
  {
    switch (c)
    {
      case 'v':
        version();
        break;
    }
  }

  for (index = optind; index < argc; index++)
  {
    printf ("Non-option argument %s\n", argv[index]);
  }
}

int main(int argc, char** argv)
{
  parseArguments(argc, argv);

  // if (argv[1] == " -v")
  // {
  //   version(argv[1]);
  //   
  //   return EXIT_SUCCESS;
  // }

  // if (argc < 3)
  // {
  //   usage(argv[0]);
  // 
  //   return EXIT_FAILURE;
  // }

  // std::string input = argv[1];
  // std::string output = argv[2];
  // 
  // // Initialise the FBX SDK and import our FBX file
  // FBXImporter importer = FBXImporter();
  // importer.Import(input);
  // 
  // // Bake component parts of FBX into raw data for export
  // FBXParser parser = FBXParser();
  // parser.Parse(importer.GetScene());
  // 
  // // Output JSON-formatted raw data
  // FBXExporter exporter = FBXExporter();
  // exporter.Export();

  return EXIT_SUCCESS;
}