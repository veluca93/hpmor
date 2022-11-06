#include <array>
#include <assert.h>
#include <random>
#include <stdio.h>

#include "lodepng.h"

#if __NVCC__
#define CUDAFUN __device__ __host__
#else
#define CUDAFUN
#endif

struct Color {
  uint8_t r = 0, g = 0, b = 0;
  CUDAFUN void Avg(Color oth) {
    r = (r * 7 + oth.r) >> 3;
    g = (g * 7 + oth.g) >> 3;
    b = (b * 7 + oth.b) >> 3;
  }
};

struct Pixel {
  Color color;
  uint32_t count = 0;
  CUDAFUN Pixel Add(Color oth) const {
    Pixel result = *this;
    result.color.Avg(oth);
    if (result.count != 0xFFFFFFFFu) {
      result.count++;
    }
    return result;
  }
};

constexpr int kNumFunctions = 3;
std::array<Color, kNumFunctions> kColors{Color{0, 192, 255}, Color{128, 0, 255},
                                         Color{0, 0, 255}};

struct Point {
  float x, y, z;
  CUDAFUN Point Affine(const float *coeffs) const {
    float c00 = coeffs[1];
    float c01 = coeffs[2];
    float c02 = coeffs[3];
    float c03 = coeffs[4];
    float c10 = coeffs[5];
    float c11 = coeffs[6];
    float c12 = coeffs[7];
    float c13 = coeffs[8];
    float c20 = coeffs[9];
    float c21 = coeffs[10];
    float c22 = coeffs[11];
    float c23 = coeffs[12];
    return {c00 * x + c01 * y + c02 * z + c03,
            c10 * x + c11 * y + c12 * z + c13,
            c20 * x + c21 * y + c22 * z + c23};
  }
};

struct Base1 {
  CUDAFUN Point operator()(Point p) { return p; }
};
struct Base2 {
  CUDAFUN Point operator()(Point p) {
    float radius = p.x * p.x + p.y * p.y + p.z * p.z + 1e-8f;
    return {p.x / radius, p.y / radius, p.z / radius};
  }
};
struct Base3 {
  CUDAFUN Point operator()(Point p) {
    return {sinf(p.x), sinf(p.y), sinf(p.z)};
  }
};

struct Function {
  static constexpr int kNumBases = 3;

  CUDAFUN Point operator()(Point p, const float *weights) {
    Point p0 = Base1()(p.Affine(weights));
    float w0 = weights[0];
    Point p1 = Base2()(p.Affine(weights + 13));
    float w1 = weights[13];
    Point p2 = Base3()(p.Affine(weights + 26));
    float w2 = weights[26];
    return {w0 * p0.x + w1 * p1.x + w2 * p2.x,
            w0 * p0.y + w1 * p1.y + w2 * p2.y,
            w0 * p0.z + w1 * p1.z + w2 * p2.z};
  }
};

constexpr size_t kWidth = 2 * 768 + 1;
constexpr size_t kHeight = kWidth;
constexpr size_t kNumFrames = 256;
constexpr size_t kTotalIters = 1LU << 42;
constexpr int kKernelRadius = 4;
constexpr float kDistMultiplier = 1.0f;

inline auto InitWeights(int seed) {
  std::array<std::array<float, 13 * Function::kNumBases>, kNumFunctions>
      weights;
  std::mt19937_64 rng(seed);
  std::normal_distribution<float> wdist(1.0f / Function::kNumBases,
                                        2.0f / Function::kNumBases);
  std::uniform_real_distribution<float> cdist(-2.0f, +2.0f);
  for (auto &ww : weights) {
    for (size_t i = 0; i < ww.size(); i++) {
      ww[i] = (i % 13 == 0 ? wdist(rng) : cdist(rng));
    }
  }
  return weights;
}

inline void WritePPM(const uint8_t *data, size_t w, size_t h,
                     const char *name) {
  FILE *f = fopen(name, "w");
  assert(f);
  fprintf(f, "P6\n%lu %lu\n255\n", w, h);
  fwrite(data, 1, w * h * 3, f);
  fclose(f);
}

inline void WritePNG(const uint8_t *data, size_t w, size_t h,
                     const char *name) {
  unsigned error = lodepng_encode32_file(name, data, w, h);
  if (error)
    printf("error %u: %s\n", error, lodepng_error_text(error));
  assert(!error);
}
