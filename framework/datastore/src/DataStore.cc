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


DataStore::DataStore() : initializeActive(false)
{
}


void DataStore::clearMaps(const EDurability& durability)
{
  B2DEBUG(100, "Start deletion process of durability " << durability);
  for (StoreObjIter iter = m_objectMap[durability].begin(); iter != m_objectMap[durability].end(); iter++) {
    if (iter->second) {
      delete iter->second; // delete object
      iter->second = 0;    // make sure pointer isn't dangling
      //    delete iter->second.get<1>();
    }
  }

  TClonesArray* array;
  for (StoreArrayIter iter = m_arrayMap[durability].begin(); iter != m_arrayMap[durability].end(); iter++) {
    array = static_cast<TClonesArray*>(iter->second);
    if (array) {
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

