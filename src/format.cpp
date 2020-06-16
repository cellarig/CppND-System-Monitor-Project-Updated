#include "format.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::ostringstream;
using std::setfill;
using std::setw;
using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  ostringstream oss;
  long int h, m, s;
  h = std::abs(seconds / (60 * 60));
  m = std::abs(seconds % (60 * 60) / 60);
  s = std::abs(seconds % (60 * 60) % 60);
  oss << setfill('0') << setw(2) << h << ':' << setfill('0') << setw(2) << m
      << ':' << setfill('0') << std::setw(2) << s;
  return oss.str();
}