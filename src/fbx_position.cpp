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

#include "fbx_position.h"

namespace Fbx2Json
{

FbxAMatrix get_global_position(FbxNode* node, const FbxTime& time, FbxPose* pose, FbxAMatrix* parent_global_position)
{
  FbxAMatrix global_position;
  bool        position_found = false;

  if(pose) {
    int node_index = pose->Find(node);

    if(node_index > -1) {
      // The bind pose is always a global matrix.
      // If we have a rest pose, we need to check if it is stored in global or local space.
      if(pose->IsBindPose() || !pose->IsLocalMatrix(node_index)) {
        global_position = get_pose_matrix(pose, node_index);
      } else {
        // We have a local matrix, we need to convert it to a global space matrix.
        FbxAMatrix local_parent_global_position;

        if(parent_global_position) {
          local_parent_global_position = *parent_global_position;
        } else {
          if(node->GetParent()) {
            local_parent_global_position = get_global_position(node->GetParent(), time, pose);
          }
        }

        FbxAMatrix local_position = get_pose_matrix(pose, node_index);
        global_position = local_parent_global_position * local_position;
      }

      position_found = true;
    }
  }

  if(!position_found) {
    global_position = node->EvaluateGlobalTransform(time);
  }

  return global_position;
}

FbxAMatrix get_pose_matrix(FbxPose* pose, int node_index)
{
  FbxAMatrix pose_matrix;
  FbxMatrix matrix = pose->GetMatrix(node_index);

  memcpy((double*)pose_matrix, (double*)matrix, sizeof(matrix.mData));

  return pose_matrix;
}

FbxAMatrix get_geometry(FbxNode* node)
{
  const FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
  const FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
  const FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);

  return FbxAMatrix(lT, lR, lS);
}

} // namespace Fbx2Json
