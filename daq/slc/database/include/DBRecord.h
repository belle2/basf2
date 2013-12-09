#ifndef _Belle2_DBRecord_h
#define _Belle2_DBRecord_h

#include <string>
#include <vector>

namespace Belle2 {

  class DBRecord {

  public:
    DBRecord() throw() {};
    ~DBRecord() throw() {};

  public:
    std::vector<std::string>& getFieldNames() throw() { return _name_v; }
    std::vector<std::string>& getFieldValues() throw() { return _value_v; }
    size_t getNFields() const throw() { return _name_v.size(); }
    const std::string getFieldValue(std::string name) const throw();
    const std::string getFieldValue(int i) const throw();
    int getFieldValueInt(std::string name) const throw();
    int getFieldValueInt(int i) const throw();
    std::vector<std::string> getFieldValueArray(std::string name) const throw();
    std::vector<std::string> getFieldValueArray(int i) const throw();
    std::vector<int> getFieldValueIntArray(std::string name) const throw();
    std::vector<int> getFieldValueIntArray(int i) const throw();
    const std::string getFieldName(int i) const throw();
    void reset();
    void addField(std::string name, std::string value) throw();

  private:
    std::vector<std::string> _name_v;
    std::vector<std::string> _value_v;

  };

  typedef std::vector<DBRecord> DBRecordList;

}

#endif
