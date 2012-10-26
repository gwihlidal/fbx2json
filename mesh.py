""""

  Copyright (c) 2012 Cameron Yule

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

"""

import math
import json

try:
    from FbxCommon import *
except ImportError:
    print("Unable to load the Autodesk Python FBX SDK. Please consult "
          "the online documentation to ensure it is correctly installed: "
          "http://www.autodesk.com/fbx-sdkdoc-2013-enu")
    sys.exit(1)

sdk_manager = None


def process(scene_file, output_file):
    global sdk_manager

    sdk_manager, scene = InitializeSdkObjects()
    LoadScene(sdk_manager, scene, scene_file)

    mesh_data = walk_scene_graph(scene)

    if len(mesh_data) > 0:
        f = open(output_file, 'w')
        f.write(json.dumps(mesh_data[0], indent=2))
    else:
        print "Error: No mesh nodes found in the scene graph. Aborting."

    sdk_manager.Destroy()


def walk_scene_graph(scene):
    node = scene.GetRootNode()
    mesh_data = []

    if node:
        for i in range(node.GetChildCount()):
            mesh_data = parse_scene_node(node.GetChild(i), mesh_data)

    for i in range(len(mesh_data)):
        for j in mesh_data[i].keys():
            mesh_data[i][j] = flatten(mesh_data[i][j])

    return mesh_data


def parse_scene_node(node, mesh_data):
    attribute_type = (node.GetNodeAttribute().GetAttributeType())

    if attribute_type == FbxNodeAttribute.eMesh:
        mesh_data.append(parse_mesh(node))

    for i in range(node.GetChildCount()):
        parse_scene_node(node.GetChild(i), mesh_data)

    return mesh_data


def parse_mesh(node):
    mesh = node.GetNodeAttribute()
    mesh = triangulate_mesh(mesh)

    mesh_data = {
        'vertices': [],
        'faces': [],
        'normals': [],
        'uvs': []
    }

    polygon_count = mesh.GetPolygonCount()
    control_points = mesh.GetControlPoints()

    for i in range(polygon_count):
        polygon_size = mesh.GetPolygonSize(i)

        for j in range(polygon_size):
            vertex = get_vertex(mesh, control_points, i, j)
            mesh_data['vertices'].append(vertex)

            normal = get_normal(mesh, i, j)
            mesh_data['normals'].append(normal)

            uv = get_uv(mesh, i, j)
            mesh_data['uvs'].append(uv)

    mesh_data['faces'] = range(len(mesh_data['vertices']))

    return mesh_data


def triangulate_mesh(mesh):
    if mesh.IsTriangleMesh() is False:
        geometry_converter = FbxGeometryConverter(sdk_manager)
        mesh = geometry_converter.TriangulateMesh(mesh)

    return mesh


def flatten(list):
    try:
        return [x for sublist in list
                for x in sublist]
    except TypeError:
        return list


def get_vertex(mesh, control_points, polygon_index, vertex_index):
    i = mesh.GetPolygonVertex(polygon_index, vertex_index)
    p = control_points[i]
    return [p[0], p[1], p[2]]


def get_normal(mesh, polygon_index, vertex_index):
    n = FbxVector4()
    mesh.GetPolygonVertexNormal(polygon_index, vertex_index, n)
    return [n[0], n[1], n[2]]


def get_uv(mesh, polygon_index, vertex_index):
    # TODO: Support multiple UV layers
    layer_count = min(mesh.GetLayerCount(), 1)

    for i in range(layer_count):
        uvs = mesh.GetLayer(i).GetUVs()
        if uvs:
            if uvs.GetMappingMode() == FbxLayerElement.eByPolygonVertex:
                j = mesh.GetTextureUVIndex(polygon_index, vertex_index)

                if (uvs.GetReferenceMode() == FbxLayerElement.eDirect or
                        uvs.GetReferenceMode() ==
                        FbxLayerElement.eIndexToDirect):
                        return uvs.GetDirectArray().GetAt(j)
