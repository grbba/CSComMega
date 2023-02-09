

/**
 * @file Vector.h
 * @author Gregor Baues
 * @brief Vector datastructure implementation based on compile time allocated memory. 
 * Size of the vector is part of the template
 * @date 2023-01-13
 *
 * @copyright Copyright (c) 2023
 *
 * This is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * See the GNU General Public License for more details <https://www.gnu.org/licenses/>
 */
#ifndef ARDUINO_VECTOR_H
#define ARDUINO_VECTOR_H

#include <Arduino.h>
#include <DCSIconfig.h>
#include <DCSIlog.h>

template<typename T, size_t N>
class Vector {
 public:
  Vector() {
    size_ = 0;
  }

  void push_back(const T& value) {
    if (size_ >= N) {
      return;
    }
    data_[size_++] = value;
  }

  void pop_back() {
    if (size_ > 0) {
      --size_;
    }
  }

  T& operator[](size_t index) {
    return data_[index];
  }

  const T& operator[](size_t index) const {
    return data_[index];
  }

  size_t size() const {
    return size_;
  }

  size_t capacity() const {
    return N;
  }

  int find(const T& value) const {
    for (int i = 0; i < size_; ++i) {
      if (data_[i] == value) {
        return i;
      }
    }
    return -1;
  }
  
 private:
  T data_[N];
  size_t size_;
};

#endif