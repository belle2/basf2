#include "daq/slc/base/ConfigFile.h"

#include "daq/slc/base/StringUtil.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace Belle2;

const std::string ConfigFile::getFilePath(const std::string& filename)
{
  std::string file_path;
  if (filename.at(0) != '/') {
    const char* path = getenv("BELLE2_LOCAL_DIR");
    if (path == NULL) {
      std::cerr << "[ERROR] Enveriment varialble : BELLE_LOCAL_DIR" << std::endl;
      exit(1);
    }
    file_path = path;
    if (filename.find("/") == std::string::npos) {
      file_path += "/daq/slc/data/config/" + filename + ".conf";
    } else {
      file_path += "/daq/slc/data/" + filename + ".conf";
    }
  } else {
    file_path = filename;
  }
  return file_path;
}

void ConfigFile::read(const std::string& filename, bool overload)
{
  if (filename.size() == 0) return;
  std::ifstream fin(getFilePath(filename).c_str());
  std::string s;
  std::string dir = "";
  while (fin && getline(fin, s)) {
    if (s.size() == 0 || s.at(0) == '#') continue;
    std::vector<std::string> str_v = StringUtil::split(s, ':');
    if (str_v.size() >= 2) {
      std::string label = StringUtil::replace(StringUtil::replace(str_v[0],
                                                                  " ", ""), "\t", "");
      if (str_v.size() > 2) {
        for (size_t i = 2; i < str_v.size(); i++) {
          str_v[1].append(":");
          str_v[1].append(str_v[i]);
        }
      }
      std::string value = "";
      size_t i = 0;
      std::stringstream ss;
      for (; i < str_v[1].size(); i++) {
        if (str_v[1].at(i) == '#' || str_v[1].at(i) == '\n') break;
        if (str_v[1].at(i) == ' ' || str_v[1].at(i) == '\t') continue;
        if (str_v[1].at(i) == '"') {
          for (i++ ; i < str_v[1].size(); i++) {
            if (str_v[1].at(i) == '"') break;
            ss << str_v[1].at(i);
          }
          break;
        }
        if (str_v[1].at(i) == '$') {
          i++;
          if (str_v[1].at(i) == '{') {
            for (i++ ; i < str_v[1].size(); i++) {
              if (str_v[1].at(i) == '}') break;
              ss << str_v[1].at(i);
            }
          }
          std::string tmp = ss.str();
          const char* env = getenv(tmp.c_str());
          ss.str("");
          if (env != NULL) {
            ss << env;
          } else if (m_value_m.find(tmp) != m_value_m.end()) {
            ss << m_value_m[tmp];
          }
          continue;
        }
        ss << str_v[1].at(i);
      }
      add(label, ss.str(), overload);
    }
  }
  fin.close();
}

void ConfigFile::clear()
{
  m_value_m.clear();
}

const std::string ConfigFile::get(const std::string& label)
{
  if (m_value_m.find(label) != m_value_m.end()) {
    return m_value_m[label];
  }
  return "";
}

int ConfigFile::getInt(const std::string& label)
{
  std::string value = get(label);
  if (value.size() > 0) return atoi(value.c_str());
  else return 0;
}

double ConfigFile::getFloat(const std::string& label)
{
  std::string value = get(label);
  if (value.size() > 0) return atof(value.c_str());
  else return 0;
}

void ConfigFile::add(const std::string& label,
                     const std::string& value, bool overload)
{
  if (m_value_m.find(label) == m_value_m.end()) {
    m_value_m.insert(ValueList::value_type(label, value));
    m_label_v.push_back(label);
  } else if (overload) {
    m_value_m[label] = value;
  }
}

void ConfigFile::write(const std::string& path)
{
  std::stringstream ss;
  ss << "#" << std::endl
     << "#" << std::endl
     << "#" << std::endl
     << "" << std::endl;
  for (std::vector<std::string>::iterator it = m_label_v.begin();
       it != m_label_v.end(); it++) {
    std::string& label(*it);
    std::string& value(m_value_m[label]);
    ss << label << " : " << value << std::endl;
  }
  ss << "" << std::endl
     << "#" << std::endl
     << "#" << std::endl
     << "#" << std::endl;
  std::ofstream fout(getFilePath(path).c_str());
  fout << ss.str();
}
