#pragma once
#include <atomic>

class Lock {
 public:
  int counter = 0;
  /*
  elock: counter = -1
  slock: 0 < counter
  unlock: 0
  */

  bool read_try_lock() {
    int expected = __atomic_load_n(&counter, __ATOMIC_SEQ_CST);
    int desired;
    if (expected != -1) {      // slock or unlock
      desired = expected + 1;  // slock
      if (__atomic_compare_exchange_n(&counter, &expected, desired, false,
                                      __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        return true;
      }
    }
    return false;
  }

  void read_lock() {
    while (!read_try_lock()) {
      // spin lock
    }
  }

  void read_unlock() { __atomic_fetch_sub(&counter, 1, __ATOMIC_SEQ_CST); }

  bool write_try_lock() {
    int expected = 0;
    int desired = -1;
    if (__atomic_compare_exchange_n(&counter, &expected, desired, false,
                                    __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
      return true;
    }
    return false;
  }

  void write_lock() {
    while (!write_try_lock()) {
      // spin lock
    }
  }

  void write_unlock() { __atomic_store_n(&counter, 0, __ATOMIC_SEQ_CST); }

  bool try_upgrade_lock() {
    int expected = __atomic_load_n(&counter, __ATOMIC_SEQ_CST);
    int desired;
    if (expected == 1) {  // 自分だけがslockを保持している
      desired = -1;
      if (__atomic_compare_exchange_n(&counter, &expected, desired, false,
                                      __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        return true;
      }
    }
    return false;
  }

  void upgrade_lock() {
    while (!try_upgrade_lock()) {
      // spin
    }
  }
};