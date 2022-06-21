#pragma once

/****************************************************************************
Copyright (c) 2010-2013 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <map>
//#include "CCPlatformMacros.h"

#define CC_USE_ASSET_MANAGER 1

#if CC_USE_ASSET_MANAGER
#include "android/asset_manager.h"
#include "android/asset_manager_jni.h"
#endif // CC_USE_ASSET_MANAGER

#define CC_DLL

class CC_DLL kkFileUtils
{
public:

    static kkFileUtils* sharedFileUtils();

    static void purgeFileUtils();

    virtual ~kkFileUtils();

    virtual std::string getFileData(std::string filename, const char *mode);

protected:
    std::string m_strDefaultResRootPath;
    static kkFileUtils* s_sharedFileUtils;


public:
    static void setAssetManager(AAssetManager *assetManager);
    static AAssetManager *getAssetManager();
private:
    unsigned char *readFileWithAsset(const std::string &fullPath, unsigned long *pSize) const;
    static AAssetManager *s_assetManager;
};
