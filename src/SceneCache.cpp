/****************************************************************************************
 
 Copyright (C) 2012 Autodesk, Inc.
 All rights reserved.
 
 Use of this software is subject to the terms of the Autodesk license agreement
 provided at the time of installation or download, or which otherwise accompanies
 this software in either electronic or hard copy form.
 
 ****************************************************************************************/

#include "SceneCache.h"

namespace
{
    const float ANGLE_TO_RADIAN = 3.1415926f / 180.f;
    
    const int TRIANGLE_VERTEX_COUNT = 3;
    
    // Four floats for every position.
    const int VERTEX_STRIDE = 3;
    // Three floats for every normal.
    const int NORMAL_STRIDE = 3;
    // Two floats for every UV.
    const int UV_STRIDE = 2;
    
    // Get specific property value and connected texture if any.
    // Value = Property value * Factor property value (if no factor property, multiply by 1).
    FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial * pMaterial,
                                   const char * pPropertyName,
                                   const char * pFactorPropertyName,
                                   GLuint & pTextureName)
    {
        FbxDouble3 lResult(0, 0, 0);
        const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
        const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
        if (lProperty.IsValid() && lFactorProperty.IsValid())
        {
            lResult = lProperty.Get<FbxDouble3>();
            double lFactor = lFactorProperty.Get<FbxDouble>();
            if (lFactor != 1)
            {
                lResult[0] *= lFactor;
                lResult[1] *= lFactor;
                lResult[2] *= lFactor;
            }
        }
        
        if (lProperty.IsValid())
        {
            const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();
            if (lTextureCount)
            {
                const FbxFileTexture* lTexture = lProperty.GetSrcObject<FbxFileTexture>();
                if (lTexture && lTexture->GetUserDataPtr())
                {
                    pTextureName = *(static_cast<GLuint *>(lTexture->GetUserDataPtr()));
                }
            }
        }
        
        return lResult;
    }
}

VBOMesh::VBOMesh() : mHasNormal(false), mHasUV(false), mAllByControlPoint(true)
{
    // Reset every VBO to zero, which means no buffer.
    for (int lVBOIndex = 0; lVBOIndex < VBO_COUNT; ++lVBOIndex)
    {
        mVBONames[lVBOIndex] = 0;
    }
}

VBOMesh::~VBOMesh()
{    
	for(int i=0; i < mSubMeshes.GetCount(); i++)
	{
		delete mSubMeshes[i];
	}
    
    mVertices->clear();
    mIndices->clear();
    mNormals->clear();
    mUVs->clear();
	
	mSubMeshes.Clear();    
}

bool VBOMesh::Initialize(const FbxMesh *pMesh)
{
    if (!pMesh->GetNode())
        return false;
    
    const int lPolygonCount = pMesh->GetPolygonCount();
    
    // Count the polygon count of each material
    FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
    FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
    
    if (pMesh->GetElementMaterial())
    {
        lMaterialIndice = &pMesh->GetElementMaterial()->GetIndexArray();
        lMaterialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
        
        if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
        {
            FBX_ASSERT(lMaterialIndice->GetCount() == lPolygonCount);
            
            if (lMaterialIndice->GetCount() == lPolygonCount)
            {
                // Count the faces of each material
                for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
                {
                    const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
                    if (mSubMeshes.GetCount() < lMaterialIndex + 1)
                    {
                        mSubMeshes.Resize(lMaterialIndex + 1);
                    }
                    if (mSubMeshes[lMaterialIndex] == NULL)
                    {
                        mSubMeshes[lMaterialIndex] = new SubMesh;
                    }
                    mSubMeshes[lMaterialIndex]->TriangleCount += 1;
                }
                
                // Make sure we have no "holes" (NULL) in the mSubMeshes table. This can happen
                // if, in the loop above, we resized the mSubMeshes by more than one slot.
                for (int i = 0; i < mSubMeshes.GetCount(); i++)
                {
                    if (mSubMeshes[i] == NULL)
                        mSubMeshes[i] = new SubMesh;
                }
                
                // Record the offset (how many vertex)
                const int lMaterialCount = mSubMeshes.GetCount();
                int lOffset = 0;
                for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
                {
                    mSubMeshes[lIndex]->IndexOffset = lOffset;
                    lOffset += mSubMeshes[lIndex]->TriangleCount * 3;
                    // This will be used as counter in the following procedures, reset to zero
                    mSubMeshes[lIndex]->TriangleCount = 0;
                }
                FBX_ASSERT(lOffset == lPolygonCount * 3);
            }
        }
    }
    
    // All faces will use the same material.
    if (mSubMeshes.GetCount() == 0)
    {
        mSubMeshes.Resize(1);
        mSubMeshes[0] = new SubMesh();
    }
    
    // Congregate all the data of a mesh to be cached in VBOs.
    // If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
    mHasNormal = pMesh->GetElementNormalCount() > 0;
    mHasUV = pMesh->GetElementUVCount() > 0;
    
    FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
    
    if (mHasNormal)
    {
        lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
        
        if (lNormalMappingMode == FbxGeometryElement::eNone)
        {
            mHasNormal = false;
        }
        if (mHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
        {
            mAllByControlPoint = false;
        }
    }
    if (mHasUV)
    {
        lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
        
        if (lUVMappingMode == FbxGeometryElement::eNone)
        {
            mHasUV = false;
        }
        if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
        {
            mAllByControlPoint = false;
        }
    }
    
    // Allocate the array memory, by control point or by polygon vertex.
    int lPolygonVertexCount = pMesh->GetControlPointsCount();
    
    if (!mAllByControlPoint)
    {
        lPolygonVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
    }
  
    mVertices = new std::vector<float>[lPolygonVertexCount * VERTEX_STRIDE];
    mIndices = new std::vector<GLuint>[lPolygonCount * TRIANGLE_VERTEX_COUNT];
    mNormals = NULL;
    
    if (mHasNormal)
    {
        mNormals = new std::vector<float>[lPolygonVertexCount * NORMAL_STRIDE];
    }
    
    mUVs = NULL;
    FbxStringList lUVNames;
    pMesh->GetUVSetNames(lUVNames);
    const char * lUVName = NULL;
    
    if (mHasUV && lUVNames.GetCount())
    {
        mUVs = new std::vector<float>[lPolygonVertexCount * UV_STRIDE];
        lUVName = lUVNames[0];
    }
    
    // Populate the array with vertex attribute, if by control point.
    const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
    FbxVector4 lCurrentVertex;
    FbxVector4 lCurrentNormal;
    FbxVector2 lCurrentUV;
    
    if (mAllByControlPoint)
    {
        const FbxGeometryElementNormal * lNormalElement = NULL;
        const FbxGeometryElementUV * lUVElement = NULL;
        
        if (mHasNormal)
        {
            lNormalElement = pMesh->GetElementNormal(0);
        }
        
        if (mHasUV)
        {
            lUVElement = pMesh->GetElementUV(0);
        }
        
        for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
        {
            // Save the vertex position.
            lCurrentVertex = lControlPoints[lIndex];
            mVertices->insert(mVertices->begin() + (lIndex * VERTEX_STRIDE), static_cast<float>(lCurrentVertex[0]));
            mVertices->insert(mVertices->begin() + (lIndex * VERTEX_STRIDE + 1), static_cast<float>(lCurrentVertex[1]));
            mVertices->insert(mVertices->begin() + (lIndex * VERTEX_STRIDE + 2), static_cast<float>(lCurrentVertex[2]));
//            mVertices->insert(mVertices->begin() + (lIndex * VERTEX_STRIDE + 3), 1);
            
            // Save the normal.
            if (mHasNormal)
            {
                int lNormalIndex = lIndex;
                
                if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
                }
                
                lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
                mNormals->insert(mNormals->begin() + (lIndex * NORMAL_STRIDE), static_cast<float>(lCurrentNormal[0]));
                mNormals->insert(mNormals->begin() + (lIndex * NORMAL_STRIDE + 1), static_cast<float>(lCurrentNormal[1]));
                mNormals->insert(mNormals->begin() + (lIndex * NORMAL_STRIDE + 2), static_cast<float>(lCurrentNormal[2]));
            }
            
            // Save the UV.
            if (mHasUV)
            {
                int lUVIndex = lIndex;
                
                if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
                }
                
                lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
                mUVs->insert(mUVs->begin() + (lIndex * UV_STRIDE), static_cast<float>(lCurrentUV[0]));
                mUVs->insert(mUVs->begin() + (lIndex * UV_STRIDE + 1), static_cast<float>(lCurrentUV[1]));
            }
        }
        
    }
    
    int lVertexCount = 0;
    
    for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
    {
        // The material for current face.
        int lMaterialIndex = 0;
        
        if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
        {
            lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
        }
        
        // Where should I save the vertex attribute index, according to the material
        const int lIndexOffset = mSubMeshes[lMaterialIndex]->IndexOffset +
        mSubMeshes[lMaterialIndex]->TriangleCount * 3;
        
        for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
        {
            const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
            
            if (mAllByControlPoint)
            {
                mIndices->insert(mIndices->begin() + (lIndexOffset + lVerticeIndex), static_cast<GLuint>(lControlPointIndex));
            }
            // Populate the array with vertex attribute, if by polygon vertex.
            else
            {
                mIndices->insert(mIndices->begin() + (lIndexOffset + lVerticeIndex), static_cast<GLuint>(lVertexCount));
                
                lCurrentVertex = lControlPoints[lControlPointIndex];
                mVertices->insert(mVertices->begin() + (lVertexCount * VERTEX_STRIDE), static_cast<float>(lCurrentVertex[0]));
                mVertices->insert(mVertices->begin() + (lVertexCount * VERTEX_STRIDE + 1), static_cast<float>(lCurrentVertex[1]));
                mVertices->insert(mVertices->begin() + (lVertexCount * VERTEX_STRIDE + 2), static_cast<float>(lCurrentVertex[2]));
//                mVertices->insert(mVertices->begin() + (lVertexCount * VERTEX_STRIDE + 3), 1);
                
                if (mHasNormal)
                {
                    pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
                    mNormals->insert(mNormals->begin() + (lVertexCount * NORMAL_STRIDE), static_cast<float>(lCurrentNormal[0]));
                    mNormals->insert(mNormals->begin() + (lVertexCount * NORMAL_STRIDE + 1), static_cast<float>(lCurrentNormal[1]));
                    mNormals->insert(mNormals->begin() + (lVertexCount * NORMAL_STRIDE + 2), static_cast<float>(lCurrentNormal[2]));
                }
                
                if (mHasUV)
                {
                    pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV);
                    mUVs->insert(mUVs->begin() + (lVertexCount * UV_STRIDE), static_cast<float>(lCurrentUV[0]));
                    mUVs->insert(mUVs->begin() + (lVertexCount * UV_STRIDE + 1), static_cast<float>(lCurrentUV[1]));
                }
            }
            ++lVertexCount;
        }
        mSubMeshes[lMaterialIndex]->TriangleCount += 1;
    }
    
    return true;
}

MaterialCache::MaterialCache() : mShinness(0)
{
    
}

MaterialCache::~MaterialCache()
{
    
}

// Bake material properties.
bool MaterialCache::Initialize(const FbxSurfaceMaterial * pMaterial)
{
    const FbxDouble3 lEmissive = GetMaterialProperty(pMaterial,
                                                     FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, mEmissive.mTextureName);
    mEmissive.mColor[0] = static_cast<GLfloat>(lEmissive[0]);
    mEmissive.mColor[1] = static_cast<GLfloat>(lEmissive[1]);
    mEmissive.mColor[2] = static_cast<GLfloat>(lEmissive[2]);
    
    const FbxDouble3 lAmbient = GetMaterialProperty(pMaterial,
                                                    FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, mAmbient.mTextureName);
    mAmbient.mColor[0] = static_cast<GLfloat>(lAmbient[0]);
    mAmbient.mColor[1] = static_cast<GLfloat>(lAmbient[1]);
    mAmbient.mColor[2] = static_cast<GLfloat>(lAmbient[2]);
    
    const FbxDouble3 lDiffuse = GetMaterialProperty(pMaterial,
                                                    FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, mDiffuse.mTextureName);
    mDiffuse.mColor[0] = static_cast<GLfloat>(lDiffuse[0]);
    mDiffuse.mColor[1] = static_cast<GLfloat>(lDiffuse[1]);
    mDiffuse.mColor[2] = static_cast<GLfloat>(lDiffuse[2]);
    
    const FbxDouble3 lSpecular = GetMaterialProperty(pMaterial,
                                                     FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, mSpecular.mTextureName);
    mSpecular.mColor[0] = static_cast<GLfloat>(lSpecular[0]);
    mSpecular.mColor[1] = static_cast<GLfloat>(lSpecular[1]);
    mSpecular.mColor[2] = static_cast<GLfloat>(lSpecular[2]);
    
    FbxProperty lShininessProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
    if (lShininessProperty.IsValid())
    {
        double lShininess = lShininessProperty.Get<FbxDouble>();
        mShinness = static_cast<GLfloat>(lShininess);
    }
    
    return true;
}