#pragma once

#include <pthread.h>

#include "definition.hh"
#include "lock.cpp"

class Tuple {
 public:
  int value = 1;
  Lock lock;
};

extern Tuple database[DATA_SIZE];
