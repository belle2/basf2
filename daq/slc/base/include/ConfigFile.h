#ifndef _Belle2_ConfigFile_h
#define _Belle2_ConfigFile_h

#include <daq/slc/base/Debugger.h>

#include <string>
#include <map>

namespace Belle2 {

  class ConfigFile {

  public:
    ConfigFile() {}
    ConfigFile(const std::string& file1) {
      read(file1, true);
    }
    ConfigFile(const std::string& file1, const std::string& file2) {
      read(file1, true);
      read(file2, true);
    }
    ConfigFile(const std::string& file1, const char* file2) {
      read(file1, true);
      read(file2, true);
    }
    ConfigFile(const std::string& file1, bool overload) {
      read(file1, overload);
    }
    ~ConfigFile() {}

  public:
    void clear();
    void read(const std::string& filename, bool overload = true);
    const std::string get(const std::string& label);
    int getInt(const std::string& label);
    double getDouble(const std::string& label);
    void cd() { __base = ""; }
    void cd(const std::string& dir) { __base += dir + "."; }

  private:
    static std::map<std::string, std::string> __value_m;
    std::string __base;

  };

};

#endif
