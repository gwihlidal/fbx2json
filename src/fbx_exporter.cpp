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

#include "fbx_exporter.h"

namespace Fbx2Json
{

Exporter::Exporter()
{

}

void Exporter::write(const std::string output, std::vector<VBOMesh *> * meshes)
{
    JsonBox::Array output_meshes;
    
    for(std::vector<VBOMesh *>::iterator m = meshes->begin(); m != meshes->end(); ++m)
    {
        VBOMesh* mesh = *m;
        
        JsonBox::Object container;
        
        JsonBox::Array vertices;
        JsonBox::Array normals;
        JsonBox::Array uvs;
        JsonBox::Array indices;
        
        for(std::vector<float>::iterator vertex = mesh->vertices->begin(); vertex != mesh->vertices->end(); ++vertex)
        {
            vertices.push_back(*vertex);
        }
        
        for(std::vector<float>::iterator normal = mesh->normals->begin(); normal != mesh->normals->end(); ++normal)
        {
            normals.push_back(*normal);
        }
        
        for(std::vector<float>::iterator uv = mesh->uvs->begin(); uv != mesh->uvs->end(); ++uv)
        {
            uvs.push_back(*uv);
        }
        
        for(std::vector<GLuint>::iterator index = mesh->indices->begin(); index != mesh->indices->end(); ++index)
        {
            indices.push_back(int(*index));
        }
        
        container["vertices"] = vertices;
        container["normals"] = normals;
        container["uvs"] = uvs;
        container["indices"] = indices;
        
        output_meshes.push_back(container);
    }
    
    JsonBox::Value v(output_meshes);
    v.writeToFile(output);
}

Exporter::~Exporter()
{

}

} // namespace Fbx2Json
