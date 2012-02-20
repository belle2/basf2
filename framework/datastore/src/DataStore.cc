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

DataStore* DataStore::m_instance = 0;


DataStore& DataStore::Instance()
{
  //Access to singleton
  if (!m_instance) { m_instance = new DataStore;}
  return *m_instance;
}


DataStore::DataStore() : m_initializeActive(false)
{
}


void DataStore::clearMaps(const EDurability& durability)
{
  B2DEBUG(100, "Start deletion process of durability " << durability);
  for (StoreObjIter iter = m_objectMap[durability].begin(); iter != m_objectMap[durability].end(); ++iter) {
    if (iter->second) {
      B2DEBUG(250, iter->second->ClassName() << " is going to be deleted.");
      delete iter->second; // delete object
      iter->second = 0;    // make sure pointer isn't dangling
    }
  }

  for (StoreObjIter iter = m_arrayMap[durability].begin(); iter != m_arrayMap[durability].end(); ++iter) {
    TClonesArray* array = static_cast<TClonesArray*>(iter->second);
    if (array) {
      array->Delete();
    }
  }
}


StoreMapIter<DataStore::StoreObjMap>*  DataStore::getObjectIterator(const EDurability& durability)
{
  B2WARNING("DataStore::getObjectIterator() and the StoreIter class are deprecated! Please use getObjMap() and std:map<>::iterators instead.");
  return new StoreMapIter <DataStore::StoreObjMap> (&m_objectMap[durability]);
}


StoreMapIter<DataStore::StoreObjMap>*  DataStore::getArrayIterator(const EDurability& durability)
{
  B2WARNING("DataStore::getArrayIterator() and the StoreIter class are deprecated! Please use getArrayMap() and std:map<>::iterators instead.");
  return new StoreMapIter <DataStore::StoreObjMap> (&m_arrayMap[durability]);
}

