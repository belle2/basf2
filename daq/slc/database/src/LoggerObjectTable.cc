#include <daq/slc/database/LoggerObjectTable.h>
#include <daq/slc/database/FieldInfoTable.h>
#include <daq/slc/database/LoggerInfoTable.h>
#include <daq/slc/database/NodeInfoTable.h>
#include <daq/slc/database/ConfigObject.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace Belle2;

int LoggerObjectTable::add(DBObject& obj, bool isroot)
throw(DBHandlerException)
{
  int id = 0;
  if (obj.getIndex() == 0) {
    FieldInfoTable(m_db).createTable(obj, isroot);
    LoggerInfoTable table(m_db);
    //LogFile::debug("adding LogggerInfo %s:%s:%d",
    //               obj.getNode().c_str(),
    //               obj.getTable().c_str(), obj.getRevision());
    id = table.add(LoggerInfo(obj.getNode(), obj.getTable(), obj.getRevision()));
  }
  std::stringstream ss1, ss2;
  ss1 << "loggerid";
  if (id > 0) ss2 << id;
  else ss2 << "(select loggerid('" << obj.getNode()
             << "', '" << obj.getTable() << "', "
             << obj.getRevision() << "))";
  if (!isroot) {
    ss1 << ", index";
    ss2 << ", " << obj.getIndex();
  }
  const FieldNameList& name_v(obj.getFieldNames());
  for (size_t i = 0; i < name_v.size(); i++) {
    const std::string name = name_v[i];
    ss1 << ", " << name;
    if (obj.hasObject(name)) {
      size_t nobj = obj.getNObjects(name);
      for (size_t j = 0; j < nobj; j++) {
        DBObject& cobj(obj.getObject(name, j));
        if (!cobj.isConfig()) add(cobj, false);
      }
    }
    const FieldInfo::Property& pro(obj.getProperty(name));
    if (!obj.hasObject(name) && pro.getLength() > 0) {
      const int size = pro.getTypeSize();
      const int length = pro.getLength();
      const char* ptr = (const char*)obj.getValue(name);
      int cid = 0;
      for (int j = 0; j < length; j++) {
        ConfigObject cobj;
        cobj.setConfig(false);
        cobj.setNode(obj.getNode());
        cobj.setName(obj.getName());
        cobj.setTable(obj.getTable() + "." + name);
        cobj.setRevision(obj.getRevision());
        cobj.setIndex(j);
        cobj.addValue(name, ptr + size * j, pro.getType(), 0);
        cid = add(cobj, false);
      }
      ss2 << "," << cid;
    } else {
      ss2 << ", " << FieldInfo::getSQL(obj, name);
    }
  }
  m_db->execute("insert into \"loggerinfo:%s:%d\" (%s) values (%s) returning loggerid;",
                obj.getTable().c_str(), obj.getRevision(),
                ss1.str().c_str(), ss2.str().c_str());
  obj.setId(id);
  return id;
}

