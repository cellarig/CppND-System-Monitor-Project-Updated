#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <string>

class Processor {
 public:
  float Utilization();

 private:
  // cached ticks holder (will be updated every time Utilization called)
  long mCached_active_ticks{0};
  long mCached_idle_ticks{0};
};

#endif