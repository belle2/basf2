/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/DataStore.h>

using namespace std;
using namespace Belle2;

DataStore::DataStore* DataStore::m_instance = 0;


DataStore::DataStore& DataStore::Instance()
{
  //Access to singleton
  if (!m_instance) { m_instance = new DataStore;}
  return *m_instance;
}


DataStore::DataStore()
{
}


bool DataStore::storeObject(TObject* object, const std::string& name, const EDurability& durability)
{
  //check for existence
  map<string, TObject*>::iterator iter = m_objectMap[durability].find(name);

  if (iter != m_objectMap[durability].end()) {
    B2ERROR("Object of chosen name already exists@durability." + name);
    return false;
  }

  m_objectMap[durability][name] = object;
  return true;
}


bool DataStore::storeArray(TClonesArray* array, const std::string& name, const EDurability& durability)
{
  //check for existence
  map<string, TClonesArray*>::iterator iter = m_arrayMap[durability].find(name);

  if (iter != m_arrayMap[durability].end()) {
    iter->second = array;
    return false;
  }

  m_arrayMap[durability][name] = array;
  return true;
}


void DataStore::clearMaps(const EDurability& durability)
{
  for (StoreObjIter iter = m_objectMap[durability].begin(); iter != m_objectMap[durability].end(); iter++) {
    //iter->second->Clear();
    m_objectMap[durability].erase(iter);
  }

  TClonesArray* array;
  for (map<string, TClonesArray*>::iterator iter = m_arrayMap[durability].begin(); iter != m_arrayMap[durability].end(); iter++) {
    array = static_cast<TClonesArray*>(iter->second);
    if (array) {
      //array->Clear("C");
      array->Delete();
    }
  }
}


StoreMapIter<DataStore::StoreObjMap>*  DataStore:: getObjectIterator(const EDurability& durability)
{
  return new StoreMapIter <DataStore::StoreObjMap> (&m_objectMap[durability]);
}


StoreMapIter<DataStore::StoreArrayMap>*  DataStore:: getArrayIterator(const EDurability& durability)
{
  return new StoreMapIter <DataStore::StoreArrayMap> (&m_arrayMap[durability]);
}

