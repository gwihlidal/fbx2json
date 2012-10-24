#!/usr/bin/env python

import sys
import optparse
import mesh

version = '0.1'

def main():
    """Runs program and handles command line options"""
        
    p = optparse.OptionParser(description='Convert Autodesk FBX files to a ' \
    'custom JSON format.',
    prog='fbx2json.py',
    version='%prog ' + version,
    usage='%prog input.fbx output.js')

    options, arguments = p.parse_args()
        
    if len(arguments) == 2:
        # goafsdfds()
        sys.exit(0)
    else:
        p.print_help()
        
if __name__ == "__main__":
    try:
        from FbxCommon import *
    except ImportError:
        print("Unable to load the Autodesk Python FBX SDK. Please consult " \
        "the online documentation to ensure it is correctly installed: " \
        "http://www.autodesk.com/fbx-sdkdoc-2013-enu") 
        sys.exit(1)
        
    main()