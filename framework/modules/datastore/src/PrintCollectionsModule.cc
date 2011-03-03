/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/modules/datastore/PrintCollectionsModule.h"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/EventMetaData.h>

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


void PrintCollectionsModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  B2INFO("============================================================================");
  B2INFO("DataStore collections in event " << eventMetaDataPtr->getEvent());
  B2INFO("============================================================================");
  B2INFO("")
  B2INFO("(Type)    Name                      Number entries                   <Event>");
  printCollections(DataStore::c_Event);
  B2INFO("")
  B2INFO("(Type)    Name                      Number entries                     <Run>");
  printCollections(DataStore::c_Run);
  B2INFO("")
  B2INFO("(Type)    Name                      Number entries              <Persistent>");
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
  StoreMapIter<DataStore::StoreObjMap>* objectMapIter = DataStore::Instance().getObjectIterator(durability);
  if (objectMapIter == NULL) return;
  objectMapIter->first();

  while (!objectMapIter->isDone()) {
    TObject* currCol = dynamic_cast<TObject*>(objectMapIter->value());

    if (currCol != NULL) {
      B2INFO(boost::format("(Object)  %1%") % objectMapIter->key());
    }
    objectMapIter->next();
  }


  //-----------------------------
  //Print the array information
  //-----------------------------
  StoreMapIter<DataStore::StoreArrayMap>* arrayMapIter = DataStore::Instance().getArrayIterator(durability);
  if (arrayMapIter == NULL) return;
  arrayMapIter->first();

  while (!arrayMapIter->isDone()) {
    TClonesArray* currCol = dynamic_cast<TClonesArray*>(arrayMapIter->value());

    if (currCol != NULL) {
      B2INFO(boost::format("(Array)   %1% %|35t| %2%") % arrayMapIter->key()  % currCol->GetEntriesFast());
    }
    arrayMapIter->next();
  }
}
