#include "FBXImporter.h"

FBXImporter::FBXImporter()
{
  InitializeSdkObjects(mSdkManager, mScene);
}

void FBXImporter::InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
  pManager = FbxManager::Create();

  if (!pManager)
  {
    std::cerr << "Error: Unable to create FBX Manager!";
    std::cerr <<  std::endl;
    exit(1);
  }

  FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
  pManager->SetIOSettings(ios);

  pScene = FbxScene::Create(pManager, "My Scene");

  if (!pScene)
  {
    std::cerr << "Error: Unable to create FBX scene.";
    std::cerr << std::endl;
    exit(1);
  }
}

void FBXImporter::Import(const std::string input)
{
  const char* lFileName = input.c_str();

  if (mSdkManager)
  {
    int lFileFormat = -1;

    mImporter = FbxImporter::Create(mSdkManager,"");

    if (!mSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(lFileName, lFileFormat))
    {
    // Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
      lFileFormat = mSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
    }

    if(mImporter->Initialize(lFileName, lFileFormat) == true)
    {
      std::cout << "Importing file: " << input << std::endl;

      ImportScene();
    }
    else
    {
      std::cerr << "Unable to load file: " << input << std::endl;
      std::cerr << "Error reported: " << mImporter->GetLastErrorString() << std::endl;
    }
  }
}

void FBXImporter::ImportScene()
{
  if (mImporter->Import(mScene) == true)
  {
    NormaliseScene();
  }

  mImporter->Destroy();
  mImporter = NULL;
}

void FBXImporter::NormaliseScene()
{
  // Convert Axis System to what is used in this example, if needed
  FbxAxisSystem SceneAxisSystem = mScene->GetGlobalSettings().GetAxisSystem();
  FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
  
  if( SceneAxisSystem != OurAxisSystem )
  {
      OurAxisSystem.ConvertScene(mScene);
  }
  
  // Convert Unit System to what is used in this example, if needed
  FbxSystemUnit SceneSystemUnit = mScene->GetGlobalSettings().GetSystemUnit();
  
  if( SceneSystemUnit.GetScaleFactor() != 1.0 )
  {
      //The unit in this example is centimeter.
      FbxSystemUnit::cm.ConvertScene(mScene);
  }
  
  // Convert mesh, NURBS and patch into triangle mesh
  TriangulateRecursive(mScene->GetRootNode());
}

void FBXImporter::TriangulateRecursive(FbxNode* pNode)
{
  FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

  if (lNodeAttribute)
  {
    if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
      lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
        lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
          lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
    {
      FbxGeometryConverter lConverter(pNode->GetFbxManager());
      lConverter.TriangulateInPlace(pNode);
    }
  }

  const int lChildCount = pNode->GetChildCount();

  for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
  {
    TriangulateRecursive(pNode->GetChild(lChildIndex));
  }
}

FBXImporter::~FBXImporter()
{
  DestroySdkObjects(mSdkManager, true);
}

void FBXImporter::DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
  if (pManager)
  {
     pManager->Destroy();
  }
}