/****************************************************************************************
 
 Copyright (C) 2012 Autodesk, Inc.
 All rights reserved.
 
 Use of this software is subject to the terms of the Autodesk license agreement
 provided at the time of installation or download, or which otherwise accompanies
 this software in either electronic or hard copy form.
 
 ****************************************************************************************/

#ifndef __fbx2json__SceneCache__
#define __fbx2json__SceneCache__

#include <fbxsdk.h>
#include <glew.h>

#include "GlFunctions.h"

#include <vector>
using namespace std;

// Save mesh vertices, normals, UVs and indices in GPU with OpenGL Vertex Buffer Objects
class VBOMesh
{
public:
    VBOMesh();
    ~VBOMesh();
    
    bool Initialize(const FbxMesh * pMesh);
    
    std::vector<float> * mVertices;
    std::vector<GLuint> * mIndices;
    std::vector<float> * mNormals;
    std::vector<float> * mUVs;
    
    bool mHasNormal;
    bool mHasUV;
    bool mAllByControlPoint; // Save data in VBO by control point or by polygon vertex.
    
private:
    enum
    {
        VERTEX_VBO,
        NORMAL_VBO,
        UV_VBO,
        INDEX_VBO,
        VBO_COUNT,
    };
    
    // For every material, record the offsets in every VBO and triangle counts
    struct SubMesh
    {
        SubMesh() : IndexOffset(0), TriangleCount(0) {}
        
        int IndexOffset;
        int TriangleCount;
    };
    
    GLuint mVBONames[VBO_COUNT];
    FbxArray<SubMesh*> mSubMeshes;
};

// Cache for FBX material
class MaterialCache
{
public:
    MaterialCache();
    ~MaterialCache();
    
    bool Initialize(const FbxSurfaceMaterial * pMaterial);
    
private:
    struct ColorChannel
    {
        ColorChannel() : mTextureName(0)
        {
            mColor[0] = 0.0f;
            mColor[1] = 0.0f;
            mColor[2] = 0.0f;
            mColor[3] = 1.0f;
        }
        
        GLuint mTextureName;
        GLfloat mColor[4];
    };
    ColorChannel mEmissive;
    ColorChannel mAmbient;
    ColorChannel mDiffuse;
    ColorChannel mSpecular;
    GLfloat mShinness;
};

// Property cache, value and animation curve.
struct PropertyChannel
{
    PropertyChannel() : mAnimCurve(NULL), mValue(0.0f) {}
    // Query the channel value at specific time.
    GLfloat Get(const FbxTime & pTime) const
    {
        if (mAnimCurve)
        {
            return mAnimCurve->Evaluate(pTime);
        }
        else
        {
            return mValue;
        }
    }
    
    FbxAnimCurve * mAnimCurve;
    GLfloat mValue;
};

#endif /* defined(__fbx2json__SceneCache__) */
