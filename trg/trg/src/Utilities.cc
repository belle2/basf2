/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#define TRG_SHORT_NAMES

#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "trg/trg/Utilities.h"

using namespace std;

namespace Belle2 {

  string
  TRGUtilities::dateString(void)
  {
    time_t t;
    time(& t);
    struct tm* tl;
    tl = localtime(& t);
    char ts1[80];
    strftime(ts1, sizeof(ts1), "%Y/%m/%d %H:%M %Z", tl);
    return (ts1);
  }

  string
  TRGUtilities::dateStringF(void)
  {
    time_t t;
    time(& t);
    struct tm* tl;
    tl = localtime(& t);
    char ts0[80];
    strftime(ts0, sizeof(ts0), "%Y%m%d_%H%M", tl);
    return string(ts0);
  }

  std::string
  TRGUtilities::itostring(int i)
  {
    // std::ostringstream s;
    // s << i;
    // return s.str();

    return std::to_string(i);
  }

  std::string
  TRGUtilities::dtostring(double d, unsigned int precision)
  {
    std::ostringstream s;
    s << std::setprecision(precision) << d;
    return s.str();
  }

  std::string
  TRGUtilities::carstring(const std::string& s)
  {
    std::string ret;
//    const char * p = str;
//    while ( *p && isspace(*p) ) p++;
//    while ( *p && !isspace(*p) ) ret += *(p++);
    int i;
    int len = s.length();
    for (i = 0; i < len; i++) {
      if (!isspace(s[i])) break;
    }
    for (; i < len; i++) {
      if (!isspace(s[i])) {
        ret += s[i];
      } else break;
    }
    return ret;
  }

  std::string
  TRGUtilities::cdrstring(const std::string& s)
  {
//    const char * p = str;
//    while ( *p && isspace(*p) ) p++;
//    while ( *p && !isspace(*p) ) p++;
//    while ( *p && isspace(*p) ) p++;
    int i;
    int len = s.length();
    for (i = 0; i < len; i++) {
      if (!isspace(s[i])) break;
    }
    for (; i < len; i++) {
      if (isspace(s[i])) break;
    }
    for (; i < len; i++) {
      if (!isspace(s[i])) break;
    }
    return s.substr(i);
  }

  void
  TRGUtilities::bitDisplay(unsigned val)
  {
    bitDisplay(val, 31, 0);
  }

  void
  TRGUtilities::bitDisplay(unsigned val, unsigned f, unsigned l)
  {
    unsigned i;
    for (i = 0; i < f - l; i++) {
      if ((i % 8) == 0) cout << " ";
      cout << (val >> (f - i)) % 2;
    }
  }

  string
  TRGUtilities::streamDisplay(unsigned val, unsigned f, unsigned l)
  {
    string s;
    for (unsigned i = f; i < l + 1; i++) {
      if ((i % 8) == 0) s += " ";
      s += itostring((val >> i) % 2);
    }
    return s;
  }

  string
  TRGUtilities::streamDisplay(unsigned val)
  {
    return streamDisplay(val, 0, 63);
  }

} // namespace Belle2

