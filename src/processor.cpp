#include "processor.h"

#include "linux_parser.h"

using std::stof;

// Return the aggregate CPU utilization
float Processor::Utilization() {
  float idle_ticks = LinuxParser::IdleJiffies();
  float active_ticks = LinuxParser::ActiveJiffies();
  // compute delta
  idle_ticks -= mCached_idle;
  active_ticks -= mCached_active;
  // cached current idle, active
  mCached_idle = idle_ticks;
  mCached_active = active_ticks;
  // cpu utilization
  return active_ticks / (idle_ticks + active_ticks);
}