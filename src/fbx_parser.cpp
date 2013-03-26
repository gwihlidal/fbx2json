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

#include "fbx_parser.h"

namespace Fbx2Json
{

Parser::Parser()
{

}

// TODO: Capture scene texture filenames, convert to web-safe format
// TODO: Materials
void Parser::parse(FbxScene * scene)
{
  meshes = new std::vector<VBOMesh *>;
  FbxAnimLayer * animation_layer = NULL;

  FbxNode * node = scene->GetRootNode();

  bake_meshes_recursive(node, animation_layer);
}

void Parser::bake_meshes_recursive(FbxNode * node, FbxAnimLayer * animation_layer)
{
  FbxPose * pose = NULL;
  FbxTime current_time;
  FbxAMatrix parent_global_position;

  FbxAMatrix global_position = get_global_position(node, current_time, pose, &parent_global_position);

  FbxNodeAttribute* node_attribute = node->GetNodeAttribute();

  if(node_attribute) {
    if(node_attribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
      FbxMesh * mesh = node->GetMesh();
        
      FbxAMatrix geometry_offset = get_geometry(node);
      FbxAMatrix global_offset_position = global_position * geometry_offset;

      FbxVector4* control_points = mesh->GetControlPoints();
      bake_global_positions(control_points, mesh->GetControlPointsCount(), global_offset_position);

      if(mesh && !mesh->GetUserDataPtr()) {
        VBOMesh * mesh_cache = new VBOMesh;

        if(mesh_cache->initialize(mesh)) {
          bake_mesh_deformations(mesh, mesh_cache, current_time, animation_layer, global_offset_position, pose);

          meshes->push_back(mesh_cache);
        }
      }
    }
  }

  const int node_child_count = node->GetChildCount();

  for(int node_child_index = 0; node_child_index < node_child_count; ++node_child_index) {
    bake_meshes_recursive(node->GetChild(node_child_index), animation_layer);
  }
}
    
void Parser::bake_global_positions(FbxVector4* control_points, int control_points_count, FbxAMatrix& global_offset_position)
{
    for (int i = 0; i < control_points_count; i++) {
        control_points[i] = global_offset_position.MultT(control_points[i]);
    }
}

// In FBX, geometries can be deformed using skinning, shapes, or vertex caches.
void Parser::bake_mesh_deformations(FbxMesh* mesh, VBOMesh * mesh_cache, FbxTime& current_time, FbxAnimLayer* animation_layer, FbxAMatrix& global_offset_position, FbxPose* pose)
{
  const int vertex_count = mesh->GetControlPointsCount();

  if(vertex_count == 0) {
    return;
  }

  //  VBOMesh * mesh_cache = static_cast<VBOMesh *>(mesh->GetUserDataPtr());

  // If it has some defomer connection, update the vertices position
  const bool has_shape = mesh->GetShapeCount() > 0;
  const bool has_skin = mesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
  const bool has_deformation = has_shape || has_skin;

  FbxVector4* vertex_array = NULL;

  if(!mesh_cache || has_deformation) {
    vertex_array = new FbxVector4[vertex_count];
    memcpy(vertex_array, mesh->GetControlPoints(), vertex_count * sizeof(FbxVector4));
    bake_global_positions(vertex_array, vertex_count, global_offset_position);
  }

  if(has_deformation) {
      if(has_shape) {
        // Deform the vertex array with the shapes.
        compute_shape_deformation(mesh, current_time, animation_layer, vertex_array);
      }

      //we need to get the number of clusters
      const int skin_count = mesh->GetDeformerCount(FbxDeformer::eSkin);
      int cluster_count = 0;

      for(int skin_index = 0; skin_index < skin_count; ++skin_index) {
        cluster_count += ((FbxSkin *)(mesh->GetDeformer(skin_index, FbxDeformer::eSkin)))->GetClusterCount();
      }

      if(cluster_count) {
        // Deform the vertex array with the skin deformer.
        compute_skin_deformation(global_offset_position, mesh, current_time, vertex_array, pose);
      }
    }

    if(mesh_cache && vertex_array) {
      mesh_cache->update_vertex_position(mesh, vertex_array);
    }

  delete [] vertex_array;
}

Parser::~Parser()
{

}

} // namespace Fbx2Json
