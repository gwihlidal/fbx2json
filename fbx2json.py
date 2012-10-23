import sys
import math
import json

def iterate_scene_nodes(scene, sdk_manager):
    node = scene.GetRootNode()

    if node:
        for i in range(node.GetChildCount()):
            parse_node_content(node.GetChild(i), sdk_manager)


def parse_node_content(node, sdk_manager):
    attribute_type = (node.GetNodeAttribute().GetAttributeType())

    if attribute_type == FbxNodeAttribute.eMesh:
        data = extract_mesh_data(node, sdk_manager)
        print json.dumps(data, indent=1)

    for i in range(node.GetChildCount()):
        parse_node_content(node.GetChild(i), sdk_manager)


def extract_mesh_data(node, sdk_manager):
    mesh = node.GetNodeAttribute()
    mesh = convert_mesh_to_triangles(mesh, sdk_manager)

    vertices    = get_vertices(mesh)
    indices     = get_indices(mesh)
    # normals     = get_normals(mesh)
    uvs         = get_uvs(mesh)

    print "Vertices: %s" % len(vertices)
    print "Indices: %s" % len(indices)
    # print "Normals: %s" % len(normals)
    print "UVs: %s" % len(uvs)

    return { 'vertices' : vertices, 'indices' : indices, 'uvs' : uvs }
    # return { 'vertices' : vertices, 'normals' : normals, 'indices' : indices, 'uvs' : uvs }


def convert_mesh_to_triangles(mesh, sdk_manager):
    if mesh.IsTriangleMesh() is False:
        geometry_converter = FbxGeometryConverter(sdk_manager)
        mesh = geometry_converter.TriangulateMesh(mesh)

    return mesh


def get_vertices(mesh):
    control_points_count = mesh.GetControlPointsCount()
    control_points = mesh.GetControlPoints()

    points_flat = []

    for i in range(len(control_points)):
        points_flat.append(control_points[i][0])
        points_flat.append(control_points[i][1])
        points_flat.append(control_points[i][2])

    return points_flat


def get_indices(mesh):
    polygon_count = mesh.GetPolygonCount()
    indices = []

    for i in range(polygon_count):
        polygon_size = mesh.GetPolygonSize(i)

        for j in range(polygon_size):
            control_point_index = mesh.GetPolygonVertex(i, j)
            indices.append(control_point_index)

    return indices


# def get_normals(mesh):
#     polygon_count = mesh.GetPolygonCount()
#     normals = []
# 
#     for i in range(polygon_count):
#         polygon_size = mesh.GetPolygonSize(i)
# 
#         for j in range(polygon_size):
#             normal = FbxVector4()
#             mesh.GetPolygonVertexNormal(i, j, normal)
#             normals.append(normal)
#             
#     normals_flat = []
#     for i in range(len(normals)):
#         normals_flat.append(normals[i][0])
#         normals_flat.append(normals[i][1])
#         normals_flat.append(normals[i][2])
# 
#     return normals_flat


# TODO: This seems broken. http://docs.autodesk.com/FBX/2013/ENU/FBX-SDK-Documentation/cpp_ref/_u_v_sample_2main_8cxx-example.html
def get_uvs(mesh):
    polygon_count = mesh.GetPolygonCount()
    uvs = []
    
    for i in range(polygon_count):

        polygon_size = mesh.GetPolygonSize(i)

        for j in range(polygon_size):
            control_point_index = mesh.GetPolygonVertex(i, j)
            layer_count = mesh.GetLayerCount()

            layer_count = min(layer_count, 1)
            
            for l in range(layer_count):
                layer_uvs = mesh.GetLayer(l).GetUVs()
                if layer_uvs:
                    if layer_uvs.GetMappingMode() ==  FbxLayerElement.eByPolygonVertex:
                        layer_texture_uv_index = mesh.GetTextureUVIndex(i, j)
                        if layer_uvs.GetReferenceMode() == FbxLayerElement.eDirect or \
                           layer_uvs.GetReferenceMode() == FbxLayerElement.eIndexToDirect:
                            uvs.append(layer_uvs.GetDirectArray().GetAt(layer_texture_uv_index))

    uvs_flat = []
    for i in range(len(uvs)):
        uvs_flat.append(uvs[i][0])
        uvs_flat.append(uvs[i][1])
        
    return uvs_flat


if __name__ == "__main__":
    try:
        from FbxCommon import *
    except ImportError:
        sys.exit(1)

    # Initialise FBX SDK
    sdk_manager, scene = InitializeSdkObjects()
    
    # Load Scene from FBX file
    result = LoadScene(sdk_manager, scene, sys.argv[1])
    
    iterate_scene_nodes(scene, sdk_manager)

    # Shutdown FBX SDK
    sdk_manager.Destroy()
    sys.exit(0)