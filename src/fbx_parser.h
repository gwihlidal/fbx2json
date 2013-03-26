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

#ifndef FBX2JSON_FBXPARSER_H
#define FBX2JSON_FBXPARSER_H

#include <vector>
#include <fbxsdk.h>
#include "fbx_deformation.h"
#include "fbx_position.h"
#include "fbx_vbomesh.h"

namespace Fbx2Json
{

class Parser
{
  public:
    Parser();
    void parse(FbxScene* pScene);
    std::vector<VBOMesh *> * get_meshes() {
      return meshes;
    };
    ~Parser();

  private:
    void bake_meshes_recursive(FbxNode * node, FbxAnimLayer * animation_layer);
    void bake_mesh_deformations(FbxMesh* mesh, VBOMesh * mesh_cache, FbxTime& time, FbxAnimLayer* animation_layer, FbxAMatrix& global_offset_position, FbxPose* pose);
    void bake_global_positions(FbxVector4* control_points, int control_points_count, FbxAMatrix& global_offset_position);
    void read_vertex_cache_data(FbxMesh* mesh, FbxTime& time, FbxVector4* vertex_array);

    std::vector<VBOMesh *> * meshes;
};

} // namespace Fbx2Json

#endif
