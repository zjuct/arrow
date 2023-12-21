#ifndef _BVH_H
#define _BVH_H

#include <obb.h>

struct BVHNode;

class BVHAccel {
public:
    BVHAccel(std::vector<Obb*> obbs);
    ~BVHAccel();

    void draw(Shader* shader);

    std::vector<std::pair<Obb*, int>> Intersect(Obb& obb) const;

private:

    BVHNode* recursiveBuild(std::vector<Obb*> obbs);
    BVHNode* root;
};


struct BVHNode {
    Obb* obb;
    BVHNode* left;
    BVHNode* right;
    BVHNode(): obb(nullptr), left(nullptr), right(nullptr) {}
    ~BVHNode();

    void Intersect(Obb& obb, std::vector<std::pair<Obb*, int>>& res) const;
    void draw(Shader* shader);
};

#endif