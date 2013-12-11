#include "daq/slc/apps/runcontrold/RCDatabaseManager.h"

#include "daq/slc/apps/runcontrold/RCMaster.h"

#include <daq/slc/base/Debugger.h>

using namespace Belle2;

RCDatabaseManager::RCDatabaseManager(DBInterface* db, RCMaster* master)
  : _db(db), _master(master)
{
  DataObject* data = master->getData();
  for (DataObject::ParamNameList::iterator it = data->getParamNames().begin();
       it != data->getParamNames().end(); it++) {
    DataObject::ParamInfoMap& param_m(data->getParams());
    std::string name = *it;
    if (param_m[name].type == DataObject::OBJECT) {
      search(data->getObject(name));
    }
  }
}

void RCDatabaseManager::search(DataObject* data)
{
  if (data == NULL) return;
  if (_data_v_m.find(data->getClassName()) == _data_v_m.end()) {
    _data_v_m.insert(DataObjectSet::value_type(data->getClassName(), DataObjectList()));
  }
  DataObjectList& data_v(_data_v_m[data->getClassName()]);
  data->setId(data_v.size());
  data_v.push_back(data);
  for (DataObject::ParamNameList::iterator it = data->getParamNames().begin();
       it != data->getParamNames().end(); it++) {
    DataObject::ParamInfoMap& param_m(data->getParams());
    std::string name = *it;
    if (param_m[name].type == DataObject::OBJECT) {
      search(data->getObject(name));
    }
  }
}

void RCDatabaseManager::createTables()
{
  DBObjectLoader loader(_db);
  try {
    _db->connect();
    loader.create(_master->getConfig());
  } catch (const DBHandlerException& e) {
  }
  try {
    loader.create(_master->getStatus());
  } catch (const DBHandlerException& e) {}
  for (DataObjectSet::iterator it = _data_v_m.begin();
       it != _data_v_m.end(); it++) {
    try {
      if (it->second.size() > 0) loader.create(it->second[0]);
    } catch (const DBHandlerException& e) {}
  }
  _db->close();
}

void RCDatabaseManager::dropTables()
{
  DBObjectLoader loader(_db);
  try {
    _db->connect();
    for (DataObjectSet::iterator it = _data_v_m.begin();
         it != _data_v_m.end(); it++) {
      try {
        if (it->second.size() > 0) loader.drop(it->second[0]);
      } catch (const DBHandlerException& e) {}
    }
    try {
      loader.drop(_master->getStatus());
    } catch (const DBHandlerException& e) {}
    try {
      loader.drop(_master->getConfig());
    } catch (const DBHandlerException& e) {}
  } catch (const DBHandlerException& e) {}
  _db->close();
}

int RCDatabaseManager::readConfigs(int confno)
{
  try {
    _db->connect();
    DBObjectLoader loader(_db);
    if (confno < 0) {
      confno = loader.getLatestConfig(_master->getConfig());
      if (confno < 0) confno = 0;
      _master->getConfig()->setConfigNumber(confno);
    }
    loader.read(_master->getConfig());
  } catch (const DBHandlerException& e) {
    Belle2::debug("%s:%d %s", __FILE__, __LINE__, e.what());
  }
  _db->close();
  RCMaster::NSMNodeList& node_v(_master->getNSMNodes());
  DataObject* data = _master->getConfig();
  for (RCMaster::NSMNodeList::iterator it = node_v.begin();
       it != node_v.end(); it++) {
    NSMNode* node = *it;
    node->setUsed(data->getBool(node->getName() + "_used"));
    if (data->hasValue(node->getName()) && node->getData() != NULL) {
      node->getData()->setConfigNumber(data->getInt(node->getName()));
    }
  }
  for (DataObjectSet::iterator it = _data_v_m.begin();
       it != _data_v_m.end(); it++) {
    std::string classname = it->first;
    if (_master->getConfig()->hasInt(classname)) {
      int no = _master->getConfig()->getInt(classname);
      readConfig(classname, no);
    }
  }
  return confno;
}

int RCDatabaseManager::writeConfigs()
{
  int confno = 0;
  RunConfig* run_config = _master->getConfig();
  DBObjectLoader loader(_db);
  try {
    _db->connect();
    confno = loader.getLatestConfig(run_config);
    if (confno < run_config->getConfigNumber()) {
      run_config->update();
    }
  } catch (const DBHandlerException& e) {}
  _db->close();
  for (DataObjectSet::iterator it = _data_v_m.begin();
       it != _data_v_m.end(); it++) {
    std::string classname = it->first;
    writeConfig(classname);
  }
  try {
    _db->connect();
    DBObjectLoader loader(_db);
    if (confno < 0 || confno < run_config->getConfigNumber()) {
      loader.write(run_config);
    }
  } catch (const DBHandlerException& e) {}
  _db->close();
  return confno;
}

int RCDatabaseManager::readConfig(const std::string classname, int confno)
{
  try {
    _db->connect();
    DBObjectLoader loader(_db);
    DataObjectList& data_v(_data_v_m[classname]);
    data_v[0]->setConfigNumber(confno);
    DBRecordList& record_v(loader.readAll(data_v[0]));
    if (confno < 0 && data_v.size() > 0) {
      confno = loader.getLatestConfig(data_v[0]);
    }
    for (size_t i = 0; i < record_v.size(); i++) {
      int id = record_v[i].getFieldValueInt("id");
      int confno = record_v[i].getFieldValueInt("confno");
      if (id >= 0 && id < (int)data_v.size()) {
        data_v[id]->setConfigNumber(confno);
        data_v[id]->setValues(record_v[i].getFieldNames(),
                              record_v[i].getFieldValues());
      }
    }
  } catch (const DBHandlerException& e) {}
  _db->close();
  return confno;
}

int RCDatabaseManager::writeConfig(const std::string classname)
{
  int confno = 0;
  try {
    _db->connect();
    DBObjectLoader loader(_db);
    DataObjectList& data_v(_data_v_m[classname]);
    if (data_v.size() > 0) confno = loader.getLatestConfig(data_v[0]);
    for (size_t i = 0; i < data_v.size(); i++) {
      if (confno < 0 || confno < data_v[i]->getConfigNumber()) {
        loader.write(data_v[i]);
      }
    }
    confno = data_v[0]->getConfigNumber();
  } catch (const DBHandlerException& e) {}
  _db->close();
  return confno;
}

int RCDatabaseManager::readStatus(int confno)
{
  try {
    _db->connect();
    DBObjectLoader loader(_db);
    RunStatus* status = _master->getStatus();
    if (confno < 0) {
      confno = loader.getLatestConfig(status);
    }
    RunConfig* config = _master->getConfig();
    status->setConfigNumber(confno);
    loader.read(status);
    config->setConfigNumber(status->getRunConfig());
    _db->close();
    readConfigs();
  } catch (const DBHandlerException& e) {
    _db->close();
  }
  return confno;
}

int RCDatabaseManager::writeStatus()
{
  int confno = 0;
  try {
    _db->connect();
    DBObjectLoader loader(_db);
    confno = loader.getLatestConfig(_master->getStatus()) + 1;
    _master->getStatus()->setConfigNumber(confno);
    loader.write(_master->getStatus());
  } catch (const DBHandlerException& e) {}
  _db->close();
  return confno;
}
