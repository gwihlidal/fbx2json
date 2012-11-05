# fbx2json 

A simple command-line utility to convert [Autodesk FBX](http://www.autodesk.com/developfbx) files to a custom JSON format, suitable for use in OpenGL/WebGL projects.

`fbx2json` is only intended to support a small subset of the functionality of the FBX format. Check the Features and Roadmap sections below for more information.

## Features

* Exports mesh data (vertices, normals, uvs and faces) for a single object in an FBX scene
* Triangulates mesh data to simplify rendering

## Requirements

* Python 2.6
* [Autodesk Python FBX SDK 2013.3](http://usa.autodesk.com/adsk/servlet/pc/item?siteID=123112&id=10775847) ([Installation instructions](http://docs.autodesk.com/FBX/2013/ENU/FBX-SDK-Documentation/index.html?url=files/GUID-2F3A42FA-4C19-42F2-BC4F-B9EC64EA16AA.htm,topicNumber=d30e11018))

## Usage

```
python fbx2json.py input.fbx output.js
```

## Roadmap

### 1.0

* ~~Parse command line parameters for in/out files~~
* ~~[PEP 8](http://www.python.org/dev/peps/pep-0008/) compliance~~
* ~~Extract and return normals~~
* ~~Support multiple nodes in the [FBX scene graph](http://docs.autodesk.com/FBX/2013/ENU/FBX-SDK-Documentation/files/GUID-F194000D-5AD4-49C1-86CC-5DAC2CE64E97.htm)~~
* Capture and apply mesh attributes (e.g. scale, rotate, translate, etc)
* Capture material attributes (e.g. diffuse, ambient, texture filename, etc)
* Document JSON output format
* ~~Determine when repeating vertices is necessary (e.g. UVs/normals that share a vertex)~~
* ~~Error handling, help, instructions, etc~~

### Future

* Investigate the [FBX animation format](http://docs.autodesk.com/FBX/2013/ENU/FBX-SDK-Documentation/index.html?url=files/GUID-B3311B8D-5390-4C63-AB9F-662AC7D5C6CC.htm,topicNumber=d30e9650)
* Support multiple scene graph object types (e.g. Camera, Lights, etc)
* Potentially support the [Three.js JSON format](https://github.com/mrdoob/three.js/wiki/JSON-Model-format-3.1) / multiple export formats
* Support multiple layer element reference modes
* Support multiple layers

## FBX References

* [Autodesk FBX Developer Center](http://www.autodesk.com/developfbx)
* [Autodesk FBX SDK Documentation](http://www.autodesk.com/fbx-sdkdoc-2013-enu)