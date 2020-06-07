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
  long s{seconds};
  // substract hours
  long h = s / (60 * 60);
  s -= h * (60 * 60);
  // substract minutes
  long m = s / 60;
  s -= m * 60;
  oss << setfill('0') << setw(2) << h << ':' << setw(2) << m << ':'
      << std::setw(2) << s;
  return oss.str();
}