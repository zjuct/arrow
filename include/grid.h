#ifndef _GRID_H
#define _GRID_H

#include <object.hpp>

class Grid {
public:
    Grid();
    void init(const std::vector<Object*> objs, float extent);

    const Object* intersectWith(const Object& obj);
    const Object* intersectWith(Obb& obb);

    std::vector<Obb*> obbs;

private:
    struct GridIndex {
        int i, j, k;
    };
    bool findGrid(glm::vec3 v, GridIndex& out);
    void buildGrid(const std::vector<Object*> objs);
    std::vector<Object*> getNeighbours(const Obb& obb);

    glm::vec3 min_extent;
    glm::vec3 max_extent;
    float extent;       // 单个grid长度
    std::vector<std::vector<std::vector<std::vector<Object*>>>> grid;
};

#endif