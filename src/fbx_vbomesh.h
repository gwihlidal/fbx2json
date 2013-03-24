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

#ifndef FBX2JSON_FBXVBOMESH_H_
#define FBX2JSON_FBXVBOMESH_H_

#include <vector>
#include <fbxsdk.h>
#include <glew.h>

namespace Fbx2Json
{
class VBOMesh
{
  public:
    VBOMesh();
    ~VBOMesh();
    bool initialize(const FbxMesh * mesh);
    void update_vertex_position(FbxMesh * mesh, const FbxVector4 * vertices);
    int get_submesh_count() const {
      return submeshes.GetCount();
    }

  private:
    std::vector<float> * vertices;
    std::vector<float> * normals;
    std::vector<float> * uvs;
    std::vector<GLuint> * indices;

    enum {
      VERTEX_VBO,
      NORMAL_VBO,
      UV_VBO,
      INDEX_VBO,
      VBO_COUNT,
    };

    struct SubMesh {
      SubMesh() : index_offset(0), triangle_count(0) {}
      int index_offset;
      int triangle_count;
    };

    GLuint vbo_names[VBO_COUNT];
    FbxArray<SubMesh*> submeshes;
    bool has_normal;
    bool has_uv;
    bool all_by_control_points;
};
} // namespace Fbx2Json

#endif
