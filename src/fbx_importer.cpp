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

#include "fbx_importer.h"

namespace Fbx2Json
{

Importer::Importer()
{
  initialize_sdk_objects(mSdkManager, mScene);
}

void Importer::initialize_sdk_objects(FbxManager*& pManager, FbxScene*& pScene)
{
  pManager = FbxManager::Create();

  if(!pManager) {
    std::cerr << "Error: Unable to create FBX Manager!";
    std::cerr <<  std::endl;
    exit(1);
  }

  FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
  pManager->SetIOSettings(ios);

  pScene = FbxScene::Create(pManager, "My Scene");

  if(!pScene) {
    std::cerr << "Error: Unable to create FBX scene.";
    std::cerr << std::endl;
    exit(1);
  }
}

void Importer::import(const std::string input)
{
  const char* lFileName = input.c_str();

  if(mSdkManager) {
    int lFileFormat = -1;

    mImporter = FbxImporter::Create(mSdkManager,"");

    if(!mSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(lFileName, lFileFormat)) {
      // Unrecognizable file format. Try to fall back to Fbximporter::eFBX_BINARY
      lFileFormat = mSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
    }

    if(mImporter->Initialize(lFileName, lFileFormat) == true) {
      std::cout << "Importing file: " << input << std::endl;

      import_scene();
    } else {
      std::cerr << "Unable to load file: " << input << std::endl;
      std::cerr << "Error reported: " << mImporter->GetLastErrorString() << std::endl;
    }
  }
}

void Importer::import_scene()
{
  if(mImporter->Import(mScene) == true) {
    normalise_scene();
  }

  mImporter->Destroy();
  mImporter = NULL;
}

void Importer::normalise_scene()
{
  // Convert Axis System to what is used in this example, if needed
  FbxAxisSystem SceneAxisSystem = mScene->GetGlobalSettings().GetAxisSystem();
  FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);

  if(SceneAxisSystem != OurAxisSystem) {
    OurAxisSystem.ConvertScene(mScene);
  }

  // Convert Unit System to what is used in this example, if needed
  FbxSystemUnit SceneSystemUnit = mScene->GetGlobalSettings().GetSystemUnit();

  if(SceneSystemUnit.GetScaleFactor() != 1.0) {
    //The unit in this example is centimeter.
    FbxSystemUnit::cm.ConvertScene(mScene);
  }

  // Convert mesh, NURBS and patch into triangle mesh
  triangulate_recursive(mScene->GetRootNode());
}

void Importer::triangulate_recursive(FbxNode* pNode)
{
  FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

  if(lNodeAttribute) {
    if(lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
        lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
        lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
        lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch) {
      FbxGeometryConverter lConverter(pNode->GetFbxManager());
      lConverter.TriangulateInPlace(pNode);
    }
  }

  const int lChildCount = pNode->GetChildCount();

  for(int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex) {
    triangulate_recursive(pNode->GetChild(lChildIndex));
  }
}

Importer::~Importer()
{
  destroy_sdk_objects(mSdkManager, true);
}

void Importer::destroy_sdk_objects(FbxManager* pManager, bool pExitStatus)
{
  if(pManager) {
    pManager->Destroy();
  }
}

} // namespace Fbx2Json
