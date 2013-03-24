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
  initialize_sdk_objects(sdk_manager, scene);
}

void Importer::initialize_sdk_objects(FbxManager*& manager, FbxScene*& scene)
{
  manager = FbxManager::Create();

  if(!manager) {
    std::cerr << "Error: Unable to create FBX Manager!";
    std::cerr <<  std::endl;
    exit(1);
  }

  FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
  manager->SetIOSettings(ios);

  scene = FbxScene::Create(manager, "My Scene");

  if(!scene) {
    std::cerr << "Error: Unable to create FBX scene.";
    std::cerr << std::endl;
    exit(1);
  }
}

void Importer::import(const std::string input)
{
  const char* file_name = input.c_str();

  if(sdk_manager) {
    int file_format = -1;

    importer = FbxImporter::Create(sdk_manager,"");

    if(!sdk_manager->GetIOPluginRegistry()->DetectReaderFileFormat(file_name, file_format)) {
      // Unrecognizable file format. Try to fall back to Fbximporter::eFBX_BINARY
      file_format = sdk_manager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
    }

    if(importer->Initialize(file_name, file_format) == true) {
      std::cout << "Importing file: " << input << std::endl;

      import_scene();
    } else {
      std::cerr << "Unable to load file: " << input << std::endl;
      std::cerr << "Error reported: " << importer->GetLastErrorString() << std::endl;
    }
  }
}

void Importer::import_scene()
{
  if(importer->Import(scene) == true) {
    normalise_scene();
  }

  importer->Destroy();
  importer = NULL;
}

void Importer::normalise_scene()
{
  // Convert Axis System to what is used in this example, if needed
  FbxAxisSystem SceneAxisSystem = scene->GetGlobalSettings().GetAxisSystem();
  FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);

  if(SceneAxisSystem != OurAxisSystem) {
    OurAxisSystem.ConvertScene(scene);
  }

  // Convert Unit System to what is used in this example, if needed
  FbxSystemUnit SceneSystemUnit = scene->GetGlobalSettings().GetSystemUnit();

  if(SceneSystemUnit.GetScaleFactor() != 1.0) {
    //The unit in this example is centimeter.
    FbxSystemUnit::cm.ConvertScene(scene);
  }

  // Convert mesh, NURBS and patch into triangle mesh
  triangulate_recursive(scene->GetRootNode());
}

void Importer::triangulate_recursive(FbxNode* node)
{
  FbxNodeAttribute* node_attribute = node->GetNodeAttribute();

  if(node_attribute) {
    if(node_attribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
        node_attribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
        node_attribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
        node_attribute->GetAttributeType() == FbxNodeAttribute::ePatch) {
      FbxGeometryConverter converter(node->GetFbxManager());
      converter.TriangulateInPlace(node);
    }
  }

  const int child_count = node->GetChildCount();

  for(int i = 0; i < child_count; ++i) {
    triangulate_recursive(node->GetChild(i));
  }
}

Importer::~Importer()
{
  destroy_sdk_objects(sdk_manager, true);
}

void Importer::destroy_sdk_objects(FbxManager* manager, bool exit_status)
{
  if(manager) {
    manager->Destroy();
  }
}

} // namespace Fbx2Json
