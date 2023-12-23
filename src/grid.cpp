#include <grid.h>

#include <thread>
#include <algorithm>

Grid::Grid() {
    min_extent = glm::vec3(INFINITY);
    max_extent = glm::vec3(-INFINITY);
    this->extent = 0;
}

void Grid::init(const std::vector<Object*> objs, float extent) {
    min_extent = glm::vec3(INFINITY);
    max_extent = glm::vec3(-INFINITY);
    this->extent = extent;
    buildGrid(objs);
}

// 要求obj的obb尺寸比网格小
const Object* Grid::intersectWith(const Object& obj) {
    if(obj.getObb()) {
        return intersectWith(*(obj.getObb()));
    } else {
        for(Object* c : obj.getChildren()) {
            const Object* ret = intersectWith(*c);
            if(ret)
                return ret;
        }
    }
    return nullptr;
}

const Object* Grid::intersectWith(Obb& obb) {
    assert(2.0f * obb.extends.x < extent);
    assert(2.0f * obb.extends.y < extent);
    assert(2.0f * obb.extends.z < extent);
    for(Object* o : getNeighbours(obb)) {
        int res = obb.intersectWith(*(o->getObb()));
        if(res)
            return o;
    }
    return nullptr;
}

void printVec3(glm::vec3 v) {
    std::cout << v.x << " " << v.y << " " << v.z << std::endl;
}

void Grid::buildGrid(const std::vector<Object*> objs) {
    // 找到objs中所有包围盒x, y, z方向上的最值
    for(Object* obj : objs) {
        assert(obj->getObb());
        Obb* obb = obj->getObb();
        for(glm::vec3 v : obb->getBoxPoint()) {
            min_extent = glm::min(min_extent, v);
            max_extent = glm::max(max_extent, v);
        }
    }
    int cnt = 0;
    // 用每个网格与每个obb求教，如果obb与网格相交，则将对应的object放入该网格
    for(float x = min_extent.x; x < max_extent.x; x += extent) {
        std::vector<std::vector<std::vector<Object*>>> xgrid;
        for(float y = min_extent.y; y < max_extent.y; y += extent) {
            std::vector<std::vector<Object*>> ygrid;
            for(float z = min_extent.z; z < max_extent.z; z += extent) {
                std::vector<Object*> zgrid;
                glm::vec3 min_point(x, y, z);
                glm::vec3 center = min_point + extent / 2.0f;
//                Obb* obb = new Obb(center, glm::vec3(extent / 2.0f));
                Obb obb(center, glm::vec3(extent / 2.0f));
                obb.init();
                for(Object* obj : objs) {
//                    for(int i = 0; i < 10000; i++) {}
//                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    if(obb.intersectWith(*(obj->getObb()))) {
                        cnt++;
                        zgrid.push_back(obj);
                    }
                }
                ygrid.push_back(zgrid);
            }
            xgrid.push_back(ygrid);
        }
        grid.push_back(xgrid);
    }
    std::cout << cnt << std::endl;

//    for(int i = 0; i < grid.size(); i++) {
//        for(int j = 0; j < grid[i].size(); j++) {
//            for(int k = 0; k < grid[i][j].size(); k++) {
//                std::cout << "grid[i][j][k].size(): " << grid[i][j][k].size() << std::endl;
//            }
//        }
//    }
}

std::vector<Object*> Grid::getNeighbours(const Obb& obb) {
    // 获取obb八个顶点所处的grid中的所有Object
    std::vector<Object*> neighbours;
    GridIndex idx;
    for(glm::vec3 v : obb.getBoxPoint()) {
        if(findGrid(v, idx)) {
            neighbours.insert(neighbours.end(), grid[idx.i][idx.j][idx.k].begin(), grid[idx.i][idx.j][idx.k].end());
        }
    }
    std::sort(neighbours.begin(), neighbours.end());
    neighbours.erase(std::unique(neighbours.begin(), neighbours.end()), neighbours.end());
    return neighbours;
}

bool Grid::findGrid(glm::vec3 v, GridIndex& out) {
    if(v.x < min_extent.x || v.x > max_extent.x)
        return false;
    if(v.y < min_extent.y || v.y > max_extent.y)
        return false;
    if(v.z < min_extent.z || v.z > max_extent.z)
        return false;

    out.i = (v.x - min_extent.x) / extent;
    out.j = (v.y - min_extent.y) / extent;
    out.k = (v.z - min_extent.z) / extent;
    
    return true;
}