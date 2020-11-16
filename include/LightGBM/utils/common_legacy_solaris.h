/*!
 * Copyright (c) 2016 Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See LICENSE file in the project root for license information.
 */
/*!
 * This file is meant to be used ONLY IN SOLARIS!
 * The newer code that replaced it is much faster!
 */
#ifndef LIGHTGBM_UTILS_COMMON_FUN_LEGACY_H_
#define LIGHTGBM_UTILS_COMMON_FUN_LEGACY_H_

#include <LightGBM/utils/log.h>

#include <sstream>
#include <type_traits>
#include <vector>

namespace LightGBM {

namespace Common {

inline static unsigned CountDecimalDigit32(uint32_t n) {	
#if defined(_MSC_VER) || defined(__GNUC__)	
  static const uint32_t powers_of_10[] = {	
    0,	
    10,	
    100,	
    1000,	
    10000,	
    100000,	
    1000000,	
    10000000,	
    100000000,	
    1000000000	
  };	
#ifdef _MSC_VER	
  // NOLINTNEXTLINE	
  unsigned long i = 0;	
  _BitScanReverse(&i, n | 1);	
  uint32_t t = (i + 1) * 1233 >> 12;	
#elif __GNUC__	
  uint32_t t = (32 - __builtin_clz(n | 1)) * 1233 >> 12;	
#endif	
  return t - (n < powers_of_10[t]) + 1;	
#else	
  if (n < 10) return 1;	
  if (n < 100) return 2;	
  if (n < 1000) return 3;	
  if (n < 10000) return 4;	
  if (n < 100000) return 5;	
  if (n < 1000000) return 6;	
  if (n < 10000000) return 7;	
  if (n < 100000000) return 8;	
  if (n < 1000000000) return 9;	
  return 10;	
#endif	
}	

inline static void Uint32ToStr(uint32_t value, char* buffer) {	
  const char kDigitsLut[200] = {	
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',	
    '1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',	
    '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',	
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',	
    '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',	
    '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',	
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',	
    '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',	
    '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',	
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'	
  };	
  unsigned digit = CountDecimalDigit32(value);	
  buffer += digit;	
  *buffer = '\0';	

  while (value >= 100) {	
    const unsigned i = (value % 100) << 1;	
    value /= 100;	
    *--buffer = kDigitsLut[i + 1];	
    *--buffer = kDigitsLut[i];	
  }	

  if (value < 10) {	
    *--buffer = static_cast<char>(value) + '0';	
  } else {	
    const unsigned i = value << 1;	
    *--buffer = kDigitsLut[i + 1];	
    *--buffer = kDigitsLut[i];	
  }	
}	

inline static void Int32ToStr(int32_t value, char* buffer) {	
  uint32_t u = static_cast<uint32_t>(value);	
  if (value < 0) {	
    *buffer++ = '-';	
    u = ~u + 1;	
  }	
  Uint32ToStr(u, buffer);	
}	

inline static void DoubleToStr(double value, char* buffer, size_t buffer_len) {	
  #ifdef _MSC_VER	
  int num_chars = sprintf_s(buffer, buffer_len, "%.17g", value);	
  #else	
  int num_chars = snprintf(buffer, buffer_len, "%.17g", value);	
  #endif	
  CHECK_GE(num_chars, 0);	
}


template<typename T, bool is_float, bool is_unsign>	
struct __TToStringHelperFast {	
  void operator()(T value, char* buffer, size_t) const {	
    Int32ToStr(value, buffer);	
  }	
};	

template<typename T>	
struct __TToStringHelperFast<T, true, false> {	
  void operator()(T value, char* buffer, size_t buf_len)	
  const {	
    #ifdef _MSC_VER	
    int num_chars = sprintf_s(buffer, buf_len, "%g", value);	
    #else	
    int num_chars = snprintf(buffer, buf_len, "%g", value);	
    #endif	
    CHECK_GE(num_chars, 0);	
  }	
};	

template<typename T>	
struct __TToStringHelperFast<T, false, true> {	
  void operator()(T value, char* buffer, size_t) const {	
    Uint32ToStr(value, buffer);	
  }	
};	

template<typename T>	
inline static std::string _ArrayToStringFast(const std::vector<T>& arr, size_t n) {	
  if (arr.empty() || n == 0) {	
    return std::string("");	
  }	
  __TToStringHelperFast<T, std::is_floating_point<T>::value, std::is_unsigned<T>::value> helper;	
  const size_t buf_len = 16;	
  std::vector<char> buffer(buf_len);	
  std::stringstream str_buf;	
  helper(arr[0], buffer.data(), buf_len);	
  str_buf << buffer.data();	
  for (size_t i = 1; i < std::min(n, arr.size()); ++i) {	
    helper(arr[i], buffer.data(), buf_len);	
    str_buf << ' ' << buffer.data();	
  }	
  return str_buf.str();	
}	

inline static std::string _ArrayToString(const std::vector<double>& arr, size_t n) {	
  if (arr.empty() || n == 0) {	
    return std::string("");	
  }	
  const size_t buf_len = 32;	
  std::vector<char> buffer(buf_len);	
  std::stringstream str_buf;	
  DoubleToStr(arr[0], buffer.data(), buf_len);	
  str_buf << buffer.data();	
  for (size_t i = 1; i < std::min(n, arr.size()); ++i) {	
    DoubleToStr(arr[i], buffer.data(), buf_len);	
    str_buf << ' ' << buffer.data();	
  }	
  return str_buf.str();	
}


template<bool high_precision_output = false, typename T>
inline static typename std::enable_if<high_precision_output==false, std::string>::type
ArrayToString(const std::vector<T>& arr, size_t n) {
    return _ArrayToStringFast(arr, n);
}

template<bool high_precision_output, typename T>
inline static typename std::enable_if<
(high_precision_output==true) && (std::is_same<T, double>::value), std::string>::type
ArrayToString(const std::vector<T>& arr, size_t n) {    
    return _ArrayToString(arr, n);
}

}  // namespace Common

}  // namespace LightGBM

#endif   // LightGBM_UTILS_COMMON_FUN_H_