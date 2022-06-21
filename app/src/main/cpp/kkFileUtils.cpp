
#include "kkFileUtils.h"

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

#include "kkFileUtils.h"
#include <stack>
#include <algorithm>

#include "kk_log.h"


using namespace std;

#if CC_USE_ASSET_MANAGER
AAssetManager *kkFileUtils::s_assetManager = NULL;
#endif // #if CC_USE_ASSET_MANAGER

kkFileUtils* kkFileUtils::s_sharedFileUtils = NULL;


kkFileUtils* kkFileUtils::sharedFileUtils()
{
    if (s_sharedFileUtils == NULL)
    {
        s_sharedFileUtils = new kkFileUtils();
    }
    return s_sharedFileUtils;
}

void kkFileUtils::purgeFileUtils()
{

}

kkFileUtils::~kkFileUtils()
{

}

std::string kkFileUtils::getFileData(std::string filename, const char *mode)
{
    unsigned long size = 0;
    unsigned char *data = 0;

    data = readFileWithAsset(filename, &size);

    std::string ret(reinterpret_cast<const char *>(data), size);
    return ret;
}


void kkFileUtils::setAssetManager(AAssetManager *assetManager)
{
    s_assetManager = assetManager;
}
AAssetManager *kkFileUtils::getAssetManager()
{
    return s_assetManager;
}

unsigned char *kkFileUtils::readFileWithAsset(const std::string &fullPath, unsigned long *pSize) const
{
    static const std::string apkprefix("assets/");
    if (s_assetManager == NULL)
    {
        CCLOG("%s CCFileUtilsAndroid::s_assetManager is NULL", __FUNCTION__ );
        *pSize = 0;
        return NULL;
    }

    string relativePath = string();
    size_t position = fullPath.find(apkprefix);
    if (0 == position)
    {
        // "assets/" is at the beginning of the path and we don't want it
        relativePath += fullPath.substr(apkprefix.size());
    }
    else
    {
        relativePath = fullPath;
    }

    AAsset* asset = AAssetManager_open(s_assetManager, relativePath.data(), AASSET_MODE_UNKNOWN);
    if (NULL == asset)
    {
        *pSize = 0;
        return NULL;
    }

    auto size = AAsset_getLength(asset);
    unsigned char *buf = new unsigned char[size];

    int readsize = AAsset_read(asset, buf, size);
    AAsset_close(asset);

    if (readsize < size)
    {
        if (readsize >= 0)
        {
            buf = (unsigned char *) realloc(buf, readsize);
        }
    }
    *pSize = readsize;

    return buf;
}
