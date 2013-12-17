#ifndef _Belle2_ConfigFile_h
#define _Belle2_ConfigFile_h

#include <string>
#include <map>

namespace Belle2 {

  class ConfigFile {

  public:
    ConfigFile(const std::string& file1 = "",
               bool overload = true) {
      if (file1.size() > 0) read(file1, overload);
    }
    ConfigFile(const std::string& file1,
               const std::string& file2,
               bool overload = true) {
      if (file1.size() > 0) read(file1, overload);
      if (file2.size() > 0) read(file2, overload);
    }
    ConfigFile(const std::string& file1,
               const std::string& file2,
               const std::string& file3,
               bool overload = true) {
      if (file1.size() > 0) read(file1, overload);
      if (file2.size() > 0) read(file2, overload);
      if (file3.size() > 0) read(file3, overload);
    }
    ~ConfigFile() {}

  public:
    void read(const std::string& filename, bool overload = true);
    const std::string get(const std::string& label);
    int getInt(const std::string& label);
    double getDouble(const std::string& label);

  private:
    static std::map<std::string, std::string> __value_m;

  };

};

#endif
