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

import os
import json
import math  # TODO: Remove this import after adding multiple layer support

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
        for i in range(len(mesh_data)):
            write_file(mesh_data, output_file, i)
    else:
        print "Error: No mesh nodes found in the scene graph."

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

    data = {}

    # if attribute_type == FbxNodeAttribute.eMarker:
    #     pass
    # elif attribute_type == FbxNodeAttribute.eSkeleton:
    #     pass
    if attribute_type == FbxNodeAttribute.eMesh:
        data = parse_mesh(node)
        mesh_data.append(data)
    # elif attribute_type == FbxNodeAttribute.eNull:
    #     pass
    # elif attribute_type == FbxNodeAttribute.eNurbs:
    #     pass
    # elif attribute_type == FbxNodeAttribute.ePatch:
    #     pass
    # elif attribute_type == FbxNodeAttribute.eCamera:
    #     pass
    # elif attribute_type == FbxNodeAttribute.eLight:
    #     pass

    data['name'] = node.GetName()

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
        'uvs': [],
        'translation': [],
        'rotation': [],
        'scale': []
    }
    print node.GetName()
    translation = node.EvaluateLocalTransform().GetT()
    rotation = node.EvaluateLocalTransform().GetR()
    scale = node.EvaluateLocalTransform().GetS()
    print 'translation %s' % translation
    print 'rotation %s' % rotation
    print 'scale %s' % scale

    # translation = node.LclTranslation.Get()
    # mesh_data['translation'].append(translation)
    # 
    # rotation = node.LclRotation.Get()
    # mesh_data['rotation'].append(rotation)
    # 
    # scale = node.LclScaling.Get()
    # mesh_data['scale'].append(scale)

    polygon_count = mesh.GetPolygonCount()
    control_points = mesh.GetControlPoints()

    for i in range(polygon_count):
        polygon_size = mesh.GetPolygonSize(i)

        for j in range(polygon_size):
            vertex = get_vertex(mesh, control_points, i, j)
            
            print 'result %s' % (translation * rotation * scale * vertex)
            
            # vertex = apply_translation(vertex, translation)
            # vertex = apply_rotation(vertex, rotation)
            # vertex = apply_scale(vertex, scale)

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


def get_vertex(mesh, control_points, polygon_index, vertex_index):
    i = mesh.GetPolygonVertex(polygon_index, vertex_index)
    p = control_points[i]
    return p


# def apply_translation(vertex, translation):
#     vertex[0] += translation[0]
#     vertex[1] += translation[1]
#     vertex[2] += translation[2]
# 
#     return vertex
# 
# 
# def apply_rotation(vertex, rotation):
#     return vertex
# 
# 
# def apply_scale(vertex, scale):
#     vertex[0] *= scale[0]
#     vertex[1] *= scale[1]
#     vertex[2] *= scale[2]
# 
#     return vertex


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


def flatten(expanded_list):
    try:
        if isinstance(expanded_list, list):
            return [x for sublist in expanded_list
                    for x in sublist]
        else:
            return expanded_list
    except TypeError:
        return expanded_list


def output_file_name(requested_name, i):
    file_name, file_extension = os.path.splitext(requested_name)
    return "%s_%s%s" % (file_name, i, file_extension)


def write_file(mesh_data, output_file, i):
    f = open(output_file_name(output_file, i), 'w')
    f.write(json.dumps(mesh_data[i], indent=2))
