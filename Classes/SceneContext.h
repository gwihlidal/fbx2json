//
//  SceneContext.h
//  fbx2json
//

#ifndef __fbx2json__SceneContext__
#define __fbx2json__SceneContext__

#include <fbxsdk.h>
#include <glew.h>

class SceneContext
{
public:
    SceneContext(const char * pFileName, bool pSupportVBO);
    ~SceneContext();
    
    const FbxScene * GetScene() const { return mScene; }
    bool LoadFile();
    
    // The time period for one frame.
    const FbxTime GetFrameTime() const { return mFrameTime; }
    
private:
    const char * mFileName;
    
    FbxManager * mSdkManager;
    FbxScene * mScene;
    FbxImporter * mImporter;
    FbxAnimLayer * mCurrentAnimLayer;
    FbxNode * mSelectedNode;
    
    int mPoseIndex;
    
    mutable FbxTime mFrameTime, mStart, mStop, mCurrentTime;
	mutable FbxTime mCache_Start, mCache_Stop;
    
};

#endif /* defined(__fbx2json__SceneContext__) */
