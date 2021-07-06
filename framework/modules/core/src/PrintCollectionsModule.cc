/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/modules/core/PrintCollectionsModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TClass.h>
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
  setDescription("Prints the contents of the DataStore in an event, listing all objects and arrays (including size).");

  addParam("printForEvent", m_printForEvent,
           "Print the collections only for a specific event number.  If set to -1 (default) only the collections of the first event will be printed, if set to 0, the collections of all events will be printed, which might be a lot of output.",
           m_printForEvent);
}

PrintCollectionsModule::~PrintCollectionsModule() = default;

void PrintCollectionsModule::initialize()
{
  StoreObjPtr<EventMetaData>().isRequired();
}

void PrintCollectionsModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr;

  // check if printing only for the first event
  if (m_printForEvent < 0) {
    if (m_firstEvent)
      m_firstEvent = false;
    else
      return;
  }
  // or for a specific event
  else if ((m_printForEvent > 0) && ((unsigned int)m_printForEvent != eventMetaDataPtr->getEvent()))
    return;

  B2INFO("============================================================================");
  B2INFO("DataStore collections in event " << eventMetaDataPtr->getEvent());
  B2INFO("============================================================================");
  B2INFO(boost::format("Type %|20t| Name %|47t| #Entries           <Event>"));
  printCollections(DataStore::c_Event);
  B2INFO("----------------------------------------------------------------------------");
  B2INFO(boost::format("Type %|20t| Name %|47t| #Entries      <Persistent>"));
  printCollections(DataStore::c_Persistent);
  B2INFO("============================================================================");
}


/** remove Belle2 namespace prefix from className, if present. */
std::string shorten(std::string className)
{
  if (className.compare(0, 8, "Belle2::") == 0) {
    //we know the experiment name, thanks
    return className.substr(8);
  }
  return className;
}

//===============================================================
//                    Protected methods
//===============================================================


void PrintCollectionsModule::printCollections(DataStore::EDurability durability)
{
  //-----------------------------
  //Print the object information
  //-----------------------------
  const DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(durability);
  for (auto iter = map.begin(); iter != map.end(); ++iter) {
    if (iter->second.isArray)
      continue;
    const TObject* currCol = iter->second.ptr;

    if (currCol != nullptr) {
      B2INFO(boost::format("%1% %|20t| %2%") % shorten(currCol->ClassName()) % iter->first);
    }
  }


  //-----------------------------
  //Print the array information
  //-----------------------------
  for (auto iter = map.begin(); iter != map.end(); ++iter) {
    if (!iter->second.isArray)
      continue;
    const TClonesArray* currCol = dynamic_cast<TClonesArray*>(iter->second.ptr);

    long entries = 0;
    if (currCol != nullptr)
      entries = currCol->GetEntriesFast();

    std::string type = shorten(iter->second.objClass->GetName());

    B2INFO(boost::format("%1%[] %|20t| %2% %|47t| %3%") % type % iter->first % entries);
  }
  B2INFO("");
}
