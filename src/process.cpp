#include "process.h"

#include <unistd.h>

#include <cctype>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : mPid(pid) {}

int Process::Pid() const { return mPid; }

// Return this process's CPU utilization
float Process::CpuUtilization() const {
  float total_time = LinuxParser::ActiveJiffies(mPid) / sysconf(_SC_CLK_TCK);
  return total_time / LinuxParser::UpTime(mPid);
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(mPid); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(mPid); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(mPid); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(mPid); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return this->CpuUtilization() > a.CpuUtilization();
}