#ifndef FBXImporter_H
#define FBXImporter_H

#include <iostream>
#include <string>
#include <fbxsdk.h>

class FBXImporter
{
  public:
    FBXImporter();
    void Import(const std::string input);
    FbxScene * GetScene() { return mScene; };
    ~FBXImporter();

  private:
    void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
    void ImportScene();
    void NormaliseScene();
    void TriangulateRecursive(FbxNode* pNode);
    void DestroySdkObjects(FbxManager* pManager, bool pExitStatus);

    FbxManager * mSdkManager;
    FbxScene * mScene;
    FbxImporter * mImporter;
};

#endif