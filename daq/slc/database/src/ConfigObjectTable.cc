#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/TableInfoTable.h>
#include <daq/slc/database/FieldInfoTable.h>
#include <daq/slc/database/ConfigInfoTable.h>
#include <daq/slc/database/NodeInfoTable.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <sstream>
#include <cstdlib>

using namespace Belle2;

ConfigObjectList ConfigObjectTable::getList(const std::string& configname,
                                            const std::string& nodename,
                                            const std::string& tablename_in,
                                            int revision)
throw()
{
  ConfigObjectList obj_v;
  if (m_db != NULL) {
    std::string tablename = tablename_in;
    if (tablename_in.size() == 0 || revision <= 0) {
      m_db->execute("select name, \"table\", revision from "
                    "confignames('%s', '%s')",
                    configname.c_str(), nodename.c_str());
      DBRecordList record_v(m_db->loadRecords());
      if (record_v.size() > 0) {
        tablename = record_v[0].get("table");
        revision = record_v[0].getInt("revision");
      }
    }
    TableInfo tinfo = TableInfoTable(m_db).get(tablename, revision);
    std::string tablename_imp =
      StringUtil::form("\"configinfo:%s:%d\"", tablename.c_str(), revision);
    FieldInfoTable ftable(m_db);
    FieldInfoList finfo_v = ftable.getList(tablename, revision);
    EnumNameList enum_m_m;
    std::stringstream ss;
    ss << "select configid" << ((!tinfo.isRoot()) ? ", index" : "");
    for (FieldInfoList::iterator it = finfo_v.begin();
         it != finfo_v.end(); it++) {
      ss << ", " << it->setSQL();
      FieldInfo::Type type = it->getType();
      if (type == FieldInfo::ENUM)  {
        enum_m_m.insert(EnumNameList::value_type(it->getName(),
                                                 ftable.getEnums(*it)));
      }
    }
    ConfigInfo cinfo(configname, nodename, tablename, revision);
    ss << StringUtil::form("from %s where configid = (%s)",
                           tablename_imp.c_str(), cinfo.getSQL().c_str());
    try {
      m_db->execute(ss.str().c_str());
      DBRecordList record_v(m_db->loadRecords());
      for (size_t i = 0; i < record_v.size(); i++) {
        DBRecord& record(record_v[i]);
        ConfigObject obj;
        obj.setName(configname);
        obj.setNode(nodename);
        obj.setTable(tablename);
        obj.setRevision(revision);
        obj.setId(record.getInt("configid"));
        if (!tinfo.isRoot()) obj.setIndex(record.getInt("index"));
        for (FieldInfoList::iterator it = finfo_v.begin();
             it != finfo_v.end(); it++) {
          it->setSQL(record, obj);
          std::string name = it->getName();
          if (it->getType() == FieldInfo::ENUM) {
            obj.addEnum(name, record.get(name), enum_m_m[name]);
          } else if (it->getType() == FieldInfo::OBJECT) {
            StringList str_v = StringUtil::split(record.get(name), ',');
            if (str_v.size() > 3) {
              obj.addObjects(name, getList(str_v[0], str_v[1], str_v[2],
                                           atoi(str_v[3].c_str())));
            } else {
              obj.addObject(name, ConfigObject());
            }
          }
        }
        obj_v.push_back(obj);
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return obj_v;
}

ConfigObject ConfigObjectTable::get(const std::string& configname,
                                    const std::string& nodename) throw()
{
  return getList(configname, nodename, "", 0)[0];
}

void ConfigObjectTable::add(const ConfigObjectList& obj_v, bool isroot) throw()
{
  if (m_db != NULL) {
    if (obj_v.size() > 0) {
      ConfigInfoTable table(m_db);
      const ConfigObject& obj(obj_v[0]);
      if (!obj.isConfig()) return;
      table.add(ConfigInfo(obj.getName(), obj.getNode(),
                           obj.getTable(), obj.getRevision()));
      try {
        for (size_t i = 0; i < obj_v.size(); i++) {
          add(obj_v[i], isroot);
        }
      } catch (const DBHandlerException& e) {
        LogFile::error("error on DB acess: %s", e.what());
      }
    }
  }
}

void ConfigObjectTable::add(const ConfigObject& obj, bool isroot)
throw(DBHandlerException)
{
  if (!obj.isConfig()) return;
  ConfigInfoTable table(m_db);
  table.add(ConfigInfo(obj.getName(), obj.getNode(),
                       obj.getTable(), obj.getRevision()));
  std::stringstream ss1, ss2;
  ss1 << "configid";
  ss2 << "(" << FieldInfo::getSQL(obj) << ")";
  if (!isroot) {
    ss1 << ", index";
    ss2 << ", " << obj.getIndex();
  }
  const FieldNameList& name_v(obj.getFieldNames());
  for (size_t i = 0; i < name_v.size(); i++) {
    ss1 << ", " << name_v[i];
    ss2 << ", " << FieldInfo::getSQL(obj, name_v[i]);
  }
  m_db->execute("insert into \"configinfo:%s:%d\" (%s) values (%s);",
                obj.getTable().c_str(), obj.getRevision(),
                ss1.str().c_str(), ss2.str().c_str());
}

