#include "linux_parser.h"

// #include <dirent.h>
#include <unistd.h>

#include <filesystem>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;
namespace fs = std::filesystem;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;

  fs::path directory{kProcDirectory};
  for (auto itEntry = fs::recursive_directory_iterator(directory);
       itEntry != fs::recursive_directory_iterator(); ++itEntry) {
    // Is this a directory?
    if (itEntry->is_directory()) {
      // Is every character of the name a digit?
      string filename(itEntry->path().filename().string());
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line, key, value, kb;
  float memTotal{0.0f}, memFree{0.0f}, memory_utilization{0.0f};

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value >> kb) {
        // parse total memory in kb
        if (key == "MemTotal:") {
          memTotal = stof(value);
        }
        // parse free memory in kb
        if (key == "MemFree:") {
          memFree = stof(value);
        }
        // calculate memory utilization
        if (memTotal > 0.0f && memFree > 0.0f) {
          memory_utilization = 100 * (memTotal - memFree) / memTotal;
          break;
        }
      }
    }
  }
  return memory_utilization;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  float upTime, idleTime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> upTime >> idleTime;
  }
  return static_cast<long>(upTime);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid [[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { return {}; }

int LinuxParser::TotalProcesses() {
  string line, key;
  int total_procs{0};
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      // line starts with specified substring for the total number of processes
      if (line.rfind("processes", 0) == 0) {
        std::istringstream linestream(line);
        // read the total number then return it
        while (linestream >> key >> total_procs) {
          return total_procs;
        }
      }
    }
  }
  return total_procs;
}

int LinuxParser::RunningProcesses() {
  string line, key;
  int running_procs{0};
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      // line starts with specified substring for the total number of processes
      if (line.rfind("procs_running", 0) == 0) {
        std::istringstream linestream(line);
        // read the total number then return it
        while (linestream >> key >> running_procs) {
          return running_procs;
        }
      }
    }
  }
  return running_procs;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid [[maybe_unused]]) { return 0; }