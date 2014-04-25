#ifndef _Belle2_DBRecord_h
#define _Belle2_DBRecord_h

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  typedef std::map<std::string, std::string> DBFieldList;

  class DBRecord {

  public:
    DBRecord() throw() {};
    ~DBRecord() throw() {};

  public:
    std::vector<std::string>& getFieldNames() throw() { return m_name_v; }
    DBFieldList& getFieldValues() throw() { return m_value_m; }
    size_t getNFields() const throw() { return m_name_v.size(); }
    const std::string get(const std::string& name) const throw();
    const std::string get(int i) const throw();
    int getInt(const std::string& name) const throw();
    int getInt(int i) const throw();
    bool getBool(const std::string& name) const throw();
    bool getBool(int i) const throw();
    float getFloat(const std::string& name) const throw();
    float getFloat(int i) const throw();
    std::vector<std::string> getArray(const std::string& name) const throw();
    std::vector<std::string> getArray(int i) const throw();
    std::vector<int> getIntArray(const std::string& name) const throw();
    std::vector<int> getIntArray(int i) const throw();
    std::vector<float> getFloatArray(const std::string& name) const throw();
    std::vector<float> getFloatArray(int i) const throw();
    const std::string getFieldName(int i) const throw();
    void reset();
    void add(std::string name, std::string value) throw();

  private:
    std::vector<std::string> m_name_v;
    mutable DBFieldList m_value_m;

  };

  typedef std::vector<DBRecord> DBRecordList;

}

#endif
