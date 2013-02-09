# fbx2json 

A simple command-line utility to convert [Autodesk FBX](http://www.autodesk.com/developfbx) files to a custom JSON format, suitable for use in OpenGL/WebGL projects.

`fbx2json` is only intended to support a small subset of the functionality of the FBX format. Check the Features and Roadmap sections below for more information.

## Features

* Export multiple objects from a single FBX
* Vertices are baked in global coordinate space
* UV, normal and material data is captured for each mesh
* Meshes are triangulated to simplify rendering
* Avoids duplicating vertices where possible (eg. if UVs/Normals aren't sharing vertices)

## Requirements

* Boost (brew install boost --with-c++11)
* [Autodesk C++ FBX SDK 2013.3](http://usa.autodesk.com/adsk/servlet/pc/item?siteID=123112&id=10775847) ([Installation instructions](http://docs.autodesk.com/FBX/2013/ENU/FBX-SDK-Documentation/files/GUID-063B70C4-CADC-42B5-9DCC-55274A8B0837.htm))

## Usage

```
fbx2json input.fbx output.js
```

## Roadmap

### 1.0

* ~~Parse command line parameters for in/out files~~
* ~~Extract and return normals~~
* ~~Support multiple nodes in the [FBX scene graph](http://docs.autodesk.com/FBX/2013/ENU/FBX-SDK-Documentation/files/GUID-F194000D-5AD4-49C1-86CC-5DAC2CE64E97.htm)~~
* ~~Capture and apply mesh attributes (e.g. scale, rotate, translate, etc)~~
* ~~Determine when repeating vertices is necessary (e.g. UVs/normals that share a vertex)~~
* ~~Error handling, help, instructions, etc~~
* ~~Support multiple layer element reference modes~~
* ~~Capture material/texture data for each mesh~~
* Support rigged character poses

### Future

* Investigate the [FBX animation format](http://docs.autodesk.com/FBX/2013/ENU/FBX-SDK-Documentation/index.html?url=files/GUID-B3311B8D-5390-4C63-AB9F-662AC7D5C6CC.htm,topicNumber=d30e9650)
* Investigate the work being done on WebGL JSON standardisation (e.g. https://github.com/KhronosGroup/collada2json/wiki/WebGLTF / https://www.khronos.org/webgl/public-mailing-list/archives/1211/msg00009.html)
* Automatically convert textures to web-safe formats (e.g. PSD to JPG)
* Improve material support

## FBX References

* [Autodesk FBX Developer Center](http://www.autodesk.com/developfbx)
* [Autodesk FBX SDK Documentation](http://www.autodesk.com/fbx-sdkdoc-2013-enu)