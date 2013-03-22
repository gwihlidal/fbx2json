/****************************************************************************************
 
 Copyright (C) 2012 Autodesk, Inc.
 All rights reserved.
 
 Use of this software is subject to the terms of the Autodesk license agreement
 provided at the time of installation or download, or which otherwise accompanies
 this software in either electronic or hard copy form.
 
 ****************************************************************************************///

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include "SceneContext.h"
#include "SceneCache.h"
#include "Common.h"

namespace
{ 
    // Triangulate all NURBS, patch and mesh under this node recursively.
    void TriangulateRecursive(FbxNode* pNode)
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
    
    // Bake node attributes and materials under this node recursively.
    // Currently only mesh and material.
    void LoadCacheRecursive(std::vector<VBOMesh *> * pMeshes, FbxNode * pNode, FbxAnimLayer * pAnimLayer)
    {        
        // Bake material and hook as user data.
        const int lMaterialCount = pNode->GetMaterialCount();
        
        for (int lMaterialIndex = 0; lMaterialIndex < lMaterialCount; ++lMaterialIndex)
        {
            FbxSurfaceMaterial * lMaterial = pNode->GetMaterial(lMaterialIndex);
            
            if (lMaterial && !lMaterial->GetUserDataPtr())
            {                
                FbxAutoPtr<MaterialCache> lMaterialCache(new MaterialCache);
                
                if (lMaterialCache->Initialize(lMaterial))
                {
                    lMaterial->SetUserDataPtr(lMaterialCache.Release());
                }
            }
        }
        
        // Bake mesh as VBO(vertex buffer object)
        FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
        if (lNodeAttribute)
        {
            if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                FbxMesh * lMesh = pNode->GetMesh();
                
                if (lMesh && !lMesh->GetUserDataPtr())
                {
                    VBOMesh * lMeshCache = new VBOMesh;
                    
                    if (lMeshCache->Initialize(lMesh))
                    {
                        pMeshes->push_back(lMeshCache);
                    }
                }
            }
        }
        
        const int lChildCount = pNode->GetChildCount();
        
        for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
        {            
            LoadCacheRecursive(pMeshes, pNode->GetChild(lChildIndex), pAnimLayer);
        }
    }
    
    // Unload the cache and release the memory under this node recursively.
    void UnloadCacheRecursive(FbxNode * pNode)
    {
        // Unload the material cache
        const int lMaterialCount = pNode->GetMaterialCount();
        
        for (int lMaterialIndex = 0; lMaterialIndex < lMaterialCount; ++lMaterialIndex)
        {
            FbxSurfaceMaterial * lMaterial = pNode->GetMaterial(lMaterialIndex);
            
            if (lMaterial && lMaterial->GetUserDataPtr())
            {
                MaterialCache * lMaterialCache = static_cast<MaterialCache *>(lMaterial->GetUserDataPtr());
                lMaterial->SetUserDataPtr(NULL);
                delete lMaterialCache;
            }
        }
        
        FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
        if (lNodeAttribute)
        {
            // Unload the mesh cache
            if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                FbxMesh * lMesh = pNode->GetMesh();
                
                if (lMesh && lMesh->GetUserDataPtr())
                {
                    VBOMesh * lMeshCache = static_cast<VBOMesh *>(lMesh->GetUserDataPtr());
                    lMesh->SetUserDataPtr(NULL);
                    delete lMeshCache;
                }
            }
        }
        
        const int lChildCount = pNode->GetChildCount();
        
        for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
        {
            UnloadCacheRecursive(pNode->GetChild(lChildIndex));
        }
    }
    
    // Bake node attributes and materials for this scene and load the textures.
    void LoadCacheRecursive(std::vector<VBOMesh *> * pMeshes, FbxScene * pScene, FbxAnimLayer * pAnimLayer, const char * pFbxFileName)
    {        
        // TODO: Capture scene texture filenames, parse to websafe format

        LoadCacheRecursive(pMeshes, pScene->GetRootNode(), pAnimLayer);
    }
    
    // Unload the cache and release the memory from this scene
    void UnloadCacheRecursive(FbxScene * pScene)
    {        
        UnloadCacheRecursive(pScene->GetRootNode());
    }
}

SceneContext::SceneContext(const char * pFileName)
: mFileName(pFileName), mSdkManager(NULL), mScene(NULL), mImporter(NULL),
mCurrentAnimLayer(NULL), mSelectedNode(NULL), mMeshes(NULL), mPoseIndex(-1)
{   
	// initialize cache start and stop time
//	mCache_Start = FBXSDK_TIME_INFINITE;
//	mCache_Stop  = FBXSDK_TIME_MINUS_INFINITE;
    
    // Create the FBX SDK manager which is the object allocator for almost
    // all the classes in the SDK and create the scene.
    InitializeSdkObjects(mSdkManager, mScene);
    
    if (mSdkManager)
    {
        int lFileFormat = -1;
        
        mImporter = FbxImporter::Create(mSdkManager,"");
        
        if (!mSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(mFileName, lFileFormat) )
        {
            // Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
            lFileFormat = mSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription( "FBX binary (*.fbx)" );;
        }
        
        if(mImporter->Initialize(mFileName, lFileFormat) == true)
        {
            std::cout << "Importing file " << mFileName << ". Please wait." << std::endl;
            
            mMeshes = new std::vector<VBOMesh *>;

            LoadFile();
        }
        else
        {
            std::cerr << "Unable to open file " << mFileName << std::endl;
            std::cerr << "Error reported: " << mImporter->GetLastErrorString() << std::endl;
        }
    }
    else
    {
        std::cerr << "Unable to create the FBX SDK manager.";
    }
}

SceneContext::~SceneContext()
{   
    // Unload the cache and free the memory
    if (mScene)
    {
        UnloadCacheRecursive(mScene);
    }
    
    // Delete the FBX SDK manager. All the objects that have been allocated
    // using the FBX SDK manager and that haven't been explicitly destroyed
    // are automatically destroyed at the same time.
	DestroySdkObjects(mSdkManager, true);
}

bool SceneContext::LoadFile()
{
    bool lResult = false;

    if (mImporter->Import(mScene) == true)
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
        
        // Bake the scene for one frame
        LoadCacheRecursive(mMeshes, mScene, mCurrentAnimLayer, mFileName);
        
        // Initialize the frame period.
        mFrameTime.SetTime(0, 0, 0, 1, 0, mScene->GetGlobalSettings().GetTimeMode());
        
        lResult = true;
    }
    else
    {           
        std::cerr << "Unable to import file " << mFileName << std::endl;
        std::cerr << "Error reported: " << mImporter->GetLastErrorString() << std::endl;
    }
    
    // Destroy the importer to release the file.
    mImporter->Destroy();
    mImporter = NULL;
    
    return lResult;
}
