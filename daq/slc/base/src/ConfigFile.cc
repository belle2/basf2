#include "daq/slc/base/ConfigFile.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <cstdlib>
#include <fstream>

using namespace Belle2;

std::map<std::string, std::string> ConfigFile::__value_m;

void ConfigFile::read(const std::string& filename)
{
  const char* path = getenv("BELLE2_LOCAL_DIR");
  if (path == NULL) {
    Belle2::debug("[ERROR] Enveriment varialble : BELLE_LOCAL_DIR");
    exit(1);
  }
  std::string file_path = path;
  file_path += "/daq/slc/data/config/" + filename + ".conf";
  std::ifstream fin(file_path.c_str());
  std::string s;
  while (fin && getline(fin, s)) {
    if (s.size() == 0 || s.at(0) == '#') continue;
    std::vector<std::string> str_v = Belle2::split(s, ':');
    if (str_v.size() >= 2) {
      std::string label = Belle2::replace(Belle2::replace(str_v[0], " ", ""), "\t", "");
      std::string value = "";
      size_t i = 0;
      for (; i < str_v[1].size(); i++) {
        if (str_v[1].at(i) == '#' || str_v[1].at(i) == '\n') break;
        if (str_v[1].at(i) == ' ' || str_v[1].at(i) == '\t') continue;
        if (str_v[1].at(i) == '"') {
          for (i++ ; i < str_v[1].size(); i++) {
            if (str_v[1].at(i) == '"') break;
            value += str_v[1].at(i);
          }
          break;
        }
        if (str_v[1].at(i) == '$') {
          i++;
          if (str_v[1].at(i) == '{') {
            for (i++ ; i < str_v[1].size(); i++) {
              if (str_v[1].at(i) == '}') break;
              value += str_v[1].at(i);
            }
          }
          const char* env = getenv(value.c_str());
          if (env != NULL) {
            value = env;
          } else if (__value_m.find(value) != __value_m.end()) {
            value = __value_m[value];
          }
          continue;
        }
        value += str_v[1].at(i);
      }
      if (__value_m.find(label) == __value_m.end()) {
        __value_m.insert(std::map<std::string, std::string>::value_type(label, value));
      } else {
        Belle2::debug("%s %s %s", filename.c_str(), label.c_str(), value.c_str());
        __value_m[label] = value;
      }
    }
  }
  fin.close();
}

const std::string ConfigFile::get(const std::string& label)
{
  if (__value_m.find(label) != __value_m.end())
    return __value_m[label];
  else
    return "";
}

int ConfigFile::getInt(const std::string& label)
{
  if (__value_m.find(label) != __value_m.end())
    return atoi(__value_m[label].c_str());
  else
    return 0;
}

double ConfigFile::getDouble(const std::string& label)
{
  if (__value_m.find(label) != __value_m.end())
    return atof(__value_m[label].c_str());
  else
    return 0;
}
