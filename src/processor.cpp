#include "processor.h"

#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
  float utilization{0.0f};
  long active_ticks = LinuxParser::ActiveJiffies();
  long idle_ticks = LinuxParser::IdleJiffies();
  long duration_active(active_ticks - mCached_active_ticks);
  long duration_idle(idle_ticks - mCached_idle_ticks);
  utilization = duration_active / (duration_active + duration_idle);
  // update cached ticks
  mCached_active_ticks = active_ticks;
  mCached_idle_ticks = idle_ticks;
  return utilization;
}