#include <bvh.h>

#include <cassert>
#include <algorithm>

BVHAccel::BVHAccel(std::vector<Obb*> obbs) {
    if(obbs.empty())
        return;

    root = recursiveBuild(obbs);
}

BVHNode* BVHAccel::recursiveBuild(std::vector<Obb*> obbs) {
    BVHNode* node = new BVHNode();

    if(obbs.size() == 1) {
        // 不额外构造Obb，直接创建叶结点
        node->obb = obbs[0];
        node->left = node->right = nullptr;
        return node;
    } else if(obbs.size() == 2) {
        // 拆分成左右两个子节点
        node->obb = Obb::Union(obbs);
        node->left = recursiveBuild(std::vector{ obbs[0] });
        node->right = recursiveBuild(std::vector{ obbs[1] });
        return node;
    } else {
        // 创建一个能包含obb中所有包围盒中心的包围盒
        std::vector<glm::vec3> centers;
        for(Obb* obb : obbs) {
            centers.emplace_back(obb->center);
        }
        Obb* bounds = Obb::obbgen(centers);
        // 沿着bounds最长的那个维度将Obb分裂
        int dim = bounds->maxExtent();
        switch(dim) {
            case 0:
                std::sort(obbs.begin(), obbs.end(), [](Obb* f1, Obb* f2) {
                    return f1->center.x < f2->center.x;
                });
                break;
            case 1:
                std::sort(obbs.begin(), obbs.end(), [](Obb* f1, Obb* f2) {
                    return f1->center.y < f2->center.y;
                });
                break;
            case 2:
                std::sort(obbs.begin(), obbs.end(), [](Obb* f1, Obb* f2) {
                    return f1->center.z < f2->center.z;
                });
                break;
        }
        // 按照中位数分成左右两棵子树
        auto begin = obbs.begin();
        auto mid = obbs.begin() + (obbs.size() / 2);
        auto end = obbs.end();

        auto leftobbs = std::vector<Obb*>(begin, mid);
        auto rightobbs = std::vector<Obb*>(mid, end);
        assert(leftobbs.size() + rightobbs.size() == obbs.size());

        node->left = recursiveBuild(leftobbs);
        node->right = recursiveBuild(rightobbs);
        node->obb = Obb::Union(std::vector<Obb*>({ node->left->obb, node->right->obb }));
        delete bounds;

        return node;
    }
}

BVHAccel::~BVHAccel() {
    delete root;
}

BVHNode::~BVHNode() {
    if(left && right) {
        delete obb;
        delete left;
        delete right;
    }
}

std::vector<std::pair<Obb*, int>> BVHAccel::Intersect(Obb& obb) const {
    std::vector<std::pair<Obb*, int>> res;
    if(root)
        root->Intersect(obb, res);

    return res;
}

void BVHNode::Intersect(Obb& obb, std::vector<std::pair<Obb*, int>>& res) const {
    int inter = this->obb->intersectWith(obb);
    if(inter == 0)
        return;
    else if(left) {
        assert(right);
        left->Intersect(obb, res);
        right->Intersect(obb, res);
    } else {
        res.push_back(std::pair<Obb*, int>(this->obb, inter));
    }
}

void BVHAccel::draw(Shader* shader) {
    if(root)
        root->draw(shader);
}

void BVHNode::draw(Shader* shader) {
    assert(obb);
    obb->draw(shader);
    if(left) {
        assert(right);
        left->draw(shader);
        right->draw(shader);
    }
}