#ifndef ATP_UTILS_H_
#define ATP_UTILS_H_

#include <algorithm>
#include <cctype>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time_adjustor.hpp"

using namespace std;

template<typename T>
inline bool from_string(const string& s, T* out)
{
  std::istringstream ss(s);
  ss >> *out;
  return true;
}

// Converts the input string to all upper-case.
// Returns a transformed version without mutating input.
inline const string to_upper(const string& s)
{
  string copy(s);
  std::transform(copy.begin(), copy.end(), copy.begin(),
                 (int(*)(int)) std::toupper);
  return copy;
}

// Converts the input string to all lower-case.
// Returns a transformed version without mutating input.
inline const string to_lower(const string& s)
{
  string copy(s);
  std::transform(copy.begin(), copy.end(), copy.begin(),
                 (int(*)(int)) std::tolower);
  return copy;
}

typedef boost::uint64_t int64;

inline int64 now_micros()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return static_cast<int64>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

inline void now_micros(string* str)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  stringstream ss;
  ss << tv.tv_sec << tv.tv_usec;
  str->assign(ss.str());
}

static const boost::posix_time::ptime
utc_epoch(boost::gregorian::date(1970, 1, 1));

static const boost::posix_time::time_facet
iso_time_facet("%Y-%m-%d %H:%M:%S%F%Q");

inline boost::posix_time::time_duration utc_micros()
{
  using namespace boost::posix_time;
  return microsec_clock::universal_time() - utc_epoch;
}


using namespace boost::posix_time;
using namespace boost::gregorian;

//eastern timezone is utc-5
typedef boost::date_time::local_adjustor<ptime, -5, us_dst> us_eastern;


#endif // ATP_UTILS_H_
