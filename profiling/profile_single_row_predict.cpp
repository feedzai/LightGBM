/**
 *
 * Quick & dirty Single Row Predict benchmark.
 *
 *
 * Add this .cpp to a new "profiling/" folder and the following line to the end of CMakeLists.txt:
 *
 *   OPTION(BUILD_PROFILING_TESTS "Set to ON to compile profiling executables for development and benchmarks." OFF)
 *   if(BUILD_PROFILING_TESTS)
 *      # For profiling builds with valgrind/callgrind use -DUSE_DEBUG=ON
 *     add_executable(lightgbm_profile_single_row_predict profiling/profile_single_row_predict.cpp ${SOURCES})
 *   endif(BUILD_PROFILING_TESTS)
 *
 *
 * Requirements:
 *
 *   - Add a "LightGBM_model.txt" file at the repo root.
 *   - Adapt ``values`` below to your model to have at least 2 different input rows.
 *
 * Compilation:
 *
 *   cmake .. -DBUILD_PROFILING_TESTS=ON && make -j4
 *
 * Usage:
 *
 *   time ./lightgbm_profile_single_row_predict <# threads> <# points> [f]  # f uses the Fast single row prediction
 *
 *
 * Alberto Ferreira, 2021
 */

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <thread>
#include <ctime>
#include <cstring>
#include "LightGBM/c_api.h"

using namespace std;

#define FMT_HEADER_ONLY
#include "LightGBM/../../external_libs/fmt/include/fmt/format.h"

inline void predict(BoosterHandle handle,
            const void* data,
            int32_t ncol,
            int num_iterations,
            int64_t* out_len,
            double* out_result) {
  if (0 != LGBM_BoosterPredictForMatSingleRow(
    handle,
    data,
    C_API_DTYPE_FLOAT64,
    ncol,
    1, // is_row_major
    C_API_PREDICT_NORMAL,
    0, // start_iteration
    num_iterations,
    "",
    out_len,
    out_result)) {
      throw std::exception();
  }
}

int main() {


  BoosterHandle boosterHandle2, boosterHandle1;
  int num_iterations;

  cout << "start\n";
  LGBM_BoosterCreateFromModelfile("./LightGBM_model.txt", &num_iterations, &boosterHandle1);
  LGBM_BoosterCreateFromModelfile("./LightGBM_model.txt", &num_iterations, &boosterHandle2);

  /*
  Dataset:
      feature_names=amount num1_float num2_double num3_int
      fraud := 400<amount<700 & cat1_string="C"~=2 & num1_float < 70
  Use input "rows" that provide different output scores to ensure thread-safety:
  */
  double values[] = {
      0.25, 1.4, 0.12, -0.5,
      500, 2, 9999, 200,
  };
  const size_t NROWS=2;
  const int NUM_FEATURES = 4;
  std::vector<double> scores(NROWS);

  int64_t dummy_out_len;

  std::fill(scores.begin(), scores.end(),  0);
  predict(boosterHandle1, values             , NUM_FEATURES, num_iterations, &dummy_out_len, &scores[0]);
  predict(boosterHandle1, values+NUM_FEATURES, NUM_FEATURES, num_iterations, &dummy_out_len, &scores[1]);
  fmt::print("Scores by boosterHandle1: {:.6g}, {:.6g}\n", scores[0], scores[1]);

  std::fill(scores.begin(), scores.end(),  0);
  predict(boosterHandle2, values             , NUM_FEATURES, num_iterations, &dummy_out_len, &scores[0]);
  predict(boosterHandle2, values+NUM_FEATURES, NUM_FEATURES, num_iterations, &dummy_out_len, &scores[1]);
  fmt::print("Scores by boosterHandle2: {:.6g}, {:.6g}\n", scores[0], scores[1]);
}
