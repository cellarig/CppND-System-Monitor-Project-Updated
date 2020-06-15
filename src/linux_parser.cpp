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
  float memTotal{0.0f}, memFree{0.0f}, buffers{0.0f};
  float cached{0.0f}, sreclaimable{0.0f}, shmem{0.0f};

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value >> unit) {
        if (key == "MemTotal:" && unit == "kB") {
          memTotal = stof(value);
        } else if (key == "MemFree:" && unit == "kB") {
          memFree = stof(value);
        } else if (key == "Buffers:" && unit == "kB") {
          buffers = stof(value);
        } else if (key == "Cached:" && unit == "kB") {
          cached = stof(value);
        } else if (key == "SReclaimable:" && unit == "kB") {
          sreclaimable = stof(value);
        } else if (key == "Shmem:" && unit == "kB") {
          shmem = stof(value);
        }

        // calculate memory utilization
        if (memTotal > 0.0f) {
          float total_used = memTotal - memFree;
          float cached_mem = cached + sreclaimable - shmem;
          // green bars like in htop
          return (total_used - (buffers + cached_mem)) / memTotal;
        }
      }
    }
  }
  return 0.0f;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string upTime, idleTime;
  string line;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> upTime >> idleTime;
  }
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
  //   #14    #15    #16     #17
  long utime, stime, cutime, cstime;
  size_t pos{1};
  std::ifstream filestream(LinuxParser::kProcDirectory + to_string(pid) +
                           LinuxParser::kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (pos <= 17) {
      linestream >> value;
      if (pos == 14) utime = stol(value);
      if (pos == 15) stime = stol(value);
      if (pos == 16) cutime = stol(value);
      if (pos == 17) cstime = stol(value);
      pos++;
    }
  }
  float seconds = (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);
  return static_cast<long>(seconds);
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto cpu_utilization_str = CpuUtilization();
  // convert all values to long
  std::vector<long> cpu_utilization = {};
  for (std::string &value : cpu_utilization_str) {
    cpu_utilization.emplace_back(stof(value));
  }

  long user_time = cpu_utilization[kUser_] - cpu_utilization[kGuest_];
  long nice_time = cpu_utilization[kNice_] - cpu_utilization[kGuestNice_];
  long system_time = cpu_utilization[kSystem_] + cpu_utilization[kIRQ_] +
                     cpu_utilization[kSoftIRQ_];
  long virt_time = cpu_utilization[kGuest_] + cpu_utilization[kGuestNice_];
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
    linestream >> key >> cpu_utilization[kUser_] >> cpu_utilization[kNice_] >>
        cpu_utilization[kSystem_] >> cpu_utilization[kIdle_] >>
        cpu_utilization[kIOwait_] >> cpu_utilization[kIRQ_] >>
        cpu_utilization[kSoftIRQ_] >> cpu_utilization[kSteal_] >>
        cpu_utilization[kGuest_] >> cpu_utilization[kGuestNice_];
  }
  return cpu_utilization;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, key, value;
  int total_procs{0};
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
  int running_procs{0};
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
  string line{};
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
          float ram_mb = stof(value) / 1024.0f;
          return to_string(ram_mb);
        }
      }
    }
  }
  return string();
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
  size_t pos{1};
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (pos <= 22) {  // starttime #22
      linestream >> value;
      pos++;
    }
  }
  long seconds = stol(value) / sysconf(_SC_CLK_TCK);
  return LinuxParser::UpTime() - seconds;
}