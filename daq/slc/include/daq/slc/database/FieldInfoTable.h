#ifndef _Belle2_FieldInfoTable_h
#define _Belle2_FieldInfoTable_h

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/FieldInfo.h>

#include <vector>
#include <map>
#include <string>

namespace Belle2 {

  class DBObject;

  typedef std::map<std::string, int> EnumList;
  typedef std::vector<FieldInfo> FieldInfoList;
  typedef std::map<std::string, FieldInfoList> FieldInfoListMap;

  class FieldInfoTable {

  public:
    FieldInfoTable(DBInterface* db = NULL) : m_db(db) {}
    ~FieldInfoTable() throw() {}

  public:
    FieldInfoList getList(const std::string tablename = "",
                          int revision = 0);
    int add(const FieldInfo& info);
    int createTable(const DBObject& obj, bool isroot);
    EnumList getEnums(const FieldInfo& info) throw(DBHandlerException);

  private:
    DBInterface* m_db;

  };

};

#endif
