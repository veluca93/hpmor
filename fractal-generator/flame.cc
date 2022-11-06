#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <omp.h>
#include <random>
#include <stdio.h>
#include <vector>

#include "common.h"

static constexpr size_t kNumVariations = 4;

struct Point2d {
  float rgb[3];
  float x;
  float y;
};

struct Function2d {
  struct VariationParams {
    float w = 0;                                    // Weight
    float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0; // Affine params
    void Read() { scanf("%f%f%f%f%f%f%f", &w, &a, &b, &c, &d, &e, &f); }
    __attribute__((always_inline)) std::pair<float, float>
    Affine(float x, float y) const {
      return {a * x + b * y + c, d * x + e * y + f};
    }
  };
  float probability;
  float rgb[3] = {};
  VariationParams variations[kNumVariations] = {};
  void Read() {
    int num_variations;
    scanf("%f%f%f%f%d", &probability, &rgb[0], &rgb[1], &rgb[2],
          &num_variations);
    assert(num_variations <= kNumVariations);
    for (int i = 0; i < num_variations; i++) {
      variations[i].Read();
    }
  }
  __attribute__((always_inline)) void Update(Point2d *point) const {
    auto v0 = variations[0].Affine(point->x, point->y);
    auto v1 = variations[1].Affine(point->x, point->y);
    auto v2 = variations[2].Affine(point->x, point->y);
    auto v3 = variations[3].Affine(point->x, point->y);
    for (size_t i = 0; i < 3; i++) {
      point->rgb[i] = (rgb[i] + point->rgb[i]) / 2;
    }
    float v2_scale =
        1.0f / (v2.first * v2.first + v2.second * v2.second + 1e-6f);
    float v3_scale =
        1.0f / std::sqrt(v3.first * v3.first + v3.second * v3.second + 1e-6f);
    point->x = v0.first * variations[0].w +
               std::sin(v1.first) * variations[1].w +
               v2.first * v2_scale * variations[2].w +
               (v3.first * v3.first - v3.second * v3.second) * v3_scale *
                   variations[3].w;
    point->y = v0.second * variations[0].w +
               std::sin(v1.second) * variations[1].w +
               v2.second * v2_scale * variations[2].w +
               (2 * v3.first * v3.second) * v3_scale * variations[3].w;
  }
};

template <typename T>
void atomic_float_cxg(std::atomic<float> &af, const T &getnew) {
  float current = af.load(std::memory_order_relaxed);
  float desired;
  do {
    desired = getnew(current);
  } while (!af.compare_exchange_weak(current, desired));
}

struct PixelState {
  std::atomic<float> frequency{1};
  std::atomic<float> rgb[3] = {};

  void Add(const float color[3]) {
    atomic_float_cxg(frequency, [](float f) { return f + 1.0f; });
    for (size_t i = 0; i < 3; i++) {
      atomic_float_cxg(rgb[i],
                       [i, color](float f) { return (f + color[i]) * 0.5f; });
    }
  }
};

struct State {
  std::mt19937 rng;
  Point2d point;
  std::uniform_real_distribution<float> dist =
      std::uniform_real_distribution<float>(0, 1);
  State(size_t i) : rng(i) {
    point.x = dist(rng);
    point.y = dist(rng);
    point.rgb[0] = 255;
    point.rgb[1] = 0;
    point.rgb[2] = dist(rng) * 255;
  }
};

int main(int argc, char **argv) {
  int num_functions;
  scanf("%d", &num_functions);
  std::vector<Function2d> functions(num_functions);
  for (size_t i = 0; i < num_functions; i++) {
    functions[i].Read();
  }

  float gamma;
  float x0;
  float y0;
  float half_width;
  float half_height;
  size_t supersample;
  size_t pixel_width;
  size_t pixel_height;
  size_t num_iters;
  if (scanf("%f%f%f%f%f%zu%zu%zu%zu", &gamma, &x0, &y0, &half_width,
            &half_height, &supersample, &pixel_width, &pixel_height,
            &num_iters) != 9) {
    fprintf(stderr,
            "Did not find position and size parameters, using defaults\n");
    pixel_height = pixel_width = 4096;
    x0 = y0 = 0.0;
    half_height = half_width = 1.0;
    num_iters = 1 << 28;
    gamma = 4.0;
    supersample = 4;
  }

  std::vector<float> cdf(num_functions + 1);
  for (size_t i = 0; i < num_functions; i++) {
    cdf[i + 1] = cdf[i] + functions[i].probability;
  }
  for (size_t i = 0; i < num_functions + 1; i++) {
    cdf[i] /= cdf.back();
  }

  std::vector<PixelState> pixels(pixel_height * pixel_width * supersample *
                                 supersample);

  size_t num_threads = omp_get_max_threads();
  std::vector<State> thread_states;
  for (size_t i = 0; i < num_threads; i++) {
    thread_states.emplace_back(i);
  }

  float w_offset = half_width - x0;
  float w_scale = 0.5f / half_width * pixel_width * supersample;
  float h_offset = half_height - y0;
  float h_scale = 0.5f / half_height * pixel_height * supersample;

#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < num_iters * supersample * supersample; i++) {
    size_t num_thread = omp_get_thread_num();
    State &state = thread_states[num_thread];
    float prob = state.dist(state.rng);
    int idx = std::lower_bound(cdf.begin(), cdf.end(), prob) - cdf.begin() - 1;
    functions[idx].Update(&state.point);
    int x = (state.point.x + w_offset) * w_scale;
    int y = (state.point.y + h_offset) * h_scale;
    if (x >= pixel_width * supersample)
      continue;
    if (y >= pixel_height * supersample)
      continue;
    if (x < 0)
      continue;
    if (y < 0)
      continue;
    pixels[y * pixel_width * supersample + x].Add(state.point.rgb);
  }

  std::vector<PixelState> scaled_pixels(pixel_width * pixel_height);

  // Convert pixel states to colors.
  std::atomic<float> max_freq{};
#pragma omp parallel for schedule(static)
  for (size_t xy = 0; xy < scaled_pixels.size(); xy++) {
    size_t x = xy % pixel_width;
    size_t y = xy / pixel_width;
    scaled_pixels[xy].frequency = 0;
    for (size_t j = 0; j < supersample; j++) {
      for (size_t k = 0; k < supersample; k++) {
        scaled_pixels[xy].frequency =
            scaled_pixels[xy].frequency.load() +
            pixels[(y * supersample + j) * pixel_width * supersample +
                   x * supersample + k]
                .frequency;
        for (size_t i = 0; i < 3; i++) {
          scaled_pixels[xy].rgb[i] =
              scaled_pixels[xy].rgb[i].load() +
              pixels[(y * supersample + j) * pixel_width * supersample +
                     x * supersample + k]
                  .rgb[i];
        }
      }
    }
    scaled_pixels[xy].frequency =
        scaled_pixels[xy].frequency.load() * 1.0f / (supersample * supersample);
    for (size_t i = 0; i < 3; i++) {
      scaled_pixels[xy].rgb[i] =
          scaled_pixels[xy].rgb[i].load() * 1.0f / (supersample * supersample);
    }
    float freq = scaled_pixels[xy].frequency.load();
    atomic_float_cxg(max_freq,
                     [freq](float old) { return std::max(freq, old); });
  }

  std::vector<unsigned char> img(pixel_height * pixel_width * 4, 255);
#pragma omp parallel for schedule(static)
  for (size_t xy = 0; xy < pixel_height * pixel_width; xy++) {
    float alpha =
        std::log(scaled_pixels[xy].frequency) / std::log(max_freq.load());
    for (size_t i = 0; i < 3; i++) {
      img[4 * xy + i] =
          std::max(0.f, std::min(255.f, scaled_pixels[xy].rgb[i] *
                                            std::pow(alpha, 1 / gamma)));
    }
  }

  WritePNG(img.data(), pixel_width, pixel_height, "flame.png");
}
