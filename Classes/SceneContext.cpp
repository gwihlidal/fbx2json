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
//#include "SetCamera.h"
//#include "DrawScene.h"
//#include "DrawText.h"
//#include "targa.h"
#include "Common.h"

namespace
{
    // Find all the cameras under this node recursively.
    void FillCameraArrayRecursive(FbxNode* pNode, FbxArray<FbxNode*>& pCameraArray)
    {
        if (pNode)
        {
            if (pNode->GetNodeAttribute())
            {
                if (pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eCamera)
                {
                    pCameraArray.Add(pNode);
                }
            }
            
            const int lCount = pNode->GetChildCount();
            for (int i = 0; i < lCount; i++)
            {
                FillCameraArrayRecursive(pNode->GetChild(i), pCameraArray);
            }
        }
    }
    
    // Find all the cameras in this scene.
    void FillCameraArray(FbxScene* pScene, FbxArray<FbxNode*>& pCameraArray)
    {
        pCameraArray.Clear();
        
        FillCameraArrayRecursive(pScene->GetRootNode(), pCameraArray);
    }
    
    // Find all poses in this scene.
    void FillPoseArray(FbxScene* pScene, FbxArray<FbxPose*>& pPoseArray)
    {
        const int lPoseCount = pScene->GetPoseCount();
        
        for (int i=0; i < lPoseCount; ++i)
        {
            pPoseArray.Add(pScene->GetPose(i));
        }
    }
    
    void PreparePointCacheData(FbxScene* pScene, FbxTime &pCache_Start, FbxTime &pCache_Stop)
    {
        // This function show how to cycle through scene elements in a linear way.
		const int lNodeCount = pScene->GetSrcObjectCount<FbxNode>();
        
        for (int lIndex=0; lIndex<lNodeCount; lIndex++)
        {
            FbxNode* lNode = pScene->GetSrcObject<FbxNode>(lIndex);
            
            if (lNode->GetGeometry())
            {
                int i, lVertexCacheDeformerCount = lNode->GetGeometry()->GetDeformerCount(FbxDeformer::eVertexCache);
                
                // There should be a maximum of 1 Vertex Cache Deformer for the moment
                lVertexCacheDeformerCount = lVertexCacheDeformerCount > 0 ? 1 : 0;
                
                for (i=0; i<lVertexCacheDeformerCount; ++i )
                {
                    // Get the Point Cache object
                    FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(lNode->GetGeometry()->GetDeformer(i, FbxDeformer::eVertexCache));
                    if( !lDeformer ) continue;
                    FbxCache* lCache = lDeformer->GetCache();
                    if( !lCache ) continue;
                    
                    // Process the point cache data only if the constraint is active
                    if (lDeformer->IsActive())
                    {
                        if (lCache->GetCacheFileFormat() == FbxCache::eMaxPointCacheV2)
                        {
                            // This code show how to convert from PC2 to MC point cache format
                            // turn it on if you need it.
#if 0
                            if (!lCache->ConvertFromPC2ToMC(FbxCache::eMCOneFile,
                                                            FbxTime::GetFrameRate(pScene->GetGlobalTimeSettings().GetTimeMode())))
                            {
                                // Conversion failed, retrieve the error here
                                FbxString lTheErrorIs = lCache->GetError().GetLastErrorString();
                            }
#endif
                        }
                        else if (lCache->GetCacheFileFormat() == FbxCache::eMayaCache)
                        {
                            // This code show how to convert from MC to PC2 point cache format
                            // turn it on if you need it.
                            //#if 0
                            if (!lCache->ConvertFromMCToPC2(FbxTime::GetFrameRate(pScene->GetGlobalSettings().GetTimeMode()), 0))
                            {
                                // Conversion failed, retrieve the error here
                                FbxString lTheErrorIs = lCache->GetError().GetLastErrorString();
                            }
                            //#endif
                        }
                        
                        
                        // Now open the cache file to read from it
                        if (!lCache->OpenFileForRead())
                        {
                            // Cannot open file
                            FbxString lTheErrorIs = lCache->GetError().GetLastErrorString();
                            
                            // Set the deformer inactive so we don't play it back
                            lDeformer->SetActive(false);
                        }
						else
						{
							// get the start and stop time of the cache
							int lChannelCount = lCache->GetChannelCount();
							
							for (int iChannelNo=0; iChannelNo < lChannelCount; iChannelNo++)
							{
								FbxTime lChannel_Start;
								FbxTime lChannel_Stop;
                                
								if(lCache->GetAnimationRange(iChannelNo, lChannel_Start, lChannel_Stop))
								{
									// get the smallest start time
									if(lChannel_Start < pCache_Start) pCache_Start = lChannel_Start;
                                    
									// get the biggest stop time
									if(lChannel_Stop  > pCache_Stop)  pCache_Stop  = lChannel_Stop;
								}
							}
						}
                    }
                }
            }
        }
    }
    
    // Load a texture file (TGA only now) into GPU and return the texture object name
//    bool LoadTextureFromFile(const FbxString & pFilePath, GLuint & pTextureObject)
//    {
//        if (pFilePath.Right(3).Upper() == "TGA")
//        {
//            tga_image lTGAImage;
//            
//            if (tga_read(&lTGAImage, pFilePath.Buffer()) == TGA_NOERR)
//            {
//                // Make sure the image is left to right
//                if (tga_is_right_to_left(&lTGAImage))
//                    tga_flip_horiz(&lTGAImage);
//                
//                // Make sure the image is bottom to top
//                if (tga_is_top_to_bottom(&lTGAImage))
//                    tga_flip_vert(&lTGAImage);
//                
//                // Make the image BGR 24
//                tga_convert_depth(&lTGAImage, 24);
//                
//                // Transfer the texture date into GPU
//                glGenTextures(1, &pTextureObject);
//                glBindTexture(GL_TEXTURE_2D, pTextureObject);
//                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//                glTexImage2D(GL_TEXTURE_2D, 0, 3, lTGAImage.width, lTGAImage.height, 0, GL_BGR,
//                             GL_UNSIGNED_BYTE, lTGAImage.image_data);
//                glBindTexture(GL_TEXTURE_2D, 0);
//                
//                tga_free_buffers(&lTGAImage);
//                
//                return true;
//            }
//        }
//        
//        return false;
//    }
    
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
    void LoadCacheRecursive(FbxNode * pNode, FbxAnimLayer * pAnimLayer, bool pSupportVBO)
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
                if (pSupportVBO && lMesh && !lMesh->GetUserDataPtr())
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
            LoadCacheRecursive(pNode->GetChild(lChildIndex), pAnimLayer, pSupportVBO);
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
    void LoadCacheRecursive(FbxScene * pScene, FbxAnimLayer * pAnimLayer, const char * pFbxFileName, bool pSupportVBO)
    {
        // Load the textures into GPU, only for file texture now
        const int lTextureCount = pScene->GetTextureCount();
        for (int lTextureIndex = 0; lTextureIndex < lTextureCount; ++lTextureIndex)
        {
            FbxTexture * lTexture = pScene->GetTexture(lTextureIndex);
            FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(lTexture);
            if (lFileTexture && !lFileTexture->GetUserDataPtr())
            {
                // Try to load the texture from absolute path
                const FbxString lFileName = lFileTexture->GetFileName();
                
                // Only TGA textures are supported now.
                if (lFileName.Right(3).Upper() != "TGA")
                {
                    FBXSDK_printf("Only TGA textures are supported now: %s\n", lFileName.Buffer());
                    continue;
                }
                
                GLuint lTextureObject = 0;
                bool lStatus = true;
//                bool lStatus = LoadTextureFromFile(lFileName, lTextureObject);
                
                const FbxString lAbsFbxFileName = FbxPathUtils::Resolve(pFbxFileName);
                const FbxString lAbsFolderName = FbxPathUtils::GetFolderName(lAbsFbxFileName);
                if (!lStatus)
                {
                    // Load texture from relative file name (relative to FBX file)
                    const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lFileTexture->GetRelativeFileName());
//                    lStatus = LoadTextureFromFile(lResolvedFileName, lTextureObject);
                }
                
                if (!lStatus)
                {
                    // Load texture from file name only (relative to FBX file)
                    const FbxString lTextureFileName = FbxPathUtils::GetFileName(lFileName);
                    const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lTextureFileName);
//                    lStatus = LoadTextureFromFile(lResolvedFileName, lTextureObject);
                }
                
                if (!lStatus)
                {
                    FBXSDK_printf("Failed to load texture file: %s\n", lFileName.Buffer());
                    continue;
                }
                
                if (lStatus)
                {
                    GLuint * lTextureName = new GLuint(lTextureObject);
                    lFileTexture->SetUserDataPtr(lTextureName);
                }
            }
        }
        
        LoadCacheRecursive(pScene->GetRootNode(), pAnimLayer, pSupportVBO);
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

SceneContext::SceneContext(const char * pFileName, bool pSupportVBO)
: mFileName(pFileName), mStatus(UNLOADED), mSupportVBO(pSupportVBO),
mSdkManager(NULL), mScene(NULL), mImporter(NULL), mCurrentAnimLayer(NULL), mSelectedNode(NULL),
mPoseIndex(-1), mShadingMode(SHADING_MODE_SHADED)
{   
	// initialize cache start and stop time
	mCache_Start = FBXSDK_TIME_INFINITE;
	mCache_Stop  = FBXSDK_TIME_MINUS_INFINITE;
    
    // Create the FBX SDK manager which is the object allocator for almost
    // all the classes in the SDK and create the scene.
    InitializeSdkObjects(mSdkManager, mScene);
    
    if (mSdkManager)
    {
        // Create the importer.
        int lFileFormat = -1;
        mImporter = FbxImporter::Create(mSdkManager,"");
        if (!mSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(mFileName, lFileFormat) )
        {
            // Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
            lFileFormat = mSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription( "FBX binary (*.fbx)" );;
        }
        
        // Initialize the importer by providing a filename.
        if(mImporter->Initialize(mFileName, lFileFormat) == true)
        {
            // The file is going to be imported at
            // the end of the first display callback.
            std::cout << "Importing file " << mFileName << std::endl;
            std::cout << "Please wait." << std::endl;
            
            // Set scene status flag to ready to load.
            mStatus = MUST_BE_LOADED;

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
    FbxArrayDelete(mAnimStackNameArray);
    
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
    // Make sure that the scene is ready to load.
    if (mStatus == MUST_BE_LOADED)
    {
        if (mImporter->Import(mScene) == true)
        {
            // Set the scene status flag to refresh
            // the scene in the first timer callback.
            mStatus = MUST_BE_REFRESHED;
            
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
                FbxSystemUnit::cm.ConvertScene( mScene);
            }
            
            // Get the list of all the animation stack.
            mScene->FillAnimStackNameArray(mAnimStackNameArray);
            
            // Get the list of all the cameras in the scene.
            FillCameraArray(mScene, mCameraArray);
            
            // Convert mesh, NURBS and patch into triangle mesh
            TriangulateRecursive(mScene->GetRootNode());
            
            // Bake the scene for one frame
            LoadCacheRecursive(mScene, mCurrentAnimLayer, mFileName, mSupportVBO);
            
            // Convert any .PC2 point cache data into the .MC format for
            // vertex cache deformer playback.
            PreparePointCacheData(mScene, mCache_Start, mCache_Stop);
            
            // Get the list of pose in the scene
            FillPoseArray(mScene, mPoseArray);
            
            // Initialize the window message.
//            mWindowMessage = "File ";
//            mWindowMessage += mFileName;
//            mWindowMessage += "\nClick on the right mouse button to enter menu.";
//            mWindowMessage += "\nEsc to exit.";
            
            // Initialize the frame period.
            mFrameTime.SetTime(0, 0, 0, 1, 0, mScene->GetGlobalSettings().GetTimeMode());
            
            // Print the keyboard shortcuts.
//            FBXSDK_printf("Play/Pause Animation: Space Bar.\n");
//            FBXSDK_printf("Camera Rotate: Left Mouse Button.\n");
//            FBXSDK_printf("Camera Pan: Left Mouse Button + Middle Mouse Button.\n");
//            FBXSDK_printf("Camera Zoom: Middle Mouse Button.\n");
            
            lResult = true;
        }
        else
        {
            // Import failed, set the scene status flag accordingly.
            mStatus = UNLOADED;
            
            std::cerr << "Unable to import file " << mFileName << std::endl;
            std::cerr << "Error reported: " << mImporter->GetLastErrorString() << std::endl;
        }
        
        // Destroy the importer to release the file.
        mImporter->Destroy();
        mImporter = NULL;
    }
    
    return lResult;
}
