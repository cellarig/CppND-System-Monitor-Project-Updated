#include "system.h"

#include <unistd.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// Return a container composed of the system's processes
vector<Process>& System::Processes() {
  // read current existing pids in the system
  std::vector<int> pids{LinuxParser::Pids()};

  // create set of existing pids (unique id)
  set<int> existing_pids;
  for (auto const& process : processes_) {
    existing_pids.insert(process.Pid());
  }

  // add new processes
  for (int pid : pids) {
    // only non existing in the processes vector
    if (existing_pids.find(pid) == existing_pids.end()) {
      processes_.emplace_back(Process(pid));
    }
  }

  std::sort(processes_.begin(), processes_.end());

  return processes_;
}

// Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

// Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }