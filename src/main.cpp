//
//  main.cpp
//  fbx2json
//
//  Created by Cameron Yule on 09/02/2013.
//  Copyright (c) 2013 Cameron Yule. All rights reserved.
//

#include <iostream>
#include <string>
using namespace std;

#define FBXSDK_NEW_API true
#include <fbxsdk.h>

#include <JsonBox.h>

#include "SceneContext.h"

void ExitFunction();

SceneContext * gSceneContext;

void usage(std::string prog)
{
    std::cerr << prog << ": missing arguments" << std::endl;
    fprintf(stderr, "\nUSAGE: %s [FBX inputFile] [JSON outputFile] \n", prog.c_str());
}

void createJSON(std::string outputFile, std::vector<VBOMesh *> * pMeshes)
{
    JsonBox::Array meshes;

    for(std::vector<VBOMesh *>::iterator mesh = pMeshes->begin(); mesh != pMeshes->end(); ++mesh)
    {
        VBOMesh* lMesh = *mesh;
        
        JsonBox::Object meshObject;
        
        JsonBox::Array vertices;
        JsonBox::Array normals;
        JsonBox::Array uvs;
        JsonBox::Array indices;
        
        for(std::vector<float>::iterator vertex = lMesh->mVertices->begin(); vertex != lMesh->mVertices->end(); ++vertex)
        {
            vertices.push_back(*vertex);
        }
        
        for(std::vector<float>::iterator normal = lMesh->mNormals->begin(); normal != lMesh->mNormals->end(); ++normal)
        {
            normals.push_back(*normal);
        }
        
        for(std::vector<float>::iterator uv = lMesh->mUVs->begin(); uv != lMesh->mUVs->end(); ++uv)
        {
            uvs.push_back(*uv);
        }
        
        for(std::vector<GLuint>::iterator index = lMesh->mIndices->begin(); index != lMesh->mIndices->end(); ++index)
        {
            indices.push_back(int(*index));
        }
        
        meshObject["vertices"] = vertices;
        meshObject["normals"] = normals;
        meshObject["uvs"] = uvs;
        meshObject["indices"] = indices;
        
        meshes.push_back(meshObject);
    }
    
    JsonBox::Value v(meshes);
    v.writeToFile(outputFile);
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    // Set exit function to destroy objects created by the FBX SDK.
    atexit(ExitFunction);

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    
    gSceneContext = new SceneContext(inputFile.c_str());
    
    createJSON(outputFile, gSceneContext->mMeshes);

    return EXIT_SUCCESS;
}

// Function to destroy objects created by the FBX SDK.
void ExitFunction()
{
    delete gSceneContext;
}