#ifndef _Belle2_ConfigFile_h
#define _Belle2_ConfigFile_h

#include <string>
#include <map>

namespace Belle2 {

  class ConfigFile {

  public:
    ConfigFile() {}
    ConfigFile(const std::string& filename) {
      read(filename);
    }
    ~ConfigFile() {}

  public:
    void read(const std::string& filename);
    const std::string get(const std::string& label);
    int getInt(const std::string& label);
    double getDouble(const std::string& label);

  private:
    static std::map<std::string, std::string> __value_m;

  };

};

#endif
