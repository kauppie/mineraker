#ifndef NEURALRAKER_H
#define NEURALRAKER_H

#include <cmath>
#include <random>
#include <vector>

#include "mineraker.hpp"

namespace rake {
// @brief Class designed to learn to play minesweeper with various settings and
// difficulty.
class NeuralRaker {
public:
private:
  std::vector<float> m_input_layer;
  std::vector<std::vector<float>> m_hidden_layers;
  std::vector<float> m_output_layer;

  std::mt19937_64 m_rand_gen;

public:
  NeuralRaker() {}
  ~NeuralRaker() {}

private:
  static constexpr float m_sigmoid(float value) {
    return 1.0f / (1.0f + std::exp(value));
  }

  static constexpr float m_sigmoid_derivative(float value) {
    return m_sigmoid(value) * (1.0f - m_sigmoid(value));
  }

  void m_train(size_type iterations) {}
};
} // namespace rake

#endif