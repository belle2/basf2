/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/printcollections/PrintCollectionsModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TClonesArray.h>

#include <boost/format.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintCollections)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintCollectionsModule::PrintCollectionsModule()
{
  //Set module properties
  setDescription("Prints the current data store collections.");

  //Parameter definition
}


PrintCollectionsModule::~PrintCollectionsModule()
{
}

void PrintCollectionsModule::initialize()
{
  StoreObjPtr<EventMetaData>::required();
}

void PrintCollectionsModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  B2INFO("============================================================================");
  B2INFO("DataStore collections in event " << eventMetaDataPtr->getEvent());
  B2INFO("============================================================================");
  B2INFO("")
  B2INFO("(Type)    Name                             Number of entries         <Event>");
  printCollections(DataStore::c_Event);
  B2INFO("")
  B2INFO("(Type)    Name                             Number of entries    <Persistent>");
  printCollections(DataStore::c_Persistent);
  B2INFO("")
  B2INFO("============================================================================");
}



//===============================================================
//                    Protected methods
//===============================================================

void PrintCollectionsModule::printCollections(DataStore::EDurability durability)
{
  B2INFO("----------------------------------------------------------------------------");

  //-----------------------------
  //Print the object information
  //-----------------------------
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(durability);
  for (DataStore::StoreObjConstIter iter = map.begin(); iter != map.end(); ++iter) {
    if (iter->second->isArray)
      continue;
    const TObject* currCol = iter->second->ptr;

    if (currCol != NULL) {
      B2INFO(boost::format("(Object)  %1%") % iter->first);
    }
  }


  //-----------------------------
  //Print the array information
  //-----------------------------
  for (DataStore::StoreObjConstIter iter = map.begin(); iter != map.end(); ++iter) {
    if (!iter->second->isArray)
      continue;
    const TClonesArray* currCol = dynamic_cast<TClonesArray*>(iter->second->ptr);

    long entries = 0;
    if (currCol != NULL)
      entries = currCol->GetEntriesFast();

    B2INFO(boost::format("(Array)   %1% %|42t| %2%") % iter->first % entries);
  }
}
