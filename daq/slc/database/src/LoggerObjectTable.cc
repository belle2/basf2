#include <daq/slc/database/LoggerObjectTable.h>
#include <daq/slc/database/FieldInfoTable.h>
#include <daq/slc/database/LoggerInfoTable.h>
#include <daq/slc/database/NodeInfoTable.h>

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
    ss1 << ", " << name_v[i];
    ss2 << ", " << FieldInfo::getSQL(obj, name_v[i]);
    if (obj.hasObject(name_v[i])) {
      size_t nobj = obj.getNObjects(name_v[i]);
      for (size_t j = 0; j < nobj; j++) {
        DBObject& cobj(obj.getObject(name_v[i], j));
        if (!cobj.isConfig()) add(cobj, false);
      }
    }
  }
  m_db->execute("insert into \"loggerinfo:%s:%d\" (%s) values (%s) returning loggerid;",
                obj.getTable().c_str(), obj.getRevision(),
                ss1.str().c_str(), ss2.str().c_str());
  obj.setId(id);
  return id;
}

