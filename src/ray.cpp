#include <ray.h>
#include <object.hpp>

// 判断ray和obb是否相交
// 若相交，返回交点坐标和交点法线
IntersectPoint intersect(const Ray &ray, const Obb &obb)
{
    // obb的三个轴
    glm::vec3 translate_global = obb.object->getGModelNoscale() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat3 rotate_global = obb.object->getGModelNoscale();

    // obb轴
    glm::mat3 V = rotate_global * obb.rotate * glm::mat3(1.0f);

    // obb中心
    glm::vec3 C = translate_global + obb.center;
    glm::vec3 d = glm::normalize(ray.d);
    glm::vec3 P = C - ray.o;

    float tmin = -INFINITY;
    float tmax = INFINITY;
    glm::vec3 tmin_normal(1.0f);
    glm::vec3 tmax_normal(1.0f);

    for (int i = 0; i < 3; i++)
    {
        float r = glm::dot(V[i], d);
        float s = glm::dot(V[i], P);
        if (r == 0.0f)
        {
            if (-s - obb.extends[i] > 0 || -s + obb.extends[i] < 0)
            {
                return (IntersectPoint){.inter = false};
            }
        }
        else
        {
            glm::vec3 ttmin_normal = -V[i];
            glm::vec3 ttmax_normal = V[i];
            float ttmin = (s - obb.extends[i]) / r;
            float ttmax = (s + obb.extends[i]) / r;
            if (ttmin > ttmax)
            {
                float t = ttmin;
                ttmin = ttmax;
                ttmax = t;
                glm::vec3 tn = ttmin_normal;
                ttmin_normal = ttmax_normal;
                ttmax_normal = tn;
            }
            if (ttmin > tmin)
            {
                tmin = ttmin;
                tmin_normal = ttmin_normal;
            }
            if (ttmax < tmax)
            {
                tmax = ttmax;
                tmax_normal = ttmax_normal;
            }
            if (tmin > tmax || tmax < 0.0f)
            {
                return (IntersectPoint){.inter = false};
            }
        }
    }

    float t = tmin > 0.0f ? tmin : tmax;
    glm::vec3 n = tmin > 0.0f ? tmin_normal : tmax_normal;
    glm::vec3 Q = ray.o + t * d; // 交点
    return (IntersectPoint){.inter = true, .p = Q, .n = n};
}

IntersectPoint Ray::intersectWith(const Obb &obb)
{
    return intersect(*this, obb);
}

IntersectPoint Ray::intersectWith(const Object &object)
{
    if (object.getObb())
    {
        return intersectWith(*object.getObb());
    }
    else
    {
        std::pair<float, IntersectPoint> neareatPoint = {INFINITY, (IntersectPoint){.inter = false}};
        for (Object *c : object.getChildren())
        {
            IntersectPoint ip = intersectWith(*c);
            if (ip.inter && glm::length(ip.p - o) < neareatPoint.first)
            {
                neareatPoint = {glm::length(ip.p - o), ip};
            }
        }
        return neareatPoint.second;
    }
}