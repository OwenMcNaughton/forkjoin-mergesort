#include <iostream>
#include <algorithm>
#include <functional>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include <ctime>

using namespace std;

const int REPEAT_COUNT = 10;
const int LIST_SIZE = 100000;
const int FORK_LIMIT = LIST_SIZE / 10;  // Limits thread count to ~10


// Mergesort with fork/join
template<typename RandomAccessIterator, typename Order>
void mergesort_threaded(RandomAccessIterator first, RandomAccessIterator last, Order order) {
  if (last - first > 1) {
    RandomAccessIterator middle = first + (last - first) / 2;
    if (last - first > FORK_LIMIT) {
      // If the proposed list segment is too big, spawn a thread to sort the first half.
      thread t1([=](){ mergesort_threaded(first, middle, order); });
      // And sort the other half directly.
      mergesort_threaded(middle, last, order);
      t1.join();
    } else {
      mergesort_threaded(first, middle, order);
      mergesort_threaded(middle, last, order);
    }
    inplace_merge(first, middle, last, order);
  }
}
template<typename RandomAccessIterator>
void mergesort_threaded(RandomAccessIterator first, RandomAccessIterator last) {
  mergesort_threaded(first, last, std::less<typename std::iterator_traits<RandomAccessIterator>::value_type>());
}


// Normal mergesort
template<typename RandomAccessIterator, typename Order>
void mergesort(RandomAccessIterator first, RandomAccessIterator last, Order order) {
  if (last - first > 1) {
    RandomAccessIterator middle = first + (last - first) / 2;
    mergesort(first, middle, order);
    mergesort(middle, last, order);
    inplace_merge(first, middle, last, order);
  }
}
template<typename RandomAccessIterator>
void mergesort(RandomAccessIterator first, RandomAccessIterator last) {
  mergesort(first, last, std::less<typename std::iterator_traits<RandomAccessIterator>::value_type>());
}

bool sorted(vector<int> list) {
  for (int i = 1; i != list.size(); i++) {
    if (list[i] < list[i-1]) return false;
  }
  return true;
}

int main(int argc, char** argv) {
  auto normal_t1 = chrono::system_clock::now();
  for (int i = 0; i != REPEAT_COUNT; i++) {
    vector<int> list;
    for (int i = 0; i != LIST_SIZE; i++) {
      list.push_back(rand() % 1000);
    }
    mergesort(list.begin(), list.end());
  }
  auto normal_t2 = chrono::system_clock::now();
  chrono::duration<double> normal_secs = normal_t2 - normal_t1;
  cout << "Normal: " << normal_secs.count() << endl;

  auto threaded_t1 = chrono::system_clock::now();
    for (int i = 0; i != REPEAT_COUNT; i++) {
    vector<int> list;
    for (int i = 0; i != LIST_SIZE; i++) {
      list.push_back(rand() % 1000);
    }
    mergesort_threaded(list.begin(), list.end());
  }
  auto threaded_t2 = chrono::system_clock::now();
  chrono::duration<double> threaded_secs = threaded_t2 - threaded_t1;
  cout << "Threaded: " << threaded_secs.count() << endl;
}