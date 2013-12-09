#include "daq/slc/base/StringUtil.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace Belle2 {

  StringList split(const std::string& str, const char type, size_t max)
  {
    std::stringstream ss;
    StringList str_v;
    char c;
    size_t m = 0;
    for (size_t n = 0; n < str.size(); n++) {
      c = str.at(n);
      if (c == type) {
        std::string s = ss.str();
        if (s.size() > 0) {
          while (s.at(0) == ' ' || s.at(0) == '\t' || s.at(0) == '\n') {
            s.erase(0, 1);
          }
          while (s.at(s.size() - 1) == ' ' || s.at(s.size() - 1) == '\t' || s.at(s.size() - 1) == '\n') {
            s.erase(s.size() - 1, 1);
          }
        }
        str_v.push_back(s);
        ss.str("");
        m++;
        if (max > 0 && m >= max) {
          break;
        }
      } else {
        ss << c;
      }
    }
    if (ss.str().size() > 0) {
      if (max > 0 && m >= max) str_v[m - 1] += ss.str();
      else str_v.push_back(ss.str());
    }
    while (max > 0 && str_v.size() < max) {
      str_v.push_back("");
    }
    return str_v;
  }

  std::string join(StringList str_v, const std::string& s)
  {
    std::stringstream ss;
    for (size_t i = 0; i < str_v.size() ;) {
      ss << str_v[i];
      i++;
      if (i == str_v.size()) break;
      ss << s;
    }
    return ss.str();
  }

  std::string replace(const std::string& source,
                      const std::string& pattern,
                      const std::string& placement)
  {
    std::string result;
    std::string::size_type pos_before = 0;
    std::string::size_type pos = 0;
    std::string::size_type len = pattern.size();
    while ((pos = source.find(pattern, pos)) != std::string::npos) {
      result.append(source, pos_before, pos - pos_before);
      result.append(placement);
      pos += len;
      pos_before = pos;
    }
    result.append(source, pos_before, source.size() - pos_before);
    return result;
  }

  std::string form(const std::string& str, ...)
  {
    va_list ap;
    static char ss[1024];
    va_start(ap, str);
    vsprintf(ss, str.c_str(), ap);
    va_end(ap);
    return ss;
  }

  std::string toupper(const std::string& str)
  {
    std::string s = str;
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
  }

  std::string tolower(const std::string& str)
  {
    std::string s = str;
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
  }

}
