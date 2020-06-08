#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : mPid(pid) {}

int Process::Pid() const { return mPid; }

// Return this process's CPU utilization
float Process::CpuUtilization() {
  return std::stof(LinuxParser::CpuUtilization()[mPid]);
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(mPid); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(mPid); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(mPid); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(mPid); }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a [[maybe_unused]]) const {
  return true;
}