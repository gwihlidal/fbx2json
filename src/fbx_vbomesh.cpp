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

#include "fbx_vbomesh.h"

namespace Fbx2Json
{

const float ANGLE_TO_RADIAN = 3.1415926f / 180.f;
const int TRIANGLE_VERTEX_COUNT = 3;
const int VERTEX_STRIDE = 4;
const int NORMAL_STRIDE = 3;
const int UV_STRIDE = 2;

VBOMesh::VBOMesh() : has_normal(false), has_uv(false), all_by_control_points(true)
{
  // Reset every VBO to zero, which means no buffer.
  for(int i = 0; i < VBO_COUNT; ++i) {
    vbo_names[i] = 0;
  }
}

VBOMesh::~VBOMesh()
{
  for(int i=0; i < submeshes.GetCount(); i++) {
    delete submeshes[i];
  }

  //  vertices->clear();
  //  indices->clear();
  //  normals->clear();
  //  uvs->clear();

  submeshes.Clear();
}

bool VBOMesh::initialize(const FbxMesh *mesh)
{
  if(!mesh->GetNode()) {
    return false;
  }

  const int polygon_count = mesh->GetPolygonCount();

  // Count the polygon count of each material
  FbxLayerElementArrayTemplate<int>* material_indice = NULL;
  FbxGeometryElement::EMappingMode material_mapping_mode = FbxGeometryElement::eNone;

  if(mesh->GetElementMaterial()) {
    material_indice = &mesh->GetElementMaterial()->GetIndexArray();
    material_mapping_mode = mesh->GetElementMaterial()->GetMappingMode();

    if(material_indice && material_mapping_mode == FbxGeometryElement::eByPolygon) {

      if(material_indice->GetCount() == polygon_count) {
        // Count the faces of each material
        for(int i = 0; i < polygon_count; ++i) {
          const int material_index = material_indice->GetAt(i);

          if(submeshes.GetCount() < material_index + 1) {
            submeshes.Resize(material_index + 1);
          }

          if(submeshes[material_index] == NULL) {
            submeshes[material_index] = new SubMesh;
          }

          submeshes[material_index]->triangle_count += 1;
        }

        // Make sure we have no "holes" (NULL) in the mSubMeshes table. This can happen
        // if, in the loop above, we resized the mSubMeshes by more than one slot.
        for(int i = 0; i < submeshes.GetCount(); i++) {
          if(submeshes[i] == NULL) {
            submeshes[i] = new SubMesh;
          }
        }

        // Record the offset (how many vertex)
        const int material_count = submeshes.GetCount();
        int offset = 0;

        for(int i = 0; i < material_count; ++i) {
          submeshes[i]->index_offset = offset;
          offset += submeshes[i]->triangle_count * 3;
          // This will be used as counter in the following procedures, reset to zero
          submeshes[i]->triangle_count = 0;
        }
      }
    }
  }

  // All faces will use the same material.
  if(submeshes.GetCount() == 0) {
    submeshes.Resize(1);
    submeshes[0] = new SubMesh();
  }

  // Congregate all the data of a mesh to be cached in VBOs.
  // If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
  has_normal = mesh->GetElementNormalCount() > 0;
  has_uv = mesh->GetElementUVCount() > 0;

  FbxGeometryElement::EMappingMode normal_mapping_mode = FbxGeometryElement::eNone;
  FbxGeometryElement::EMappingMode uv_mapping_mode = FbxGeometryElement::eNone;

  if(has_normal) {
    normal_mapping_mode = mesh->GetElementNormal(0)->GetMappingMode();

    if(normal_mapping_mode == FbxGeometryElement::eNone) {
      has_normal = false;
    }

    if(has_normal && normal_mapping_mode != FbxGeometryElement::eByControlPoint) {
      all_by_control_points = false;
    }
  }

  if(has_uv) {
    uv_mapping_mode = mesh->GetElementUV(0)->GetMappingMode();

    if(uv_mapping_mode == FbxGeometryElement::eNone) {
      has_uv = false;
    }

    if(has_uv && uv_mapping_mode != FbxGeometryElement::eByControlPoint) {
      all_by_control_points = false;
    }
  }

  // Allocate the array memory, by control point or by polygon vertex.
  int polygon_vertex_count = mesh->GetControlPointsCount();

  if(!all_by_control_points) {
    polygon_vertex_count = polygon_count * TRIANGLE_VERTEX_COUNT;
  }

  vertices = std::vector<float>(polygon_vertex_count * VERTEX_STRIDE);
  indices = std::vector<GLuint>(polygon_count * TRIANGLE_VERTEX_COUNT);
  //  normals = NULL;

  if(has_normal) {
    normals = std::vector<float>(polygon_vertex_count * NORMAL_STRIDE);
  }

  //  uvs = NULL;
  FbxStringList uv_names;
  mesh->GetUVSetNames(uv_names);
  const char * uv_name = NULL;

  if(has_uv && uv_names.GetCount()) {
    uvs = std::vector<float>(polygon_vertex_count * UV_STRIDE);
    uv_name = uv_names[0];
  }

  // Populate the array with vertex attribute, if by control point.
  const FbxVector4 * control_points = mesh->GetControlPoints();
  FbxVector4 current_vertex;
  FbxVector4 current_normal;
  FbxVector2 current_uv;

  if(all_by_control_points) {
    const FbxGeometryElementNormal * normal_element = NULL;
    const FbxGeometryElementUV * uv_element = NULL;

    if(has_normal) {
      normal_element = mesh->GetElementNormal(0);
    }

    if(has_uv) {
      uv_element = mesh->GetElementUV(0);
    }

    for(int i = 0; i < polygon_vertex_count; ++i) {
      // Save the vertex position.
      current_vertex = control_points[i];
      vertices[i * VERTEX_STRIDE] = static_cast<float>(current_vertex[0]);
      vertices[i * VERTEX_STRIDE + 1] = static_cast<float>(current_vertex[1]);
      vertices[i * VERTEX_STRIDE + 2] = static_cast<float>(current_vertex[2]);
      vertices[i * VERTEX_STRIDE + 3] = 1;

      // Save the normal.
      if(has_normal) {
        int normal_index = i;

        if(normal_element->GetReferenceMode() == FbxLayerElement::eIndexToDirect) {
          normal_index = normal_element->GetIndexArray().GetAt(i);
        }

        current_normal = normal_element->GetDirectArray().GetAt(normal_index);
        normals[i * NORMAL_STRIDE] = static_cast<float>(current_normal[0]);
        normals[i * NORMAL_STRIDE + 1] = static_cast<float>(current_normal[1]);
        normals[i * NORMAL_STRIDE + 2] = static_cast<float>(current_normal[2]);
      }

      // Save the UV.
      if(has_uv) {
        int uv_index = i;

        if(uv_element->GetReferenceMode() == FbxLayerElement::eIndexToDirect) {
          uv_index = uv_element->GetIndexArray().GetAt(i);
        }

        current_uv = uv_element->GetDirectArray().GetAt(uv_index);
        uvs[i * UV_STRIDE] = static_cast<float>(current_uv[0]);
        uvs[i * UV_STRIDE + 1] = static_cast<float>(current_uv[1]);
      }
    }

  }

  int vertex_count = 0;

  for(int polygon_index = 0; polygon_index < polygon_count; ++polygon_index) {
    // The material for current face.
    int material_index = 0;

    if(material_indice && material_mapping_mode == FbxGeometryElement::eByPolygon) {
      material_index = material_indice->GetAt(polygon_index);
    }

    // Where should I save the vertex attribute index, according to the material
    const int index_offset = submeshes[material_index]->index_offset +
                             submeshes[material_index]->triangle_count * 3;

    for(int vertice_index = 0; vertice_index < TRIANGLE_VERTEX_COUNT; ++vertice_index) {
      const int control_point_index = mesh->GetPolygonVertex(polygon_index, vertice_index);

      if(all_by_control_points) {
        indices[index_offset + vertice_index] = static_cast<unsigned int>(control_point_index);
      }
      // Populate the array with vertex attribute, if by polygon vertex.
      else {
        indices[index_offset + vertice_index] = static_cast<GLuint>(vertex_count);

        current_vertex = control_points[control_point_index];
        vertices[vertex_count * VERTEX_STRIDE] = static_cast<float>(current_vertex[0]);
        vertices[vertex_count * VERTEX_STRIDE + 1] = static_cast<float>(current_vertex[1]);
        vertices[vertex_count * VERTEX_STRIDE + 2] = static_cast<float>(current_vertex[2]);
        vertices[vertex_count * VERTEX_STRIDE + 3] = 1;

        if(has_normal) {
          mesh->GetPolygonVertexNormal(polygon_index, vertice_index, current_normal);
          normals[vertex_count * NORMAL_STRIDE] = static_cast<float>(current_normal[0]);
          normals[vertex_count * NORMAL_STRIDE + 1] = static_cast<float>(current_normal[1]);
          normals[vertex_count * NORMAL_STRIDE + 2] = static_cast<float>(current_normal[2]);
        }

        if(has_uv) {
          mesh->GetPolygonVertexUV(polygon_index, vertice_index, uv_name, current_uv);
          uvs[vertex_count * UV_STRIDE] = static_cast<float>(current_uv[0]);
          uvs[vertex_count * UV_STRIDE + 1] = static_cast<float>(current_uv[1]);
        }
      }

      ++vertex_count;
    }

    submeshes[material_index]->triangle_count += 1;
  }

  return true;
}

void VBOMesh::update_vertex_position(FbxMesh * mesh, const FbxVector4 * deformed_vertices)
{
  int vertex_count = 0;

  if(all_by_control_points) {
    vertex_count = mesh->GetControlPointsCount();
    vertices = std::vector<float>(vertex_count * VERTEX_STRIDE);

    for(int i = 0; i < vertex_count; ++i) {
      vertices[i * VERTEX_STRIDE] = static_cast<float>(deformed_vertices[i][0]);
      vertices[i * VERTEX_STRIDE + 1] = static_cast<float>(deformed_vertices[i][1]);
      vertices[i * VERTEX_STRIDE + 2] = static_cast<float>(deformed_vertices[i][2]);
      vertices[i * VERTEX_STRIDE + 3] = 1;
    }
  } else {
    const int polygon_count = mesh->GetPolygonCount();
    vertex_count = polygon_count * TRIANGLE_VERTEX_COUNT;
    vertices = std::vector<float>(vertex_count * VERTEX_STRIDE);

    int vertex_count = 0;

    for(int i = 0; i < polygon_count; ++i) {
      for(int j = 0; j < TRIANGLE_VERTEX_COUNT; ++j) {
        const int control_point_index = mesh->GetPolygonVertex(i, j);
        vertices[vertex_count * VERTEX_STRIDE] = static_cast<float>(deformed_vertices[control_point_index][0]);
        vertices[vertex_count * VERTEX_STRIDE + 1] = static_cast<float>(deformed_vertices[control_point_index][1]);
        vertices[vertex_count * VERTEX_STRIDE + 2] = static_cast<float>(deformed_vertices[control_point_index][2]);
        vertices[vertex_count * VERTEX_STRIDE + 3] = 1;

        ++vertex_count;
      }
    }
  }
}

} // namespace Fbx2Json
