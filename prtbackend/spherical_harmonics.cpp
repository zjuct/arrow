#include <spherical_harmonics.h>

#include <iostream>
#include <random>

const int kCacheSize = 14;

const int kHardCodedOrderLimit = 4;

#ifndef NDEBUG
# define CHECK(condition, message) \
  do { \
    if (!(condition)) { \
      std::cerr << "Check failed (" #condition ") in " << __FILE__ \
        << ":" << __LINE__ << ", message: " << message << std::endl; \
      std::exit(EXIT_FAILURE); \
    } \
  } while(false)
#else
# define CHECK(condition, message) do {} while(false)
#endif

// Hardcoded spherical harmonic functions for low orders (l is first number
// and m is second number (sign encoded as preceeding 'p' or 'n')).
//
// As polynomials they are evaluated more efficiently in cartesian coordinates,
// assuming that @d is unit. This is not verified for efficiency.
float HardcodedSH00(const glm::vec3& d) {
  // 0.5 * sqrt(1/pi)
  return 0.282095;
}

float HardcodedSH1n1(const glm::vec3& d) {
  // -sqrt(3/(4pi)) * y
  return -0.488603 * d.y;
}

float HardcodedSH10(const glm::vec3& d) {
  // sqrt(3/(4pi)) * z
  return 0.488603 * d.z;
}

float HardcodedSH1p1(const glm::vec3& d) {
  // -sqrt(3/(4pi)) * x
  return -0.488603 * d.x;
}

float HardcodedSH2n2(const glm::vec3& d) {
  // 0.5 * sqrt(15/pi) * x * y
  return 1.092548 * d.x * d.y;
}

float HardcodedSH2n1(const glm::vec3& d) {
  // -0.5 * sqrt(15/pi) * y * z
  return -1.092548 * d.y * d.z;
}

float HardcodedSH20(const glm::vec3& d) {
  // 0.25 * sqrt(5/pi) * (-x^2-y^2+2z^2)
  return 0.315392 * (-d.x * d.x - d.y * d.y + 2.0 * d.z * d.z);
}

float HardcodedSH2p1(const glm::vec3& d) {
  // -0.5 * sqrt(15/pi) * x * z
  return -1.092548 * d.x * d.z;
}

float HardcodedSH2p2(const glm::vec3& d) {
  // 0.25 * sqrt(15/pi) * (x^2 - y^2)
  return 0.546274 * (d.x * d.x - d.y * d.y);
}

float HardcodedSH3n3(const glm::vec3& d) {
  // -0.25 * sqrt(35/(2pi)) * y * (3x^2 - y^2)
  return -0.590044 * d.y * (3.0 * d.x * d.x - d.y * d.y);
}

float HardcodedSH3n2(const glm::vec3& d) {
  // 0.5 * sqrt(105/pi) * x * y * z
  return 2.890611 * d.x * d.y * d.z;
}

float HardcodedSH3n1(const glm::vec3& d) {
  // -0.25 * sqrt(21/(2pi)) * y * (4z^2-x^2-y^2)
  return -0.457046 * d.y * (4.0 * d.z * d.z - d.x * d.x
                             - d.y * d.y);
}

float HardcodedSH30(const glm::vec3& d) {
  // 0.25 * sqrt(7/pi) * z * (2z^2 - 3x^2 - 3y^2)
  return 0.373176 * d.z * (2.0 * d.z * d.z - 3.0 * d.x * d.x
                             - 3.0 * d.y * d.y);
}

float HardcodedSH3p1(const glm::vec3& d) {
  // -0.25 * sqrt(21/(2pi)) * x * (4z^2-x^2-y^2)
  return -0.457046 * d.x * (4.0 * d.z * d.z - d.x * d.x
                             - d.y * d.y);
}

float HardcodedSH3p2(const glm::vec3& d) {
  // 0.25 * sqrt(105/pi) * z * (x^2 - y^2)
  return 1.445306 * d.z * (d.x * d.x - d.y * d.y);
}

float HardcodedSH3p3(const glm::vec3& d) {
  // -0.25 * sqrt(35/(2pi)) * x * (x^2-3y^2)
  return -0.590044 * d.x * (d.x * d.x - 3.0 * d.y * d.y);
}

float HardcodedSH4n4(const glm::vec3& d) {
  // 0.75 * sqrt(35/pi) * x * y * (x^2-y^2)
  return 2.503343 * d.x * d.y * (d.x * d.x - d.y * d.y);
}

float HardcodedSH4n3(const glm::vec3& d) {
  // -0.75 * sqrt(35/(2pi)) * y * z * (3x^2-y^2)
  return -1.770131 * d.y * d.z * (3.0 * d.x * d.x - d.y * d.y);
}

float HardcodedSH4n2(const glm::vec3& d) {
  // 0.75 * sqrt(5/pi) * x * y * (7z^2-1)
  return 0.946175 * d.x * d.y * (7.0 * d.z * d.z - 1.0);
}

float HardcodedSH4n1(const glm::vec3& d) {
  // -0.75 * sqrt(5/(2pi)) * y * z * (7z^2-3)
  return -0.669047 * d.y * d.z * (7.0 * d.z * d.z - 3.0);
}

float HardcodedSH40(const glm::vec3& d) {
  // 3/16 * sqrt(1/pi) * (35z^4-30z^2+3)
  float z2 = d.z * d.z;
  return 0.105786 * (35.0 * z2 * z2 - 30.0 * z2 + 3.0);
}

float HardcodedSH4p1(const glm::vec3& d) {
  // -0.75 * sqrt(5/(2pi)) * x * z * (7z^2-3)
  return -0.669047 * d.x * d.z * (7.0 * d.z * d.z - 3.0);
}

float HardcodedSH4p2(const glm::vec3& d) {
  // 3/8 * sqrt(5/pi) * (x^2 - y^2) * (7z^2 - 1)
  return 0.473087 * (d.x * d.x - d.y * d.y)
      * (7.0 * d.z * d.z - 1.0);
}

float HardcodedSH4p3(const glm::vec3& d) {
  // -0.75 * sqrt(35/(2pi)) * x * z * (x^2 - 3y^2)
  return -1.770131 * d.x * d.z * (d.x * d.x - 3.0 * d.y * d.y);
}

float HardcodedSH4p4(const glm::vec3& d) {
  // 3/16*sqrt(35/pi) * (x^2 * (x^2 - 3y^2) - y^2 * (3x^2 - y^2))
  float x2 = d.x * d.x;
  float y2 = d.y * d.y;
  return 0.625836 * (x2 * (x2 - 3.0 * y2) - y2 * (3.0 * x2 - y2));
}

// Compute the factorial for an integer @x. It is assumed x is at least 0.
// This implementation precomputes the results for low values of x, in which
// case this is a constant time lookup.
//
// The vast majority of SH evaluations will hit these precomputed values.
float Factorial(int x) {
  static const float factorial_cache[kCacheSize] = {1, 1, 2, 6, 24, 120, 720, 5040,
                                              40320, 362880, 3628800, 39916800,
                                              479001600, 6227020800};

  if (x < kCacheSize) {
    return factorial_cache[x];
  } else {
    float s = factorial_cache[kCacheSize - 1];
    for (int n = kCacheSize; n <= x; n++) {
      s *= n;
    }
    return s;
  }
}


// Compute the float factorial for an integer @x. This assumes x is at least
// 0.  This implementation precomputes the results for low values of x, in
// which case this is a constant time lookup.
//
// The vast majority of SH evaluations will hit these precomputed values.
// See http://mathworld.wolfram.com/floatFactorial.html
float floatFactorial(int x) {
  static const float dbl_factorial_cache[kCacheSize] = {1, 1, 2, 3, 8, 15, 48, 105,
                                                  384, 945, 3840, 10395, 46080,
                                                  135135};

  if (x < kCacheSize) {
    return dbl_factorial_cache[x];
  } else {
    float s = dbl_factorial_cache[kCacheSize - (x % 2 == 0 ? 2 : 1)];
    float n = x;
    while (n >= kCacheSize) {
      s *= n;
      n -= 2.0;
    }
    return s;
  }
}

// Evaluate the associated Legendre polynomial of degree @l and order @m at
// coordinate @x. The inputs must satisfy:
// 1. l >= 0
// 2. 0 <= m <= l
// 3. -1 <= x <= 1
// See http://en.wikipedia.org/wiki/Associated_Legendre_polynomials
//
// This implementation is based off the approach described in [1],
// instead of computing Pml(x) directly, Pmm(x) is computed. Pmm can be
// lifted to Pmm+1 recursively until Pml is found
float EvalLegendrePolynomial(int l, int m, float x) {
  // Compute Pmm(x) = (-1)^m(2m - 1)!!(1 - x^2)^(m/2), where !! is the float
  // factorial.
  float pmm = 1.0;
  // P00 is defined as 1.0, do don't evaluate Pmm unless we know m > 0
  if (m > 0) {
    float sign = (m % 2 == 0 ? 1 : -1);
    pmm = sign * floatFactorial(2 * m - 1) * pow(1 - x * x, m / 2.0);
  }

  if (l == m) {
    // Pml is the same as Pmm so there's no lifting to higher bands needed
    return pmm;
  }

  // Compute Pmm+1(x) = x(2m + 1)Pmm(x)
  float pmm1 = x * (2 * m + 1) * pmm;
  if (l == m + 1) {
    // Pml is the same as Pmm+1 so we are done as well
    return pmm1;
  }

  // Use the last two computed bands to lift up to the next band until l is
  // reached, using the recurrence relationship:
  // Pml(x) = (x(2l - 1)Pml-1 - (l + m - 1)Pml-2) / (l - m)
  for (int n = m + 2; n <= l; n++) {
    float pmn = (x * (2 * n - 1) * pmm1 - (n + m - 1) * pmm) / (n - m);
    pmm = pmm1;
    pmm1 = pmn;
  }
  // Pmm1 at the end of the above loop is equal to Pml
  return pmm1;
}


glm::vec3 ToVector(float phi, float theta) {
    float r = sin(theta);
    return glm::vec3(r * cos(phi), r * sin(phi), cos(theta));
}

void ToSphericalCoords(const glm::vec3& dir, float* phi, float* theta) {
    glm::vec3 _dir = glm::normalize(dir);
    // Explicitly clamp the z coordinate so that numeric errors don't cause it
    // to fall just outside of acos' domain.
    *theta = acos(glm::clamp(_dir.z, -1.0f, 1.0f));
    // We don't need to divide dir.y() or dir.x() by sin(theta) since they are
    // both scaled by it and atan2 will handle it appropriately.
    *phi = atan2(_dir.y, _dir.x);
}


float EvalSHSlow(int l, int m, float phi, float theta) {
    CHECK(l >= 0, "l must be at least 0.");
    CHECK(-l <= m && m <= l, "m must be between -l and l.");

    float kml = sqrt((2.0 * l + 1) * Factorial(l - abs(m)) /
                        (4.0 * M_PI * Factorial(l + abs(m))));
    if (m > 0) {
        return sqrt(2.0) * kml * cos(m * phi) *
            EvalLegendrePolynomial(l, m, cos(theta));
    } else if (m < 0) {
        return sqrt(2.0) * kml * sin(-m * phi) *
            EvalLegendrePolynomial(l, -m, cos(theta));
    } else {
        return kml * EvalLegendrePolynomial(l, 0, cos(theta));
    }
}

float EvalSHSlow(int l, int m, const glm::vec3& dir) {
    float phi, theta;
    ToSphericalCoords(dir, &phi, &theta);
    return EvalSH(l, m, phi, theta);
}


float EvalSH(int l, int m, float phi, float theta) {
  // If using the hardcoded functions, switch to cartesian
  if (l <= kHardCodedOrderLimit) {
    return EvalSH(l, m, ToVector(phi, theta));
  } else {
    // Stay in spherical coordinates since that's what the recurrence
    // version is implemented in
    return EvalSHSlow(l, m, phi, theta);
  }
}

float EvalSH(int l, int m, const glm::vec3& dir) {
  if (l <= kHardCodedOrderLimit) {
    // Validate l and m here (don't do it generally since EvalSHSlow also
    // checks it if we delegate to that function).
    CHECK(l >= 0, "l must be at least 0.");
    CHECK(-l <= m && m <= l, "m must be between -l and l.");

    switch (l) {
      case 0:
        return HardcodedSH00(dir);
      case 1:
        switch (m) {
          case -1:
            return HardcodedSH1n1(dir);
          case 0:
            return HardcodedSH10(dir);
          case 1:
            return HardcodedSH1p1(dir);
        }
      case 2:
        switch (m) {
          case -2:
            return HardcodedSH2n2(dir);
          case -1:
            return HardcodedSH2n1(dir);
          case 0:
            return HardcodedSH20(dir);
          case 1:
            return HardcodedSH2p1(dir);
          case 2:
            return HardcodedSH2p2(dir);
        }
      case 3:
        switch (m) {
          case -3:
            return HardcodedSH3n3(dir);
          case -2:
            return HardcodedSH3n2(dir);
          case -1:
            return HardcodedSH3n1(dir);
          case 0:
            return HardcodedSH30(dir);
          case 1:
            return HardcodedSH3p1(dir);
          case 2:
            return HardcodedSH3p2(dir);
          case 3:
            return HardcodedSH3p3(dir);
        }
      case 4:
        switch (m) {
          case -4:
            return HardcodedSH4n4(dir);
          case -3:
            return HardcodedSH4n3(dir);
          case -2:
            return HardcodedSH4n2(dir);
          case -1:
            return HardcodedSH4n1(dir);
          case 0:
            return HardcodedSH40(dir);
          case 1:
            return HardcodedSH4p1(dir);
          case 2:
            return HardcodedSH4p2(dir);
          case 3:
            return HardcodedSH4p3(dir);
          case 4:
            return HardcodedSH4p4(dir);
        }
    }

    // This is unreachable given the CHECK's above but the compiler can't tell.
    return 0.0;
  } else {
    // Not hard-coded so use the recurrence relation (which will convert this
    // to spherical coordinates).
    return EvalSHSlow(l, m, dir);
  }
}

std::vector<float> ProjectFunction(
    int order, const SphericalFunction& func, int sample_count) {
  CHECK(order >= 0, "Order must be at least zero.");
  CHECK(sample_count > 0, "Sample count must be at least one.");

  // This is the approach demonstrated in [1] and is useful for arbitrary
  // functions on the sphere that are represented analytically.
  const int sample_side = static_cast<int>(floor(sqrt(sample_count)));
  std::vector<float> coeffs;
  coeffs.assign(GetCoefficientCount(order), 0.0);

  // generate sample_side^2 uniformly and stratified samples over the sphere
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> rng(0.0, 1.0);

  int cnt = 0;
  for (int t = 0; t < sample_side; t++) {
    for (int p = 0; p < sample_side; p++) {
      float alpha = (t + rng(gen)) / sample_side;
      float beta = (p + rng(gen)) / sample_side;
      // See http://www.bogotobogo.com/Algorithms/uniform_distribution_sphere.php
      float phi = 2.0 * M_PI * beta;
      float theta = acos(2.0 * alpha - 1.0);
      // 随机球面坐标的生成没问题

      // evaluate the analytic function for the current spherical coords
      float func_value = func(phi, theta, cnt);

      // evaluate the SH basis functions up to band O, scale them by the
      // function's value and accumulate them over all generated samples
      for (int l = 0; l <= order; l++) {
        for (int m = -l; m <= l; m++) {
          float sh = EvalSH(l, m, phi, theta);
          coeffs[SHGetIndex(l, m)] += func_value * sh;
        }
      }
    }
  }
//  std::cout << cnt << std::endl;

  // scale by the probability of a particular sample, which is
  // 4pi/sample_side^2. 4pi for the surface area of a unit sphere, and
  // 1/sample_side^2 for the number of samples drawn uniformly.
  float weight = 4.0 * M_PI / (sample_side * sample_side);
  for (unsigned int i = 0; i < coeffs.size(); i++) {
     coeffs[i] *= weight;
  }

  return coeffs;
}