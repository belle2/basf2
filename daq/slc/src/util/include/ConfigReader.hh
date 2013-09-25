#ifndef _B2DAQ_ConfigReader_hh
#define _B2DAQ_ConfigReader_hh

#include <string>
#include <map>

#include "IOException.hh"

namespace B2DAQ {

  class ConfigReader {

  public:
    ConfigReader(const std::string& path);
    ~ConfigReader() throw() {}

  public:
    const std::string get(const std::string& label)
      throw(IOException);

  private:
    std::map<std::string, std::string> _value_m;

  };

}

#endif
