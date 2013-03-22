/****************************************************************************************
 
 Copyright (C) 2012 Autodesk, Inc.
 All rights reserved.
 
 Use of this software is subject to the terms of the Autodesk license agreement
 provided at the time of installation or download, or which otherwise accompanies
 this software in either electronic or hard copy form.
 
 ****************************************************************************************/

#ifndef __fbx2json__SceneContext__
#define __fbx2json__SceneContext__

#include <fbxsdk.h>
#include <glew.h>

#include "GlFunctions.h"
#include "SceneCache.h"

class SceneContext
{
public:
    SceneContext(const char * pFileName);
    ~SceneContext();
    
    const FbxScene * GetScene() const { return mScene; }
    bool LoadFile();
    
    // The time period for one frame.
    const FbxTime GetFrameTime() const { return mFrameTime; }
    
    std::vector<VBOMesh *> * mMeshes;
    
private:
    const char * mFileName;
    
    FbxManager * mSdkManager;
    FbxScene * mScene;
    FbxImporter * mImporter;
    FbxAnimLayer * mCurrentAnimLayer;
    FbxNode * mSelectedNode;
    
    int mPoseIndex;
    
    mutable FbxTime mFrameTime, mStart, mStop, mCurrentTime;
    
//    struct Node
//    {
//        VBOMesh mesh;
//        MaterialCache material;
//    };
//    
//    std::vector<Node> mNodes;
    
};

#endif /* defined(__fbx2json__SceneContext__) */
