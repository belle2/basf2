#include "database/DBNodeSystemConfigurator.h"

#include "base/StringUtil.h"
#include "base/Debugger.h"

#include <iostream>
#include <sstream>

using namespace Belle2;

DBNodeSystemConfigurator::DBNodeSystemConfigurator(DBInterface* db,
                                                   NodeLoader* loader) throw()
  : _db(db), _loader(loader), _dbloader(db)
{
  std::map<std::string, std::vector<DataObject*> >& obj_m(loader->getObbjectSet());
  for (std::map<std::string, std::vector<DataObject*> >::iterator it = obj_m.begin();
       it != obj_m.end(); it++) {
    _version_m.insert(std::map<std::string, int>::value_type(it->first, 0));
  }
}

void DBNodeSystemConfigurator::createTable()
{
  try {
    std::stringstream ss;
    ss << "create table version_control_rev"
       << _loader->getSystem().getRevision()
       << " (version int, date timestamp, run_type text";
    std::map<std::string, std::vector<DataObject*> >& obj_v_m(_loader->getObbjectSet());
    for (std::map<std::string, std::vector<DataObject*> >::iterator it = obj_v_m.begin();
         it != obj_v_m.end(); it++) {
      ss << ", " << it->first << " int";
      _dbloader.createTable(it->second[0]);
    }
    ss << ", description text);";
    _db->execute(ss.str());
  } catch (const DBHandlerException& e) {
    std::cout << e.what() << std::endl;
  }
}

void DBNodeSystemConfigurator::dropTable()
{
  try {
    std::stringstream ss;
    ss << "drop table version_control_rev"
       << _loader->getSystem().getRevision() << ";";
    std::map<std::string, std::vector<DataObject*> >& obj_v_m(_loader->getObbjectSet());
    for (std::map<std::string, std::vector<DataObject*> >::iterator it = obj_v_m.begin();
         it != obj_v_m.end(); it++) {
      _dbloader.dropTable(it->second[0]);
    }
    _db->execute(ss.str());
  } catch (const DBHandlerException& e) {
    std::cout << e.what() << std::endl;
  }
}

int DBNodeSystemConfigurator::rereadTable()
{
  try {
    std::string config_name = _loader->getSystem().getConfig()->getClassName();
    int config_rev = _loader->getSystem().getConfig()->getRevision();
    _db->execute(Belle2::form("select * from %s_rev%d order by start_time desc limit 1;",
                              config_name.c_str(), config_rev));
    std::vector<DBRecord>& record_v(_db->loadRecords());
    try {
      return readTable(record_v[0].getFieldValueInt("version"));
    } catch (const IOException& e) {
      Belle2::debug("[FATAL] Error on loading system configuration.:%s", e.what());
    }
  } catch (const IOException& e) {
    Belle2::debug("[DEBUG] failed to find previous configuration.:%s", e.what());
  }
  return -1;
}
int DBNodeSystemConfigurator::readTable(int version)
{
  try {
    std::stringstream ss;
    ss << "select * from version_control_rev"
       << _loader->getSystem().getRevision() << " where version=" << version << ";";
    _db->execute(ss.str());
    std::vector<DBRecord>& record_v(_db->loadRecords());
    if (record_v.size() == 0) return 0;
    std::map<std::string, std::vector<DataObject*> >& obj_v_m(_loader->getObbjectSet());
    int count = 0;
    for (std::map<std::string, std::vector<DataObject*> >::iterator it = obj_v_m.begin();
         it != obj_v_m.end(); it++) {
      std::string name = it->first;
      _dbloader.readTable(it->second, record_v[0].getFieldValueInt(name));
      count++;
    }
    return count;
  } catch (const DBHandlerException& e) {
    std::cout << e.what() << std::endl;
  }
  return -1;
}

int DBNodeSystemConfigurator::writeTable(int version)
{
  try {
    std::stringstream ss1, ss2;
    ss1 << "insert into version_control_rev"
        << _loader->getSystem().getRevision()
        << " (version, date, run_type" ;
    ss2 << ") values (" << version << ", current_timestamp, '"
        << _loader->getSystem().getConfig()->getRunType() << "' ";
    std::map<std::string, std::vector<DataObject*> >& obj_v_m(_loader->getObbjectSet());
    int count = 0;
    for (std::map<std::string, std::vector<DataObject*> >::iterator it = obj_v_m.begin();
         it != obj_v_m.end(); it++) {
      std::string name = it->first;
      _dbloader.writeTable(it->second, _version_m[name]);
      ss1 << ", " << name;
      ss2 << ", " << _version_m[name];
      count++;
    }
    ss1 << ", description" << ss2.str() << ", '"
        << _loader->getSystem().getConfig()->getDescription() << "');";
    _db->execute(ss1.str());
    return count;
  } catch (const DBHandlerException& e) {
    std::cout << e.what() << std::endl;
  }
  return -1;
}
