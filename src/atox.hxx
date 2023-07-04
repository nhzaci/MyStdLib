#pragma once

namespace my_std {

template <typename T> T strToT(const char *str) {
  size_t currIdx = 0;
  bool isNegative = false;

  // discards whitespace
  if (str[currIdx] == ' ') {
    ++currIdx;
  }

  // check '+' or '-' sign
  if (str[currIdx] == '-') {
    isNegative = true;
    ++currIdx;
  } else if (str[currIdx] == '+') {
    ++currIdx;
  }

  T acc = 0;
  auto maxVal = std::numeric_limits<T>::max();
  auto maxValDiv10 = maxVal / 10;
  auto minVal = std::numeric_limits<T>::min();
  auto minValDiv10 = minVal / 10;

  // get as many digits as possible, checking for overflow
  while (str[currIdx] >= '0' and str[currIdx] <= '9') {
    int currVal = str[currIdx] - '0';

    if (isNegative) {
      if (acc < minValDiv10 or minVal - acc * 10 > currVal)
        return minVal;
      acc *= 10;
      acc -= currVal;
    } else {
      // will overflow
      if (acc > maxValDiv10 or maxVal - acc * 10 < currVal)
        return maxVal;
      acc *= 10;
      acc += currVal;
    }

    ++currIdx;
  }

  return acc;
}

int atoi(const char *str) { return strToT<int>(str); }
long atol(const char *str) { return strToT<long>(str); }
long long atoll(const char *str) { return strToT<long long>(str); }

}; // namespace my_std
