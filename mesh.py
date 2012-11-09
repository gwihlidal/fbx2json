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
import pdb

try:
    from FbxCommon import *
except ImportError:
    print("Unable to load the Autodesk Python FBX SDK. Please consult "
          "the online documentation to ensure it is correctly installed: "
          "http://www.autodesk.com/fbx-sdkdoc-2013-enu")
    sys.exit(1)

TRIANGLE_VERTEX_COUNT = 3
VERTEX_STRIDE = 3

sdk_manager = None
scene = None


def process(scene_file, output_file):
    global sdk_manager
    global scene

    sdk_manager, scene = InitializeSdkObjects()
    LoadScene(sdk_manager, scene, scene_file)

    root_node = scene.GetRootNode()
    triangulate_mesh(root_node)

    output_data = walk_scene_graph(root_node)

    if len(output_data) > 0:
        write_file(output_data, output_file)
    else:
        print "Error: No mesh nodes found in the scene graph."

    sdk_manager.Destroy()


def triangulate_mesh(node):
    node_attribute = node.GetNodeAttribute()

    if node_attribute:
        node_attribute_type = node_attribute.GetAttributeType()

        if (node_attribute_type == FbxNodeAttribute.eMesh or
            node_attribute_type == FbxNodeAttribute.eNurbs or
            node_attribute_type == FbxNodeAttribute.eNurbsSurface or
                node_attribute_type == FbxNodeAttribute.ePatch):
                geometry_converter = FbxGeometryConverter(sdk_manager)
                geometry_converter.TriangulateInPlace(node)

    for i in range(node.GetChildCount()):
        triangulate_mesh(node.GetChild(i))


def walk_scene_graph(root_node):
    scene_graph = []

    if root_node:
        for i in range(root_node.GetChildCount()):
            scene_graph = parse_node(root_node.GetChild(i), scene_graph)

    return scene_graph


def parse_node(node, scene_graph):
    node_attribute = node.GetNodeAttribute()
    node_attribute_type = node_attribute.GetAttributeType()

    if node_attribute_type == FbxNodeAttribute.eMesh:
        mesh_data = parse_mesh(node, node_attribute)
        scene_graph.append(mesh_data)

    for i in range(node.GetChildCount()):
        parse_node(node.GetChild(i), scene_graph)

    return scene_graph


# TODO: Replace list with VO
def parse_mesh(node, mesh):
    polygon_count = mesh.GetPolygonCount()
    control_points = mesh.GetControlPoints()

    control_points = bake_global_positions(node, control_points)

    if mesh_only_uses_control_points(mesh):
        mesh_data = parse_mesh_by_control_point(mesh, polygon_count,
                                                control_points)
    else:
        mesh_data = parse_mesh_by_other_mapping_mode(mesh, polygon_count,
                                                     control_points)

    material = parse_material(node)

    return {'vertices': mesh_data[0],
            'indices': mesh_data[1],
            'normals': mesh_data[2],
            'uvs': mesh_data[3],
            'material': material}


def bake_global_positions(node, control_points):
    time = FbxTime()
    pose = None

    if (scene.GetPoseCount() > 0):
        pose = scene.GetPose(0)

    global_position = get_global_position(node, time, pose)

    geometry_offset = get_geometry(node)
    global_offset_position = global_position * geometry_offset

    for i in range(len(control_points)):
        control_points[i] = global_offset_position.MultT(control_points[i])

    return control_points


def get_global_position(node, time, pose):
    global_position = FbxAMatrix()
    position_found = False

    if (pose):
        node_index = pose.Find(node)

        if (node_index > -1):
            # The bind pose is always a global matrix
            if (pose.IsBindPose() or not pose.IsLocalMatrix(node_index)):
                global_position = get_pose_matrix(pose, node_index)
            else:
                parent_global_position = FbxAMatrix()

                if node.GetParent():
                    parent_global_position = get_global_position(
                        node.GetParent(), time, pose)

                local_position = get_pose_matrix(pose, node_index)
                global_position = global_position * local_position

            position_found = True

    if not position_found:
        global_position = node.EvaluateGlobalTransform(time)

    return global_position


def get_geometry(node):
    translation = node.GetGeometricTranslation(FbxNode.eSourcePivot)
    rotation = node.GetGeometricRotation(FbxNode.eSourcePivot)
    scaling = node.GetGeometricScaling(FbxNode.eSourcePivot)

    geometry_transform = FbxAMatrix()
    geometry_transform.SetTRS(translation, rotation, scaling)

    return geometry_transform


def get_pose_matrix(pose, node_index):
    pose_matrix = pose.GetMatrix(node_index)

    translation = FbxVector4()
    rotation = FbxQuaternion()
    shearing = FbxVector4()
    scaling = FbxVector4()
    rotation_euler = FbxVector4()

    pose_matrix.GetElements(translation, rotation, shearing, scaling)
    rotation.ComposeSphericalXYZ(rotation_euler)

    affine_pose_matrix = FbxAMatrix()
    affine_pose_matrix.SetTRS(translation, rotation_euler, scaling)

    return affine_pose_matrix


def mesh_only_uses_control_points(mesh):
    '''If normals or UVs are defined by polygon vertex, duplicate vertex data
    is needed. Otherwise we can use only the control points.'''

    if mesh.GetLayerCount() > 0:
        layer = mesh.GetLayer(0)

        normals = layer.GetNormals()

        if normals:
            normal_mapping_mode = normals.GetMappingMode()
            if normal_mapping_mode != FbxLayerElement.eByControlPoint:
                return False

        uvs = layer.GetUVs()

        if uvs:
            uv_mapping_mode = uvs.GetMappingMode()
            if uv_mapping_mode != FbxLayerElement.eByControlPoint:
                return False

    return True


def parse_mesh_by_control_point(mesh, polygon_count, control_points):
    vertex_count = mesh.GetControlPointsCount()

    vertices = []
    indices = range(vertex_count)
    normals = []
    uvs = []

    normal_element = None
    uv_element = None

    if mesh.GetLayerCount() > 0:
        layer = mesh.GetLayer(0)
        normal_element = layer.GetNormals()
        uv_element = layer.GetUVs()

    for i in range(vertex_count):
        vertices.append(control_points[i][0])
        vertices.append(control_points[i][1])
        vertices.append(control_points[i][2])

        if normal_element:
            normal_index = i
            normal_reference_mode = normal_element.GetReferenceMode()

            if normal_reference_mode == FbxLayerElement.eIndexToDirect:
                normal_index = normal_element.GetIndexArray().GetAt(i)

            normal = normal_element.GetDirectArray().GetAt(normal_index)
            normals.append(normal[0])
            normals.append(normal[1])
            normals.append(normal[2])

        if uv_element:
            uv_index = i
            uv_reference_mode = uv_element.GetReferenceMode()

            if uv_reference_mode == FbxLayerElement.eIndexToDirect:
                uv_index = uv_element.GetIndexArray().GetAt(i)

            uv = uv_element.GetDirectArray().GetAt(uv_index)
            uvs.append(uv[0])
            uvs.append(uv[1])

    return (vertices, indices, normals, uvs)


def parse_mesh_by_other_mapping_mode(mesh, polygon_count, control_points):
    vertex_count = polygon_count * TRIANGLE_VERTEX_COUNT * VERTEX_STRIDE

    vertices = []
    indices = range(vertex_count / VERTEX_STRIDE)
    normals = []
    uvs = []

    normal_element = None
    uv_element = None

    if mesh.GetLayerCount() > 0:
        layer = mesh.GetLayer(0)
        normal_element = layer.GetNormals()
        uv_element = layer.GetUVs()
        if uv_element:
            uv_name = uv_element.GetName()

    for i in range(polygon_count):
        for j in range(TRIANGLE_VERTEX_COUNT):
            index = mesh.GetPolygonVertex(i, j)

            vertex = control_points[index]
            vertices.append(vertex[0])
            vertices.append(vertex[1])
            vertices.append(vertex[2])

            if normal_element:
                normal = FbxVector4()
                mesh.GetPolygonVertexNormal(i, j, normal)
                normals.append(normal[0])
                normals.append(normal[1])
                normals.append(normal[2])

            if uv_element:
                uv = FbxVector2()
                mesh.GetPolygonVertexUV(i, j, uv_name, uv)
                uvs.append(uv[0])
                uvs.append(uv[1])

    return (vertices, indices, normals, uvs)


# TODO: Refactor
# TODO: Replace list with VO
def parse_material(node):
    data = None

    if node.GetMaterialCount() > 0:
        data = {}

        material = node.GetMaterial(0)

        transparency = material.TransparencyFactor.Get()

        data['ambient'] = flatten_double(material.Ambient.Get())
        data['ambient'].append(transparency)

        data['diffuse'] = flatten_double(material.Diffuse.Get())
        data['diffuse'].append(transparency)

        data['emissive'] = flatten_double(material.Emissive.Get())
        data['emissive'].append(transparency)

        if material.GetClassId().Is(FbxSurfacePhong.ClassId):
            data['specular'] = flatten_double(material.Specular.Get())
            data['specular'].append(transparency)

            data['shininess'] = flatten_double(material.Shininess.Get())

            data['reflection'] = flatten_double(material.Reflection.Get())
            data['reflection'].append(transparency)

        prop = material.FindProperty(FbxLayerElement.sTextureChannelNames(0))

        if prop.GetSrcObjectCount(FbxTexture.ClassId) > 0:
            texture = prop.GetSrcObject(FbxTexture.ClassId, 0)
            data['texture'] = os.path.basename(texture.GetFileName())

    return data


def flatten_double(double):
    return [double[0], double[1], double[2]]


def write_file(mesh_data, output_file):
    f = open(output_file, 'w')
    f.write(json.dumps(mesh_data, indent=2))
