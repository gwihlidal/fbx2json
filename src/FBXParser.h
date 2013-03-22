#ifndef FBXParser_H
#define FBXParser_H

#include <fbxsdk.h>

class FBXParser
{
  public:
    FBXParser();
    void Parse(FbxScene* pScene);
    ~FBXParser();

  private:

};

#endif