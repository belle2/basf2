#include "daq/slc/database/DBObjectLoader.h"
#include "daq/slc/base/Debugger.h"

#include <sstream>

using namespace Belle2;

void DBObjectLoader::create(DataObject* obj)
{
  if (obj->getParamNames().size() == 0) return;
  try {
    std::stringstream ss;
    ss << "create table \"" << obj->getClassName()
       << "_rev" << obj->getRevision() << "\" ("
       << "record_time timestamp, "
       << obj->toSQLConfig() << ");";
    _db->execute(ss.str());
  } catch (const DBHandlerException& e) {
    Belle2::debug(e.what());
  }
}

void DBObjectLoader::drop(DataObject* obj)
{
  if (obj->getParamNames().size() == 0) return;
  try {
    std::stringstream ss;
    ss << "drop table \"" << obj->getClassName()
       << "_rev" << obj->getRevision() << "\";";
    _db->execute(ss.str());
  } catch (const DBHandlerException& e) {
    Belle2::debug(e.what());
  }
}

DBRecordList& DBObjectLoader::readAll(DataObject* obj)
{
  try {
    std::stringstream ss;
    ss << "select * from \"" << obj->getClassName()
       << "_rev" << obj->getRevision() << "\" "
       << "where confno = " << obj->getConfigNumber() << ";";
    _db->execute(ss.str());
    return _db->loadRecords();
  } catch (const DBHandlerException& e) {
    Belle2::debug(e.what());
    _db->clearRecords();
  }
  return _db->getRecords();
}

void DBObjectLoader::read(DataObject* obj)
{
  try {
    if (obj->getParamNames().size() == 0) return;
    std::stringstream ss;
    ss << "select * from \"" << obj->getClassName()
       << "_rev" << obj->getRevision() << "\""
       << " where confno = " << obj->getConfigNumber()
       << " and id = " << obj->getId() << " limit 1;";
    _db->execute(ss.str());
    DBRecordList& record_v(_db->loadRecords());
    if (record_v.size() > 0) {
      obj->setValues(record_v[0].getFieldNames(),
                     record_v[0].getFieldValues());
    }
  } catch (const DBHandlerException& e) {
    Belle2::debug(e.what());
  }
}

void DBObjectLoader::write(DataObject* obj)
{
  if (obj->getParamNames().size() == 0) return;
  try {
    std::stringstream ss;
    ss << "insert into \"" << obj->getClassName()
       << "_rev" << obj->getRevision() << "\" ("
       << "" << obj->toSQLNames() << ") values ("
       << "" << obj->toSQLValues() << ");";
    _db->execute(ss.str());
  } catch (const DBHandlerException& e) {
    Belle2::debug(e.what());
  }
}

int DBObjectLoader::getLatestConfig(DataObject* obj)
{
  try {
    std::stringstream ss;
    ss << "select confno from \"" << obj->getClassName()
       << "_rev" << obj->getRevision() << "\" "
       << "order by record_time desc limit 1;";
    _db->execute(ss.str());
    DBRecordList& record_v(_db->loadRecords());
    if (record_v.size() > 0) {
      return record_v[0].getFieldValueInt("confno");
    }
  } catch (const DBHandlerException& e) {
    Belle2::debug(e.what());
  }
  return -1;
}

std::vector<int> DBObjectLoader::getConfigList(DataObject* obj)
{
  std::vector<int> conf_v;
  try {
    std::stringstream ss;
    ss << "select distinct confno from \"" << obj->getClassName()
       << "_rev" << obj->getRevision() << "\" "
       << "order by record_time desc;";
    _db->execute(ss.str());
    DBRecordList& record_v(_db->loadRecords());
    for (size_t i = 0; i < record_v.size(); i++) {
      conf_v.push_back(record_v[i].getFieldValueInt("confno"));
    }
  } catch (const DBHandlerException& e) {
    Belle2::debug(e.what());
  }
  return conf_v;
}
