#pragma once

#include <vector>

#include "database.hh"
#include "definition.hh"
#include "lock.cpp"

enum class Operation { READ, WRITE };

enum class State { ACTIVE, COMMITTED, ABORTED };

class ReadOpe {
 public:
  int index;
  int value;
  Tuple &tuple;
  bool is_upgraded = false;
  ReadOpe(int index, int value, Tuple &tuple)
      : index(index), value(value), tuple(tuple) {}
};

class WriteOpe {
 public:
  int index;
  int value;
  Tuple &tuple;
  WriteOpe(int index, int value, Tuple &tuple)
      : index(index), value(value), tuple(tuple) {}
};

class Transaction {
 public:
  State state;
  Transaction() : state(State::ACTIVE) {}
  void begin();
  void commit();
  void abort();
  int read(int index);
  void write(int index, int value);

 private:
  std::vector<ReadOpe> read_set;
  std::vector<WriteOpe> write_set;
  void unlock();
};