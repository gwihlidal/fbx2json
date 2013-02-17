/****************************************************************************************
 
 Copyright (C) 2012 Autodesk, Inc.
 All rights reserved.
 
 Use of this software is subject to the terms of the Autodesk license agreement
 provided at the time of installation or download, or which otherwise accompanies
 this software in either electronic or hard copy form.
 
 ****************************************************************************************///

#include <iostream>
#include <string>
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
    // Currently only mesh, light and material.
    void LoadCacheRecursive(FbxNode * pNode, FbxAnimLayer * pAnimLayer)
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
        
        FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
        if (lNodeAttribute)
        {
            // Bake mesh as VBO(vertex buffer object) into GPU.
            if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                FbxMesh * lMesh = pNode->GetMesh();
                if (lMesh && !lMesh->GetUserDataPtr())
                {
                    FbxAutoPtr<VBOMesh> lMeshCache(new VBOMesh);
                    if (lMeshCache->Initialize(lMesh))
                    {
                        lMesh->SetUserDataPtr(lMeshCache.Release());
                    }
                }
            }
            // Bake light properties.
            else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eLight)
            {
                FbxLight * lLight = pNode->GetLight();
                if (lLight && !lLight->GetUserDataPtr())
                {
                    FbxAutoPtr<LightCache> lLightCache(new LightCache);
                    if (lLightCache->Initialize(lLight, pAnimLayer))
                    {
                        lLight->SetUserDataPtr(lLightCache.Release());
                    }
                }
            }
        }
        
        const int lChildCount = pNode->GetChildCount();
        for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
        {
            LoadCacheRecursive(pNode->GetChild(lChildIndex), pAnimLayer);
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
            // Unload the light cache
            else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eLight)
            {
                FbxLight * lLight = pNode->GetLight();
                if (lLight && lLight->GetUserDataPtr())
                {
                    LightCache * lLightCache = static_cast<LightCache *>(lLight->GetUserDataPtr());
                    lLight->SetUserDataPtr(NULL);
                    delete lLightCache;
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
    void LoadCacheRecursive(FbxScene * pScene, FbxAnimLayer * pAnimLayer, const char * pFbxFileName)
    {
//        // Load the textures into GPU, only for file texture now
//        const int lTextureCount = pScene->GetTextureCount();
//        for (int lTextureIndex = 0; lTextureIndex < lTextureCount; ++lTextureIndex)
//        {
//            FbxTexture * lTexture = pScene->GetTexture(lTextureIndex);
//            FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(lTexture);
//            if (lFileTexture && !lFileTexture->GetUserDataPtr())
//            {
//                // Try to load the texture from absolute path
//                const FbxString lFileName = lFileTexture->GetFileName();
//                
//                // Only TGA textures are supported now.
//                if (lFileName.Right(3).Upper() != "TGA")
//                {
//                    FBXSDK_printf("Only TGA textures are supported now: %s\n", lFileName.Buffer());
//                    continue;
//                }
//                
//                GLuint lTextureObject = 0;
//                bool lStatus = true;
////                bool lStatus = LoadTextureFromFile(lFileName, lTextureObject);
//                
//                const FbxString lAbsFbxFileName = FbxPathUtils::Resolve(pFbxFileName);
//                const FbxString lAbsFolderName = FbxPathUtils::GetFolderName(lAbsFbxFileName);
//                if (!lStatus)
//                {
//                    // Load texture from relative file name (relative to FBX file)
//                    const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lFileTexture->GetRelativeFileName());
////                    lStatus = LoadTextureFromFile(lResolvedFileName, lTextureObject);
//                }
//                
//                if (!lStatus)
//                {
//                    // Load texture from file name only (relative to FBX file)
//                    const FbxString lTextureFileName = FbxPathUtils::GetFileName(lFileName);
//                    const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lTextureFileName);
////                    lStatus = LoadTextureFromFile(lResolvedFileName, lTextureObject);
//                }
//                
//                if (!lStatus)
//                {
//                    FBXSDK_printf("Failed to load texture file: %s\n", lFileName.Buffer());
//                    continue;
//                }
//                
//                if (lStatus)
//                {
//                    GLuint * lTextureName = new GLuint(lTextureObject);
//                    lFileTexture->SetUserDataPtr(lTextureName);
//                }
//            }
//        }
        
        LoadCacheRecursive(pScene->GetRootNode(), pAnimLayer);
    }
    
    // Unload the cache and release the memory fro this scene and release the textures in GPU
    void UnloadCacheRecursive(FbxScene * pScene)
    {
        const int lTextureCount = pScene->GetTextureCount();
        for (int lTextureIndex = 0; lTextureIndex < lTextureCount; ++lTextureIndex)
        {
            FbxTexture * lTexture = pScene->GetTexture(lTextureIndex);
            FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(lTexture);
            if (lFileTexture && lFileTexture->GetUserDataPtr())
            {
                GLuint * lTextureName = static_cast<GLuint *>(lFileTexture->GetUserDataPtr());
                lFileTexture->SetUserDataPtr(NULL);
//                glDeleteTextures(1, lTextureName);
                delete lTextureName;
            }
        }
        
        UnloadCacheRecursive(pScene->GetRootNode());
    }
}

SceneContext::SceneContext(const char * pFileName)
: mFileName(pFileName), mSdkManager(NULL), mScene(NULL), mImporter(NULL),
mCurrentAnimLayer(NULL), mSelectedNode(NULL), mPoseIndex(-1)
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
        LoadCacheRecursive(mScene, mCurrentAnimLayer, mFileName);
        
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
