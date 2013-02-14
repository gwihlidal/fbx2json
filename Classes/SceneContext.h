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

class SceneContext
{
public:
//    enum Status
//    {
//        UNLOADED,               // Unload file or load failure;
//        MUST_BE_LOADED,         // Ready for loading file;
//        MUST_BE_REFRESHED,      // Something changed and redraw needed;
//        REFRESHED               // No redraw needed.
//    };
//    Status GetStatus() const { return mStatus; }
    
    SceneContext(const char * pFileName, bool pSupportVBO);
    ~SceneContext();
    
    const FbxScene * GetScene() const { return mScene; }
    bool LoadFile();
    
    // The time period for one frame.
    const FbxTime GetFrameTime() const { return mFrameTime; }
    
    // Methods for creating menus.
    // Get all the cameras in current scene, including producer cameras.
    const FbxArray<FbxNode *> & GetCameraArray() const { return mCameraArray; }
    // Get all the animation stack names in current scene.
    const FbxArray<FbxString *> & GetAnimStackNameArray() const { return mAnimStackNameArray; }
    // Get all the pose in current scene.
    const FbxArray<FbxPose *> & GetPoseArray() const { return mPoseArray; }
    
    // The input index is corresponding to the array returned from GetAnimStackNameArray.
    bool SetCurrentAnimStack(int pIndex);
    // Set the current camera with its name.
    bool SetCurrentCamera(const char * pCameraName);
    // The input index is corresponding to the array returned from GetPoseArray.
    bool SetCurrentPoseIndex(int pPoseIndex);
    // Set the currently selected node from external window system.
    void SetSelectedNode(FbxNode * pSelectedNode);
    // Set the shading mode, wire-frame or shaded.
    void SetShadingMode(ShadingMode pMode);
    
private:
    const char * mFileName;
//    mutable Status mStatus;
//    mutable FbxString mWindowMessage;
    
    FbxManager * mSdkManager;
    FbxScene * mScene;
    FbxImporter * mImporter;
    FbxAnimLayer * mCurrentAnimLayer;
    FbxNode * mSelectedNode;
    
    int mPoseIndex;
    FbxArray<FbxString*> mAnimStackNameArray;
    FbxArray<FbxNode*> mCameraArray;
    FbxArray<FbxPose*> mPoseArray;
    
    mutable FbxTime mFrameTime, mStart, mStop, mCurrentTime;
	mutable FbxTime mCache_Start, mCache_Stop;
    
    // Data for camera manipulation
//    mutable int mLastX, mLastY;
//    mutable FbxVector4 mCamPosition, mCamCenter;
//    mutable double mRoll;
//    mutable CameraStatus mCameraStatus;
    
//    bool mPause;
    ShadingMode mShadingMode;
    bool mSupportVBO;
    
    //camera zoom mode
//    CameraZoomMode mCameraZoomMode;
    
//    int mWindowWidth, mWindowHeight;
    // Utility class for draw text in OpenGL.
//    DrawText * mDrawText;
    
};

#endif /* defined(__fbx2json__SceneContext__) */
