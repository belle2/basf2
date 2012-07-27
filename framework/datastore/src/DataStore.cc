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
  for (StoreObjIter iter = m_storeObjMap[durability].begin(); iter != m_storeObjMap[durability].end(); ++iter) {
    TClonesArray* array = dynamic_cast<TClonesArray*>(iter->second);
    if (array) {
      //clean TClonesArray
      array->Delete();
    } else {
      //delete TObject
      delete iter->second; // delete object
      iter->second = 0;    // make sure pointer isn't dangling
    }
  }
}
