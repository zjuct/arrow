#include <control.h>
#include <ray.h>

static Control* control = Control::getInstance();

void Camera::calcPosition(glm::vec3 Position)
{
    Ray ray = Ray(Position, -Front);
    IntersectPoint point = ray.intersectWith(control->ground.getModel());
    float length = 2.0f;
    if (point.inter && glm::length(point.p - Position) * 0.9f < length)
    {
        length = glm::length(point.p - Position) * 0.9f;
    }
    this->Position = Position - length * Front;
}