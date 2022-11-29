#ifndef VECTOR__H
#define VECTOR__H

#include <Arduino.h>

template <class T>
class Vector {
private:
  size_t size;
  T * vector;

public:
  Vector(size_t size);
  Vector(Vector&&);
  Vector(Vector&) = delete;
  ~Vector();
  T& operator[](size_t i);
  Vector<T>& operator=(Vector<T>&) = delete;
  Vector<T>& operator=(Vector<T>&&);
  T* data();
};


template <class T>
Vector<T>::Vector(size_t size) {
  this->size = size;
  vector = new T[size];
  // No valido que haya lugar.
}

template <class T>
Vector<T>::Vector(Vector<T>&& other) {
  this->size = other.size;
  this->vector = other.vector;
  other.vector = nullptr;
}

template <class T>
Vector<T>::~Vector() {
  delete[] vector;
}

template <class T>
Vector<T>& Vector<T>::operator=(Vector<T> && other) {
  this->size = other.size;
  this->vector = other.vector;
  other.vector = nullptr;
}

template <class T>
T* Vector<T>::data() {
  return vector;
}

template <class T>
T& Vector<T>::operator[](size_t i) {
  return vector[i];
}



#endif