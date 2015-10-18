#include <iostream>
#include <algorithm>
#include <functional>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include <vector>
#include <ctime>

#include "threadpool.h"

using namespace std;

const int REPEAT_COUNT = 10;
const int LIST_SIZE = 100000;
const int FORK_LIMIT = LIST_SIZE / 10;  // Limits thread count to ~10

// Mergesort with fork/join
void mergesort_pooled(vector<int>::iterator first, vector<int>::iterator last, ThreadPool* pool) {
  if (last - first > 1) {
    vector<int>::iterator middle = first + (last - first) / 2;
    if (last - first > FORK_LIMIT) {
      // If the proposed list segment is too big, spawn a thread to sort the first half.
      auto future = pool->enqueue([=](){ mergesort_pooled(first, middle, pool); });
      // And sort the other half directly.
      mergesort_pooled(middle, last, pool);
      future.get();
    } else {
      mergesort_pooled(first, middle, pool);
      mergesort_pooled(middle, last, pool);
    }
    inplace_merge(first, middle, last);
  }
}


// Mergesort with fork/join
void mergesort_threaded(vector<int>::iterator first, vector<int>::iterator last) {
  if (last - first > 1) {
    vector<int>::iterator middle = first + (last - first) / 2;
    if (last - first > FORK_LIMIT) {
      // If the proposed list segment is too big, spawn a thread to sort the first half.
      thread t1([=](){ mergesort_threaded(first, middle); });
      // And sort the other half directly.
      mergesort_threaded(middle, last);
      t1.join();
    } else {
      mergesort_threaded(first, middle);
      mergesort_threaded(middle, last);
    }
    inplace_merge(first, middle, last);
  }
}


// Normal mergesort
void mergesort(vector<int>::iterator first, vector<int>::iterator last) {
  if (last - first > 1) {
    vector<int>::iterator middle = first + (last - first) / 2;
    mergesort(first, middle);
    mergesort(middle, last);
    inplace_merge(first, middle, last);
  }
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

  ThreadPool pool(10);
  auto pooled_t1 = chrono::system_clock::now();
  for (int i = 0; i != REPEAT_COUNT; i++) {
    vector<int> list;
    for (int i = 0; i != LIST_SIZE; i++) {
      list.push_back(rand() % 1000);
    }
    mergesort_pooled(list.begin(), list.end(), &pool);
  }
  auto pooled_t2 = chrono::system_clock::now();
  chrono::duration<double> pooled_secs = pooled_t2 - pooled_t1;
  cout << "Pooled: " << pooled_secs.count() << endl;
}