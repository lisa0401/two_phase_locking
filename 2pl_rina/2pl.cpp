#pragma once

#include <unistd.h>

#include <iostream>
#include <thread>

#include "database.hh"
#include "definition.hh"
#include "lock.cpp"
#include "transaction.hh"

struct Task {
  Operation operation;
  int index;
};

// Tonyの激レアTシャツの購入者リスト
bool purchased[THREAD_NUM] = {false};

void *worker(void *arg) {
  int id = *(int *)arg;

  // TX: r(x), w(x)
  Transaction tx;
RETRY:
  usleep(10);
  tx.begin();

  int x = 0;
  int current_stock = tx.read(x);
  if (tx.state == State::ABORTED) {
    tx.abort();
    goto RETRY;
  }

  usleep(1000);
  if (current_stock > 0) {
    tx.write(x, current_stock - 1);
    if (tx.state == State::ABORTED) {
      tx.abort();
      goto RETRY;
    } else {
      purchased[id] = true;
    }
  }

  tx.commit();

  return NULL;
}

void print_db() {
  std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
  std::cout << "在庫" << std::endl;
  for (int i = 0; i < DATA_SIZE; i++) {
    std::cout << database[i].value << " ";
  }
  std::cout << std::endl;
  std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
}

int main(int argc, char *argv[]) {
  print_db();

  // Create threads
  std::vector<std::thread> threads;
  int threadIds[THREAD_NUM];

  std::cout << "start experiment..." << std::endl;
  for (int i = 0; i < THREAD_NUM; i++) {
    threadIds[i] = i;
    threads.emplace_back(worker, &threadIds[i]);
  }

  // Join threads
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].join();
  }
  std::cout << "end experiment..." << std::endl;

  std::cout << "==========================================" << std::endl;
  // count purchaseds
  int sum = 0;
  for (int i = 0; i < THREAD_NUM; i++) {
    if (purchased[i] == true) {
      sum++;
    }
  }
  std::cout << THREAD_NUM << "人中、Tonyの激レアTシャツを買った人は、" << sum
            << "人" << std::endl;
  std::cout << "==========================================" << std::endl;

  print_db();
  return 0;
}
