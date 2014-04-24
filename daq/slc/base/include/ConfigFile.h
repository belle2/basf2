#ifndef _Belle2_ConfigFile_h
#define _Belle2_ConfigFile_h

#include <string>
#include <map>
#include <vector>

namespace Belle2 {

  class ConfigFile {
  public:
    typedef std::map<std::string, std::string> ValueList;

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
    ~ConfigFile() {}

  public:
    void clear();
    void read(const std::string& filename, bool overload = true);
    bool hasKey(const std::string& label);
    const std::string get(const std::string& label);
    int getInt(const std::string& label);
    double getFloat(const std::string& label);
    void cd() { _dir = ""; }
    void cd(const std::string& dir);
    ValueList& getValues() { return _value_m; }
    std::vector<std::string>& getLabels() { return _label_v; }
    std::vector<std::string>& getDirectories() { return _dir_v; }
    void add(const std::string& label, const std::string& value);
    void write(const std::string& path);

  private:
    const std::string getFilePath(const std::string& filename);

  private:
    ValueList _value_m;
    std::vector<std::string> _dir_v;
    std::vector<std::string> _label_v;
    std::string _dir;

  };

  inline bool ConfigFile::hasKey(const std::string& label)
  {
    return _value_m.find((_dir.size() > 0) ? _dir + "." + label : label) != _value_m.end();
  }

  inline void ConfigFile::cd(const std::string& dir)
  {
    _dir = (_dir.size() > 0) ? _dir + "." + dir : dir;
  }

};

#endif
