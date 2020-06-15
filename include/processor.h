#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <string>

class Processor {
 public:
  float Utilization();

 private:
  // cached holders (will be updated every time Utilization called)
  float mCached_active{0};
  float mCached_idle{0};
};

#endif