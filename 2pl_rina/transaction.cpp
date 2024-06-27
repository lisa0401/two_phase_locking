#pragma once

#include "transaction.hh"

#include <pthread.h>

#include <iostream>

#include "database.hh"

void Transaction::begin() { state = State::ACTIVE; }

void Transaction::commit() {
  unlock();
  read_set.clear();
  write_set.clear();
  state = State::COMMITTED;
}

void Transaction::abort() {
  unlock();
  read_set.clear();
  write_set.clear();
}

int Transaction::read(int index) {
  // if: r(1) ... r(1)
  for (auto &read : read_set) {
    if (read.index == index) {
      return read.value;
    }
  }

  // if: w(1) ... r(1)
  for (auto &write : write_set) {
    if (write.index == index) {
      return write.value;
    }
  }

  // else: initial read
  Tuple &tuple = database[index];
  bool res = tuple.lock.read_try_lock();
  if (!res) {
    state = State::ABORTED;
    return 0;
  }
  int value = __atomic_load_n(&tuple.value, __ATOMIC_SEQ_CST);
  read_set.emplace_back(index, value, tuple);
  return value;
}

void Transaction::write(int index, int value) {
  // if: w(1) ... w(1)
  for (auto &write : write_set) {
    if (write.index == index) {
      // もうすでにWロックを取ってる
      __atomic_store_n(&write.tuple.value, value, __ATOMIC_SEQ_CST);
      write.value = value;
      return;
    }
  }

  // if: r(1) ... w(1)
  for (auto &read : read_set) {
    if (read.index == index) {
      // upgrade lock from read to write
      if (!read.tuple.lock.try_upgrade_lock()) {
        state = State::ABORTED;
        return;
      }
      // remove corresponding index from read_set and add to write_set
      read.is_upgraded = true;
      write_set.emplace_back(index, read.value, read.tuple);
      // print out writeset
      __atomic_store_n(&read.tuple.value, value, __ATOMIC_SEQ_CST);
      return;
    }
  }

  // else: initial write
  Tuple &tuple = database[index];
  if (!tuple.lock.write_try_lock()) {
    // if there is a write lock already: abort and retry immediately (no wait)
    state = State::ABORTED;
    return;
  }
  write_set.emplace_back(index, value, tuple);
  __atomic_store_n(&tuple.value, value, __ATOMIC_SEQ_CST);
}

void Transaction::unlock() {
  for (auto &read : read_set) {
    if (!read.is_upgraded) read.tuple.lock.read_unlock();
  }
  for (auto &write : write_set) {
    write.tuple.lock.write_unlock();
  }
}