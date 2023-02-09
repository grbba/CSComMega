/**
 * @file HashMap.h
 * @author Gregor Baues
 * @brief HashMap datastructure implementation based on compile time allocated memeory. 
 * Size of the HashMap is part of the template
 * @date 2023-01-13
 *
 * @copyright Copyright (c) 2023
 * @todo : provide the possibility to add the comparator at compiletime instatiation of the class
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

#ifndef HashMap_h
#define HashMap_h

#include <Arduino.h>
#include <DCSIlog.h>

template <typename K, typename T, int N>
class HashMap {
  private:
    struct Entry {
      K key;
      T value;
      bool used;
    };

    Entry entries[N];
    int count;

    static bool compare( char *a, char *b) {
        if ( strcmp(a,b) == 0 ) return true;
        return false;
    }


    int hash(K key) {
      int h = 0;
      for (int i = 0; key[i]; i++) {
        h = 31 * h + key[i];
      }
      return h % N;
    }

    template< typename KC, typename CO>
    static bool compareKeys(KC a, KC b, CO f) {
        return f(a,b); 
    }

  public:
    HashMap() {
      count = 0;
      for (int i = 0; i < N; i++) {
        entries[i].used = false;
      }
    }

    void put(K key, T value) {
      
      int h = hash(key);
      int i = h;
      while (entries[i].used && !compareKeys(entries[i].key,key,compare)) {
        i = (i + 1) % N;
        if (i == h) {
          // Return if the table is full
          return;
        }
      }
      entries[i].key = key;
      entries[i].value = value;
      entries[i].used = true;
      count++;
    }

    T get(K key) {
      int h = hash(key);
      int i = h;
      while (entries[i].used) {
        if (compareKeys(entries[i].key,key, compare)) {
          return entries[i].value;
        }
        i = (i + 1) % N;
        if (i == h) {
          break;
        }
      }
      return T();
    }

    bool containsKey(K key) {
      int h = hash(key);
      int i = h;
      while (entries[i].used) {
        // std::cout << "Key " << entries[i].key << " " << key << std::endl;
        if ( compareKeys(entries[i].key,key, compare) ) {
          return true;
        }
        i = (i + 1) % N;
        if (i == h) {
          break;
        }
      }
      return false;
    }

    int size() {
      return count;
    }

    void clear() {
      count = 0;
      for (int i = 0; i < N; i++) {
        entries[i].used = false;
      }
    }
};
#endif