#ifndef LTE_Stack_h__
#define LTE_Stack_h__

#include "Common.h"
#include <vector>

template <class T, int MaxElements = 100>
struct Stack {
  std::vector<T> elements;

  Stack() {}
  Stack(const T& elem) {
    elements.push_back(elem);
  }

  operator T&() {
    return back();
  }

  operator const T&() const {
    return back();
  }

  T& operator[](size_t index) {
    return elements[index];
  }

  T const& operator[](size_t index) const {
    return elements[index];
  }

  T& operator->() {
    return elements.back();
  }

  T const& operator->() const {
    return elements.back();
  }

  T& back() {
    DEBUG_ASSERT(elements.size());
    return elements.back();
  }

  T const& back() const {
    DEBUG_ASSERT(elements.size());
    return elements.back();
  }

  void clear() {
    elements.clear();
  }

  std::vector<T>& AsStdVector() {
    return elements;
  }

  std::vector<T> const& AsStdVector() const {
    return elements;
  }

  typename std::vector<T>::iterator begin() {
    return elements.begin();
  }

  typename std::vector<T>::const_iterator begin() const {
    return elements.begin();
  }

  T const* data() const {
    return elements.data();
  }

  typename std::vector<T>::iterator end() {
    return elements.end();
  }

  typename std::vector<T>::const_iterator end() const {
    return elements.end();
  }

  bool empty() const {
    return elements.empty();
  }

  void pop() {
    DEBUG_ASSERT(elements.size());
    elements.pop_back();
  }

  Stack& push(const T& t) {
    DEBUG_ASSERT(elements.size() < MaxElements);
    elements.push_back(t);
    return *this;
  }

  size_t size() const {
    return elements.size();
  }

  Stack& operator+=(const T& t) {
    push(back() + t);
    return *this;
  }

  Stack& operator-=(const T& t) {
    push(back() - t);
    return *this;
  }

  Stack& operator*=(const T& t) {
    push(back() * t);
    return *this;
  }

  Stack& operator/=(const T& t) {
    push(back() / t);
    return *this;
  }

  Stack& operator=(const T& t) {
    clear();
    elements.push_back(t);
    return *this;
  }
};

#endif
