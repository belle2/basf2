#include "daq/slc/database/FieldInfoTable.h"

#include "daq/slc/database/TableInfoTable.h"
#include "daq/slc/database/DBObject.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <sstream>
#include <iostream>
#include <cstdlib>

using namespace Belle2;

FieldInfoList FieldInfoTable::getList(const std::string tablename,
                                      int revision)
{
  FieldInfoList info_v;
  if (m_db != NULL) {
    try {
      if (tablename.size() > 0) {
        m_db->execute("select id, name, \"table\", revision, "
                      "type, length from fieldnames('%s', %d);",
                      tablename.c_str(), revision);
        /*
              m_db->execute("select fieldinfo.record_time as record_time, "
                "fieldinfo.id as id, tableinfo.name as \"table\", "
                "tableinfo.revision as revision, fieldinfo.name as name,"
                "fieldinfo.type as type, fieldinfo.length as length "
                "from fieldinfo, tableinfo where fieldinfo.tableid = tableinfo.id "
                "and tableinfo.name = '%s' and tableinfo.revision = %d;",
                            tablename.c_str(), revision);
        */
      } else {
        m_db->execute("select id, name, \"table\", revision, "
                      "type, length from fieldnames();");
        /*
        m_db->execute("select fieldinfo.record_time as record_time, "
                "fieldinfo.id as id, tableinfo.name as \"table\", "
                "tableinfo.revision as revision, fieldinfo.name as name, "
                "fieldinfo.type as type, fieldinfo.length as length "
                "from fieldinfo, tableinfo where fieldinfo.tableid = tableinfo.id");
        */
      }
      DBRecordList record_v = m_db->loadRecords();
      for (DBRecordList::iterator it = record_v.begin();
           it != record_v.end(); it++) {
        DBRecord& record(*it);
        FieldInfo::Type type = (FieldInfo::Type)record.getInt("type");
        std::string name = record.get("name");
        if (type >= FieldInfo::BOOL) {
          info_v.push_back(FieldInfo(name, type, record.get("table"),
                                     record.getInt("revison"),
                                     record.getInt("length"),
                                     record.getInt("id")));
        }
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return info_v;
}

int FieldInfoTable::add(const FieldInfo& info)
{
  if (m_db != NULL) {
    try {
      m_db->execute("select addfield as id from addfield"
                    "('%s', %d, '%s', %d, %d);",
                    info.getName().c_str(), (int)info.getType(),
                    info.getTable().c_str(), info.getRevision(),
                    info.getLength());
      DBRecordList record_v(m_db->loadRecords());
      if (record_v.size() > 0) return record_v[0].getInt("id");
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return 0;
}

int FieldInfoTable::createTable(const DBObject& obj, bool isroot)
{
  FieldInfoList info_v;
  const FieldNameList name_v(obj.getFieldNames());
  for (size_t i = 0; i < name_v.size(); i++) {
    FieldInfo::Property pro(obj.getProperty(name_v[i]));
    FieldInfo info(name_v[i], pro.getType(),
                   obj.getTable(), obj.getRevision(),
                   pro.getLength());
    info_v.push_back(info);
  }
  int id = 0;
  if (m_db != NULL) {
    try {
      std::string title = (obj.isConfig()) ? "config" : "logger";
      std::string tablename_in =
        StringUtil::form("%sinfo:%s:%d", title.c_str(),
                         obj.getTable().c_str(), obj.getRevision());
      if (m_db->checkTable(tablename_in)) return 0;
      std::stringstream ss;
      ss << "create table \"" << tablename_in << "\" (";
      std::string idname = title + "id";
      ss << idname << " int";
      if (!isroot) ss << ", index int";
      for (FieldInfoList::iterator it = info_v.begin(); it != info_v.end(); it++) {
        FieldInfo& info(*it);
        if (info.getType() != FieldInfo::NSM_OBJECT &&
            info.getType() != FieldInfo::OBJECT && info.getLength() > 0) {
          const std::string ctablename_in =
            StringUtil::form("\"%sinfo:%s.%s:%d\"", title.c_str(),
                             obj.getTable().c_str(), info.getName().c_str(),
                             obj.getRevision());
          m_db->execute("create table %s (%s int, index int, \"%s\" %s);",
                        ctablename_in.c_str(), idname.c_str(),
                        info.getName().c_str(),
                        info.getTypeAlias().c_str());
          ss << ", \"" << info.getName() << "\" int ";
          //"references " << ctablename_in << "(" << idname << ")";
        } else {
          ss << ", \"" << info.getName() << "\" "
             << " " << info.getTypeAlias();
        }
      }
      if (!isroot) ss << ", unique (" << idname << ", index))";
      else ss << ")";
      m_db->execute("select create_if_not_exists('%s', '%s');",
                    tablename_in.c_str(), ss.str().c_str());
      TableInfoTable(m_db).add(TableInfo(obj.getTable(), obj.isConfig(),
                                         obj.getRevision(), isroot));
      for (FieldInfoList::iterator it = info_v.begin(); it != info_v.end(); it++) {
        FieldInfo& info(*it);
        add(info);
      }
      return id;
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return 0;
}

