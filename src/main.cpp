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

void usage(std::string prog)
{
  std::cerr << prog << ": missing arguments" << std::endl << std::endl;
  std::cerr << "USAGE: " << prog;
  std::cerr << "[FBX inputFile] [JSON outputFile]" << std::endl;
}

int main(int argc, char** argv)
{
  if (argc < 3)
  {
    usage(argv[0]);

    return EXIT_FAILURE;
  }

  std::string input = argv[1];
  std::string output = argv[2];

  // Initialise the FBX SDK and import our FBX file
  FBXImporter importer = FBXImporter();
  importer.Import(input);

  // Bake component parts of FBX into raw data for export
  FBXSceneParser parser = FBXSceneParser();
  parser.Parse(importer.mScene);

  // Output JSON-formatted raw data
  FBXExporter exporter = FBXExporter();
  exporter.Export(parser.data);

  return EXIT_SUCCESS;
}