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
                    "confignames('%s', '%s');",
                    configname.c_str(), nodename.c_str());
      DBRecordList record_v(m_db->loadRecords());
      if (record_v.size() > 0) {
        tablename = record_v[0].get("table");
        revision = record_v[0].getInt("revision");
      }
    }
    TableInfo tinfo = TableInfoTable(m_db).get(tablename, revision);
    if (tablename.size() == 0 || revision <= 0) {
      LogFile::warning("No table for (%s:%s) found in config DB",
                       nodename.c_str(), configname.c_str());
      return obj_v;
    }
    FieldInfoTable ftable(m_db);
    FieldInfoList finfo_v = ftable.getList(tablename, revision);
    std::stringstream ss;
    ss << "select configid" << ((!tinfo.isRoot()) ? ", index " : "");
    for (FieldInfoList::iterator it = finfo_v.begin();
         it != finfo_v.end(); it++) {
      ss << ", " << it->setSQL();
    }
    ConfigInfo cinfo(configname, nodename, tablename, revision);
    const std::string tablename_imp =
      StringUtil::form("\"configinfo:%s:%d\"", tablename.c_str(), revision);
    ss << StringUtil::form("from %s where configid = (%s);",
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
          if (it->getType() == FieldInfo::OBJECT) {
            int id = record.getInt(name);
            if (id == 0) {
              LogFile::error("%s:%s", configname.c_str(), nodename.c_str());
            }
            obj.addObjects(name, getList(id));
          }
        }
        obj_v.push_back(obj);
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("%s:%s", configname.c_str(), nodename.c_str());
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return obj_v;
}

ConfigObjectList ConfigObjectTable::getList(int confid, bool isroot)
throw()
{
  ConfigObjectList obj_v;
  if (m_db != NULL) {
    std::string nodename;
    std::string configname;
    std::string tablename;
    int revision = 0;
    m_db->execute("select name, node, \"table\", revision from "
                  "confignames() where id = %d;", confid);
    DBRecordList record_v(m_db->loadRecords());
    if (record_v.size() > 0) {
      configname = record_v[0].get("name");
      nodename = record_v[0].get("node");
      tablename = record_v[0].get("table");
      revision = record_v[0].getInt("revision");
    }
    FieldInfoTable ftable(m_db);
    FieldInfoList finfo_v = ftable.getList(tablename, revision);
    try {
      m_db->execute("select * from \"configinfo:%s:%d\" where configid = %d;",
                    tablename.c_str(), revision, confid);
      DBRecordList record_v(m_db->loadRecords());
      for (size_t i = 0; i < record_v.size(); i++) {
        DBRecord& record(record_v[i]);
        ConfigObject obj;
        obj.setName(configname);
        obj.setNode(nodename);
        obj.setTable(tablename);
        obj.setRevision(revision);
        obj.setId(record.getInt("configid"));
        if (record.hasField("index")) obj.setIndex(record.getInt("index"));
        for (FieldInfoList::iterator it = finfo_v.begin();
             it != finfo_v.end(); it++) {
          it->setSQL(record, obj);
          std::string name = it->getName();
          if (it->getType() == FieldInfo::OBJECT) {
            obj.addObjects(name, getList(record.getInt(name)));
          }
        }
        obj_v.push_back(obj);
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("%s:%s", configname.c_str(), nodename.c_str());
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return obj_v;
}

ConfigObject ConfigObjectTable::get(const std::string& configname,
                                    const std::string& nodename) throw()
{
  ConfigObjectList list = getList(configname, nodename, "", 0);
  if (list.size() > 0) return list[0];
  return ConfigObject();
}

ConfigObject ConfigObjectTable::get(int confid) throw()
{
  ConfigObjectList list = getList(confid, true);
  if (list.size() > 0) return list[0];
  return ConfigObject();
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
  try {
    m_db->execute("insert into \"configinfo:%s:%d\" (%s) values (%s);",
                  obj.getTable().c_str(), obj.getRevision(),
                  ss1.str().c_str(), ss2.str().c_str());
  } catch (const DBHandlerException& e) {
    //LogFile::warning("%s", e.what());
  }
}

void ConfigObjectTable::addAll(const ConfigObject& obj, bool isroot)
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
    if (obj.hasObject(name_v[i])) {
      for (int n = 0; n < obj.getNObjects(name_v[i]); n++) {
        addAll((const ConfigObject&)obj.getObject(name_v[i], n), false);
      }
    }
  }
  try {
    m_db->execute("insert into \"configinfo:%s:%d\" (%s) values (%s);",
                  obj.getTable().c_str(), obj.getRevision(),
                  ss1.str().c_str(), ss2.str().c_str());
  } catch (const DBHandlerException& e) {
    //LogFile::warning("%s", e.what());
  }
}

