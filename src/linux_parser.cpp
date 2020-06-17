#include "linux_parser.h"

// #include <dirent.h>
#include <unistd.h>

#include <filesystem>
#include <string>
#include <vector>

using std::stof;
using std::stol;
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
  std::ifstream filestream(kProcDirectory + kVersionFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel;
  }
  return kernel;
}

// REVIEW BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;

  fs::path directory{kProcDirectory};
  for (auto itEntry = fs::directory_iterator(directory);
       itEntry != fs::directory_iterator(); ++itEntry) {
    // Is this a directory?
    if (itEntry->is_directory()) {
      // Is every character of the name a digit?
      string filename(itEntry->path().filename().string());
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.emplace_back(pid);
      }
    }
  }
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line, key, value, unit;
  float memTotal, memFree, buffers;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value >> unit) {
        if (key == "MemTotal:") {
          memTotal = stof(value);
        } else if (key == "MemFree:") {
          memFree = stof(value);
        } else if (key == "Buffers:") {
          buffers = stof(value);
        }
      }
    }
  }
  return 1.0f - (memFree / (memTotal - buffers));
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string upTime, idleTime, line;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> upTime >> idleTime;
  }
  // in seconds
  return stol(upTime);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long jiffies{0};
  auto cpu_utilization = LinuxParser::CpuUtilization();
  for (size_t i = kUser_; i < kSteal_; i++) {
    jiffies += stol(cpu_utilization[i]);
  }
  return jiffies;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line, value;
  long utime, stime, cutime, cstime;
  size_t pos{0};
  std::ifstream filestream(LinuxParser::kProcDirectory + to_string(pid) +
                           LinuxParser::kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      if (pos == 13) utime = stol(value);
      if (pos == 14) stime = stol(value);
      if (pos == 15) cutime = stol(value);
      if (pos == 16) cstime = stol(value);
      pos++;
    }
  }
  // in clock ticks
  return utime + stime + cutime + cstime;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long user_time, nice_time, system_time, virt_time;
  auto cpu_utilization_str = CpuUtilization();
  // convert all values to long
  std::vector<long> cpu_utilization = {};
  for (std::string &value : cpu_utilization_str) {
    cpu_utilization.emplace_back(stof(value));
  }

  user_time = cpu_utilization[kUser_] - cpu_utilization[kGuest_];
  nice_time = cpu_utilization[kNice_] - cpu_utilization[kGuestNice_];
  system_time = cpu_utilization[kSystem_] + cpu_utilization[kIRQ_] +
                cpu_utilization[kSoftIRQ_];
  virt_time = cpu_utilization[kGuest_] + cpu_utilization[kGuestNice_];
  return user_time + nice_time + system_time + virt_time +
         cpu_utilization[kSteal_];
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto cpu_utilization = LinuxParser::CpuUtilization();
  return stol(cpu_utilization[kIdle_]) + stol(cpu_utilization[kIOwait_]);
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line, key;
  vector<string> cpu_utilization{10, ""};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> key;
    for (size_t i = kUser_; i <= kGuestNice_; i++) {
      linestream >> cpu_utilization[i];
    }
  }
  return cpu_utilization;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, key, value;
  int total_procs = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes" && !value.empty()) {
          total_procs = stoi(value);
          return total_procs;
        }
      }
    }
  }
  return total_procs;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line, key;
  int running_procs = 0;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
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

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
  }
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line, key, value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          long mb = (stof(value) / 1024);
          return to_string(mb);
        }
      }
    }
  }
  return string{"0"};
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line, key;
  int uid;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      if (line.rfind("Uid:", 0) == 0) {
        std::istringstream linestream(line);
        while (linestream >> key >> uid) {
          return to_string(uid);
        }
      }
    }
  }
  return string();
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line;
  // get string uid
  string strUid = LinuxParser::Uid(pid);
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open() && !strUid.empty()) {
    while (getline(filestream, line)) {
      if (line.find(strUid) != string::npos) {
        return line.substr(0, line.find_first_of(':'));
      }
    }
  }
  return string();
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, value;
  float starttime;
  long seconds;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (size_t i = 0; i < 21; ++i) {
      linestream >> value;
    }
    // #22 starttime in clock ticks
    linestream >> starttime;
  }
  seconds = starttime / sysconf(_SC_CLK_TCK);
  return LinuxParser::UpTime() - seconds;
}