/*
 * Copyright 2013 Cameron Yule.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "fbx_deformation.h"

namespace Fbx2Json
{

// Deform the vertex array with the shapes contained in the mesh.
void compute_shape_deformation(FbxMesh* pMesh, FbxTime& pTime, FbxAnimLayer * pAnimLayer, FbxVector4* pVertexArray)
{
  int lVertexCount = pMesh->GetControlPointsCount();

  FbxVector4* lSrcVertexArray = pVertexArray;
  FbxVector4* lDstVertexArray = new FbxVector4[lVertexCount];
  memcpy(lDstVertexArray, pVertexArray, lVertexCount * sizeof(FbxVector4));

  int lBlendShapeDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eBlendShape);

  for(int lBlendShapeIndex = 0; lBlendShapeIndex<lBlendShapeDeformerCount; ++lBlendShapeIndex) {
    FbxBlendShape* lBlendShape = (FbxBlendShape*)pMesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);

    int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();

    for(int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; ++lChannelIndex) {
      FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);

      if(lChannel) {
        // Get the percentage of influence of the shape.
        FbxAnimCurve* lFCurve = pMesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer);

        if(!lFCurve) {
          continue;
        }

        double lWeight = lFCurve->Evaluate(pTime);

        //Find which shape should we use according to the weight.
        int lShapeCount = lChannel->GetTargetShapeCount();
        double* lFullWeights = lChannel->GetTargetShapeFullWeights();

        for(int lShapeIndex = 0; lShapeIndex<lShapeCount; ++lShapeIndex) {
          FbxShape* lShape = NULL;

          if(lWeight > 0 && lWeight <= lFullWeights[0]) {
            lShape = lChannel->GetTargetShape(0);
          }

          if(lWeight > lFullWeights[lShapeIndex] && lWeight < lFullWeights[lShapeIndex+1]) {
            lShape = lChannel->GetTargetShape(lShapeIndex+1);
          }

          if(lShape) {
            for(int j = 0; j < lVertexCount; j++) {
              // Add the influence of the shape vertex to the mesh vertex.
              FbxVector4 lInfluence = (lShape->GetControlPoints()[j] - lSrcVertexArray[j]) * lWeight * 0.01;
              lDstVertexArray[j] += lInfluence;
            }
          }
        }//For each target shape
      }//If lChannel is valid
    }//For each blend shape channel
  }//For each blend shape deformer

  memcpy(pVertexArray, lDstVertexArray, lVertexCount * sizeof(FbxVector4));

  delete [] lDstVertexArray;
}

//Compute the transform matrix that the cluster will transform the vertex.
void compute_cluster_deformation(FbxAMatrix& pGlobalPosition,
                                 FbxMesh* pMesh,
                                 FbxCluster* pCluster,
                                 FbxAMatrix& pVertexTransformMatrix,
                                 FbxTime pTime,
                                 FbxPose* pPose)
{
  FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

  FbxAMatrix lReferenceGlobalInitPosition;
  FbxAMatrix lReferenceGlobalCurrentPosition;
  FbxAMatrix lAssociateGlobalInitPosition;
  FbxAMatrix lAssociateGlobalCurrentPosition;
  FbxAMatrix lClusterGlobalInitPosition;
  FbxAMatrix lClusterGlobalCurrentPosition;

  FbxAMatrix lReferenceGeometry;
  FbxAMatrix lAssociateGeometry;
  FbxAMatrix lClusterGeometry;

  FbxAMatrix lClusterRelativeInitPosition;
  FbxAMatrix lClusterRelativeCurrentPositionInverse;

  if(lClusterMode == FbxCluster::eAdditive && pCluster->GetAssociateModel()) {
    pCluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
    // Geometric transform of the model
    lAssociateGeometry = get_geometry(pCluster->GetAssociateModel());
    lAssociateGlobalInitPosition *= lAssociateGeometry;
    lAssociateGlobalCurrentPosition = get_global_position(pCluster->GetAssociateModel(), pTime, pPose);

    pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
    // Multiply lReferenceGlobalInitPosition by Geometric Transformation
    lReferenceGeometry = get_geometry(pMesh->GetNode());
    lReferenceGlobalInitPosition *= lReferenceGeometry;
    lReferenceGlobalCurrentPosition = pGlobalPosition;

    // Get the link initial global position and the link current global position.
    pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
    // Multiply lClusterGlobalInitPosition by Geometric Transformation
    lClusterGeometry = get_geometry(pCluster->GetLink());
    lClusterGlobalInitPosition *= lClusterGeometry;
    lClusterGlobalCurrentPosition = get_global_position(pCluster->GetLink(), pTime, pPose);

    // Compute the shift of the link relative to the reference.
    //ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
    pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
                             lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
  } else {
    pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
    lReferenceGlobalCurrentPosition = pGlobalPosition;
    // Multiply lReferenceGlobalInitPosition by Geometric Transformation
    lReferenceGeometry = get_geometry(pMesh->GetNode());
    lReferenceGlobalInitPosition *= lReferenceGeometry;

    // Get the link initial global position and the link current global position.
    pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
    lClusterGlobalCurrentPosition = get_global_position(pCluster->GetLink(), pTime, pPose);

    // Compute the initial position of the link relative to the reference.
    lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

    // Compute the current position of the link relative to the reference.
    lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

    // Compute the shift of the link relative to the reference.
    pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
  }
}

// Deform the vertex array in classic linear way.
void compute_linear_deformation(FbxAMatrix& pGlobalPosition,
                                FbxMesh* pMesh,
                                FbxTime& pTime,
                                FbxVector4* pVertexArray,
                                FbxPose* pPose)
{
  // All the links must have the same link mode.
  FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

  int lVertexCount = pMesh->GetControlPointsCount();
  FbxAMatrix* lClusterDeformation = new FbxAMatrix[lVertexCount];
  memset(lClusterDeformation, 0, lVertexCount * sizeof(FbxAMatrix));

  double* lClusterWeight = new double[lVertexCount];
  memset(lClusterWeight, 0, lVertexCount * sizeof(double));

  if(lClusterMode == FbxCluster::eAdditive) {
    for(int i = 0; i < lVertexCount; ++i) {
      lClusterDeformation[i].SetIdentity();
    }
  }

  // For all skins and all clusters, accumulate their deformation and weight
  // on each vertices and store them in lClusterDeformation and lClusterWeight.
  int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

  for(int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex) {
    FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);

    int lClusterCount = lSkinDeformer->GetClusterCount();

    for(int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex) {
      FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);

      if(!lCluster->GetLink()) {
        continue;
      }

      FbxAMatrix lVertexTransformMatrix;
      compute_cluster_deformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

      int lVertexIndexCount = lCluster->GetControlPointIndicesCount();

      for(int k = 0; k < lVertexIndexCount; ++k) {
        int lIndex = lCluster->GetControlPointIndices()[k];

        // Sometimes, the mesh can have less points than at the time of the skinning
        // because a smooth operator was active when skinning but has been deactivated during export.
        if(lIndex >= lVertexCount) {
          continue;
        }

        double lWeight = lCluster->GetControlPointWeights()[k];

        if(lWeight == 0.0) {
          continue;
        }

        // Compute the influence of the link on the vertex.
        FbxAMatrix lInfluence = lVertexTransformMatrix;
        matrix_scale(lInfluence, lWeight);

        if(lClusterMode == FbxCluster::eAdditive) {
          // Multiply with the product of the deformations on the vertex.
          matrix_add_to_diagonal(lInfluence, 1.0 - lWeight);
          lClusterDeformation[lIndex] = lInfluence * lClusterDeformation[lIndex];

          // Set the link to 1.0 just to know this vertex is influenced by a link.
          lClusterWeight[lIndex] = 1.0;
        } else { // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
          // Add to the sum of the deformations on the vertex.
          matrix_add(lClusterDeformation[lIndex], lInfluence);

          // Add to the sum of weights to either normalize or complete the vertex.
          lClusterWeight[lIndex] += lWeight;
        }
      }//For each vertex
    }//lClusterCount
  }

  //Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
  for(int i = 0; i < lVertexCount; i++) {
    FbxVector4 lSrcVertex = pVertexArray[i];
    FbxVector4& lDstVertex = pVertexArray[i];
    double lWeight = lClusterWeight[i];

    // Deform the vertex if there was at least a link with an influence on the vertex,
    if(lWeight != 0.0) {
      lDstVertex = lClusterDeformation[i].MultT(lSrcVertex);

      if(lClusterMode == FbxCluster::eNormalize) {
        // In the normalized link mode, a vertex is always totally influenced by the links.
        lDstVertex /= lWeight;
      } else if(lClusterMode == FbxCluster::eTotalOne) {
        // In the total 1 link mode, a vertex can be partially influenced by the links.
        lSrcVertex *= (1.0 - lWeight);
        lDstVertex += lSrcVertex;
      }
    }
  }

  delete [] lClusterDeformation;
  delete [] lClusterWeight;
}

// Deform the vertex array in Dual Quaternion Skinning way.
void compute_dual_quaternion_deformation(FbxAMatrix& pGlobalPosition,
    FbxMesh* pMesh,
    FbxTime& pTime,
    FbxVector4* pVertexArray,
    FbxPose* pPose)
{
  // All the links must have the same link mode.
  FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

  int lVertexCount = pMesh->GetControlPointsCount();
  int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

  FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[lVertexCount];
  memset(lDQClusterDeformation, 0, lVertexCount * sizeof(FbxDualQuaternion));

  double* lClusterWeight = new double[lVertexCount];
  memset(lClusterWeight, 0, lVertexCount * sizeof(double));

  // For all skins and all clusters, accumulate their deformation and weight
  // on each vertices and store them in lClusterDeformation and lClusterWeight.
  for(int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex) {
    FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
    int lClusterCount = lSkinDeformer->GetClusterCount();

    for(int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex) {
      FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);

      if(!lCluster->GetLink()) {
        continue;
      }

      FbxAMatrix lVertexTransformMatrix;
      compute_cluster_deformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

      FbxQuaternion lQ = lVertexTransformMatrix.GetQ();
      FbxVector4 lT = lVertexTransformMatrix.GetT();
      FbxDualQuaternion lDualQuaternion(lQ, lT);

      int lVertexIndexCount = lCluster->GetControlPointIndicesCount();

      for(int k = 0; k < lVertexIndexCount; ++k) {
        int lIndex = lCluster->GetControlPointIndices()[k];

        // Sometimes, the mesh can have less points than at the time of the skinning
        // because a smooth operator was active when skinning but has been deactivated during export.
        if(lIndex >= lVertexCount) {
          continue;
        }

        double lWeight = lCluster->GetControlPointWeights()[k];

        if(lWeight == 0.0) {
          continue;
        }

        // Compute the influence of the link on the vertex.
        FbxDualQuaternion lInfluence = lDualQuaternion * lWeight;

        if(lClusterMode == FbxCluster::eAdditive) {
          // Simply influenced by the dual quaternion.
          lDQClusterDeformation[lIndex] = lInfluence;

          // Set the link to 1.0 just to know this vertex is influenced by a link.
          lClusterWeight[lIndex] = 1.0;
        } else { // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
          if(lClusterIndex == 0) {
            lDQClusterDeformation[lIndex] = lInfluence;
          } else {
            // Add to the sum of the deformations on the vertex.
            // Make sure the deformation is accumulated in the same rotation direction.
            // Use dot product to judge the sign.
            double lSign = lDQClusterDeformation[lIndex].GetFirstQuaternion().DotProduct(lDualQuaternion.GetFirstQuaternion());

            if(lSign >= 0.0) {
              lDQClusterDeformation[lIndex] += lInfluence;
            } else {
              lDQClusterDeformation[lIndex] -= lInfluence;
            }
          }

          // Add to the sum of weights to either normalize or complete the vertex.
          lClusterWeight[lIndex] += lWeight;
        }
      }//For each vertex
    }//lClusterCount
  }

  //Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
  for(int i = 0; i < lVertexCount; i++) {
    FbxVector4 lSrcVertex = pVertexArray[i];
    FbxVector4& lDstVertex = pVertexArray[i];
    double lWeightSum = lClusterWeight[i];

    // Deform the vertex if there was at least a link with an influence on the vertex,
    if(lWeightSum != 0.0) {
      lDQClusterDeformation[i].Normalize();
      lDstVertex = lDQClusterDeformation[i].Deform(lDstVertex);

      if(lClusterMode == FbxCluster::eNormalize) {
        // In the normalized link mode, a vertex is always totally influenced by the links.
        lDstVertex /= lWeightSum;
      } else if(lClusterMode == FbxCluster::eTotalOne) {
        // In the total 1 link mode, a vertex can be partially influenced by the links.
        lSrcVertex *= (1.0 - lWeightSum);
        lDstVertex += lSrcVertex;
      }
    }
  }

  delete [] lDQClusterDeformation;
  delete [] lClusterWeight;
}

// Deform the vertex array according to the links contained in the mesh and the skinning type.
void compute_skin_deformation(FbxAMatrix& pGlobalPosition,
                              FbxMesh* pMesh,
                              FbxTime& pTime,
                              FbxVector4* pVertexArray,
                              FbxPose* pPose)
{
  FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(0, FbxDeformer::eSkin);
  FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();

  if(lSkinningType == FbxSkin::eLinear || lSkinningType == FbxSkin::eRigid) {
    compute_linear_deformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
  } else if(lSkinningType == FbxSkin::eDualQuaternion) {
    compute_dual_quaternion_deformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
  } else if(lSkinningType == FbxSkin::eBlend) {
    int lVertexCount = pMesh->GetControlPointsCount();

    FbxVector4* lVertexArrayLinear = new FbxVector4[lVertexCount];
    memcpy(lVertexArrayLinear, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

    FbxVector4* lVertexArrayDQ = new FbxVector4[lVertexCount];
    memcpy(lVertexArrayDQ, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

    compute_linear_deformation(pGlobalPosition, pMesh, pTime, lVertexArrayLinear, pPose);
    compute_dual_quaternion_deformation(pGlobalPosition, pMesh, pTime, lVertexArrayDQ, pPose);

    // To blend the skinning according to the blend weights
    // Final vertex = DQSVertex * blend weight + LinearVertex * (1- blend weight)
    // DQSVertex: vertex that is deformed by dual quaternion skinning method;
    // LinearVertex: vertex that is deformed by classic linear skinning method;
    int lBlendWeightsCount = lSkinDeformer->GetControlPointIndicesCount();

    for(int lBWIndex = 0; lBWIndex<lBlendWeightsCount; ++lBWIndex) {
      double lBlendWeight = lSkinDeformer->GetControlPointBlendWeights()[lBWIndex];
      pVertexArray[lBWIndex] = lVertexArrayDQ[lBWIndex] * lBlendWeight + lVertexArrayLinear[lBWIndex] * (1 - lBlendWeight);
    }
  }
}

// Scale all the elements of a matrix.
void matrix_scale(FbxAMatrix& pMatrix, double pValue)
{
  int i,j;

  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      pMatrix[i][j] *= pValue;
    }
  }
}


// Add a value to all the elements in the diagonal of the matrix.
void matrix_add_to_diagonal(FbxAMatrix& pMatrix, double pValue)
{
  pMatrix[0][0] += pValue;
  pMatrix[1][1] += pValue;
  pMatrix[2][2] += pValue;
  pMatrix[3][3] += pValue;
}


// Sum two matrices element by element.
void matrix_add(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix)
{
  int i,j;

  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      pDstMatrix[i][j] += pSrcMatrix[i][j];
    }
  }
}

} // namespace Fbx2Json
