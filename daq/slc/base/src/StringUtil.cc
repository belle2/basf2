/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/base/StringUtil.h"

#include <cstdarg>
#include <cstdio>
#include <sstream>
#include <cctype>
#include <algorithm>

using namespace Belle2;

StringList StringUtil::split(const std::string& str, const char type, size_t max)
{
  StringList str_v;
  size_t current = 0, found;
  while ((found = str.find_first_of(type, current)) != std::string::npos) {
    str_v.push_back(std::string(str, current, found - current));
    current = found + 1;
  }
  if (str.size() - current > 0) {
    str_v.push_back(std::string(str, current, str.size() - current));
  }
  while (max > 0 && str_v.size() < max) {
    str_v.push_back("");
  }
  return str_v;
}

std::string StringUtil::join(StringList str_v, const std::string& s, size_t start, size_t end)
{
  std::stringstream ss;
  for (size_t i = start; i < str_v.size();) {
    ss << str_v[i];
    i++;
    if ((end > 0 && i == end) || i == str_v.size()) break;
    ss << s;
  }
  return ss.str();
}

std::string StringUtil::replace(const std::string& source,
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

std::string StringUtil::form(const std::string& str, ...)
{
  va_list ap;
  static __thread char ss[1024 * 10];
  va_start(ap, str);
  vsnprintf(ss, sizeof(ss), str.c_str(), ap);
  va_end(ap);
  return ss;
}

std::string StringUtil::toupper(const std::string& str)
{
  std::string s = str;
  transform(s.begin(), s.end(), s.begin(), ::toupper);
  return s;
}

std::string StringUtil::tolower(const std::string& str)
{
  std::string s = str;
  transform(s.begin(), s.end(), s.begin(), ::tolower);
  return s;
}

int StringUtil::atoi(const std::string str)
{
  return atoi(str.c_str());
}

double StringUtil::atof(const std::string str)
{
  return atof(str.c_str());
}

long long StringUtil::atoll(const std::string str)
{
  return atoll(str.c_str());
}

bool StringUtil::find(const std::string& s, const std::string& str)
{
  return s.find(str) != std::string::npos;
}

bool StringUtil::isdigit(const std::string& s)
{
  if (s.find("0x") == 0) return true;
  for (size_t i = 0; i < s.size(); i++) {
    if (i == 0 && s.at(0) == '-') continue;
    if (!::isdigit(s.at(i))) return false;
  }
  return s.size() > 0 && s != "-";
}
