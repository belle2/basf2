/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_ConfigFile_h
#define _Belle2_ConfigFile_h

#include <string>
#include <map>
#include <vector>
#include <istream>

namespace Belle2 {

  class ConfigFile {
  public:
    typedef std::map<std::string, std::string> ValueList;

  public:
    ConfigFile() {}
    ConfigFile(const std::string& file1)
    {
      read(file1, true);
    }
    ConfigFile(const std::string& file1, const std::string& file2)
    {
      read(file1, true);
      read(file2, true);
    }
    ConfigFile(const std::string& file1,
               const std::string& file2,
               const std::string& file3)
    {
      read(file1, true);
      read(file2, true);
      read(file3, true);
    }
    ConfigFile(std::istream& is)
    {
      read(is);
    }
    ~ConfigFile() {}

  public:
    void clear();
    void read(const std::string& filename, bool overload = true);
    void read(std::istream& is, bool overload = true);
    bool hasKey(const std::string& label);
    const std::string get(const std::string& label);
    int getInt(const std::string& label);
    bool getBool(const std::string& label);
    double getFloat(const std::string& label);
    ValueList& getValues() { return m_value_m; }
    std::vector<std::string>& getLabels() { return m_label_v; }
    void add(const std::string& label,
             const std::string& value, bool overload);
    void write(const std::string& path);
    void print();

  private:
    const std::string getFilePath(const std::string& filename);

  private:
    ValueList m_value_m;
    std::vector<std::string> m_label_v;
    std::string m_dir;
    std::map<std::string, int> m_count;

  };

  inline bool ConfigFile::hasKey(const std::string& label)
  {
    return m_value_m.find((m_dir.size() > 0) ? m_dir + "." + label : label)
           != m_value_m.end();
  }

};

#endif
