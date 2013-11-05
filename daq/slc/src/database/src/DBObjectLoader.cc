#include "database/DBObjectLoader.h"

#include <sstream>
#include <iostream>

using namespace Belle2;

void DBObjectLoader::createTable(DataObject* obj)
{
  if (obj->getParamNames().size() == 0) return;
  try {
    std::stringstream ss;
    ss << "create table " << obj->getClassName()
       << "_rev" << obj->getRevision() << " ("
       << "version int, id smallint"
       << obj->toSQLConfig() << ");";
    _db->execute(ss.str());
  } catch (const DBHandlerException& e) {
    std::cout << e.what() << std::endl;
  }
}

void DBObjectLoader::dropTable(DataObject* obj)
{
  if (obj->getParamNames().size() == 0) return;
  try {
    std::stringstream ss;
    ss << "drop table " << obj->getClassName()
       << "_rev" << obj->getRevision() << ";";
    _db->execute(ss.str());
  } catch (const DBHandlerException& e) {
    std::cout << e.what() << std::endl;
  }
}

int DBObjectLoader::readTable(std::vector<DataObject*>& obj_v, int version)
{
  try {
    if (obj_v.size() == 0 ||
        obj_v[0]->getParamNames().size() == 0) return 0;
    std::stringstream ss;
    ss << "select * from " << obj_v[0]->getClassName()
       << "_rev" << obj_v[0]->getRevision() << " where version = " << version << ";";
    _db->execute(ss.str());
    std::vector<DBRecord>& record_v(_db->loadRecords());
    int count = 0;
    for (size_t i = 0; i < record_v.size(); i++) {
      const int id = record_v[i].getFieldValueInt("id");
      if (id >= 0 && id < (int)obj_v.size()) {
        obj_v[id]->setSQLValues(record_v[i].getFieldNames(),
                                record_v[i].getFieldValues());
        count++;
      }
    }
    return count;
  } catch (const DBHandlerException& e) {
    std::cout << e.what() << std::endl;
  }
  return -1;
}

int DBObjectLoader::writeTable(std::vector<DataObject*>& obj_v, int version)
{
  if (obj_v.size() == 0 ||
      obj_v[0]->getParamNames().size() == 0) return 0;
  try {
    int count = 0;
    for (size_t i = 0; i < obj_v.size(); i++) {
      std::stringstream ss;
      ss << "insert into " << obj_v[i]->getClassName()
         << "_rev" << obj_v[i]->getRevision() << "("
         << "version, id" << obj_v[i]->toSQLNames() << ") values ("
         << version << ", " << i << obj_v[i]->toSQLValues() << ");";
      _db->execute(ss.str());
      count++;
    }
    return count;
  } catch (const DBHandlerException& e) {
    std::cout << e.what() << std::endl;
  }
  return -1;
}
