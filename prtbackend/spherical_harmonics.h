#ifndef _SPHERICAL_HARMONICS_H
#define _SPHERICAL_HARMONICS_H

#include<functional>

#include <glm/glm.hpp>

typedef std::function<float(float, float, int&)> SphericalFunction;

constexpr int SHGetIndex(int l, int m) {
    return l * (l + 1) + m;
}

// Get the total number of coefficients for a function represented by
// all spherical harmonic basis of degree <= @order (it is a point of
// confusion that the order of an SH refers to its degree and not the order).
constexpr int GetCoefficientCount(int order) {
  return (order + 1) * (order + 1);
}

// 将球面参数左边转化为直角坐标
glm::vec3 ToVector(float phi, float theta);


// 返回[l, m]阶的SH在(phi, theta)上的值
float EvalSH(int l, int m, float phi, float theta);

float EvalSH(int l, int m, const glm::vec3& dir);

// 始终使用递归的方式求SH的值
float EvalSHSlow(int l, int m, float phi, float theta);
float EvalSHSlow(int l, int m, const glm::vec3& dir);


// 将func投影到order阶的SH上，返回系数
std::vector<float> ProjectFunction(int order, const SphericalFunction& func, int sample_count);

#endif