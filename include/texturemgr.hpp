#ifndef _TEXTUREMGR_HPP_
#define _TEXTUREMGR_HPP_

#include <map>
#include <string>

enum TexType {
    TEX_NONE = 0,
    TEX_2D,
    TEX_CUBEMAP,
};

class TextureMgr {
public:
    static TextureMgr* getInstance();
    
    bool load(const std::string& file, TexType type);
    bool load_2D(const std::string& file);
    bool load_CubeMap(const std::string& path);
    int gettex(const std::string& tex, TexType type) {
        if(texs.find(tex) == texs.end()) {
            if(!load(tex, type)) {
                return -1;
            }
        }
        return texs[tex];
    }

private:
    TextureMgr();
    std::map<std::string, unsigned int> texs;
};

#endif