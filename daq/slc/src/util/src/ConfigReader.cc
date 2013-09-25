#include "ConfigReader.hh"

#include "StringUtil.hh"

#include <fstream>

using namespace B2DAQ;

ConfigReader::ConfigReader(const std::string& path)
{
  std::ifstream fin(path.c_str());
  std::string buf;
  while (fin && getline(fin, buf)) {
    if (buf.at(0) == '#') continue;
    std::vector<std::string> str_v = B2DAQ::split(buf, '=');
    if (str_v.size() >= 2) {
      _value_m.insert(std::map<std::string, std::string>::value_type(str_v[0], str_v[1]));
    }
  }
}

const std::string ConfigReader::get(const std::string& label) throw(IOException)
{
  if (_value_m.find(label) == _value_m.end()) {
    throw (IOException(__FILE__, __LINE__, B2DAQ::form("No entry:%s", label.c_str())));
  }
  return _value_m[label];
}
