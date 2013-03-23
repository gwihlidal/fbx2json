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

#ifndef FBX2JSON_IMPORTER_H
#define FBX2JSON_IMPORTER_H

#include <iostream>
#include <string>
#include <fbxsdk.h>

namespace Fbx2Json
{

class Importer
{
  public:
    Importer();
    void import(const std::string input);
    FbxScene * get_scene() {
      return mScene;
    };
    ~Importer();

  private:
    void initialize_sdk_objects(FbxManager*& pManager, FbxScene*& pScene);
    void import_scene();
    void normalise_scene();
    void triangulate_recursive(FbxNode* pNode);
    void destroy_sdk_objects(FbxManager* pManager, bool pExitStatus);

    FbxManager * mSdkManager;
    FbxScene * mScene;
    FbxImporter * mImporter;
};

} // namespace Fbx2Json

#endif
