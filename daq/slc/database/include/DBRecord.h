#ifndef _Belle2_DBRecord_h
#define _Belle2_DBRecord_h

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  typedef std::map<std::string, std::string> DBFieldList;

  class DBRecord {

  public:
    DBRecord() {};
    ~DBRecord() {};

  public:
    std::vector<std::string>& getFieldNames() { return m_name_v; }
    DBFieldList& getFieldValues() { return m_value_m; }
    bool hasField(const std::string& name)
    {
      return m_value_m.find(name) != m_value_m.end();
    }
    size_t getNFields() const { return m_name_v.size(); }
    const std::string get(const std::string& name) const;
    const std::string get(int i) const;
    unsigned long long int getULLInt(const std::string& name) const;
    int getInt(const std::string& name) const;
    int getInt(int i) const;
    bool getBool(const std::string& name) const;
    bool getBool(int i) const;
    float getFloat(const std::string& name) const;
    float getFloat(int i) const;
    std::vector<std::string> getArray(const std::string& name) const;
    std::vector<std::string> getArray(int i) const;
    std::vector<int> getIntArray(const std::string& name) const;
    std::vector<int> getIntArray(int i) const;
    std::vector<float> getFloatArray(const std::string& name) const;
    std::vector<float> getFloatArray(int i) const;
    const std::string getFieldName(int i) const;
    void reset();
    void add(std::string name, std::string value);

  private:
    std::vector<std::string> m_name_v;
    mutable DBFieldList m_value_m;

  };

  typedef std::vector<DBRecord> DBRecordList;

}

#endif
