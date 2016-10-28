/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 **************************************************************************/

#include <framework/modules/prunedatastore/PruneDataStoreModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

REG_MODULE(PruneDataStore)

PruneDataStoreModule::PruneDataStoreModule() :
  Module()
{
  setDescription(
    "Clears the content of the DataStore while it keeps entries listed in the keepEntries option. The EventMetaData object will always be kept, as it is required by the framework to properly work with the DataStore. Note: Also all Relations will be cleared if they are not matched by one entry in the keepEntries list. You have to ensure the objects referenced by kept relations are also matched by one entry in the keepEntries list so a relation does not point to nirvana.");
  addParam("keepEntries", m_keepEntries,
           "name of all entries (with regex wildcard ) to not remove from the DataStore. For example, you can use 'Raw.*' to keep all Raw-Objects.",
           m_keepEntries);
  setPropertyFlags(c_ParallelProcessingCertified);

}

void PruneDataStoreModule::initialize()
{
  // prepare the regex_matchers, otherwise this nede to be done for each DataStore item
  for (auto& kEntry : m_keepEntries) {
    m_compiled_regex.push_back(boost::regex(kEntry));
  }
  // also get the regex for the implicit keeps
  for (auto& kEntry : m_keepEntriesImplicit) {
    m_compiled_regex.push_back(boost::regex(kEntry));
  }
}

void PruneDataStoreModule::event()
{
  auto& storemap = DataStore::Instance().getStoreEntryMap(DataStore::c_Event);

  // iterate through all StoreEntries and check the Regex expression for each eintry
  for (auto& datastore_item : storemap) {
    std::string const& datastore_key = datastore_item.first;

    // check if this entry is in our to keep list
    bool toKeep = false;
    for (auto const& regx : m_compiled_regex) {
      if (regex_match(datastore_key, regx)) {
        toKeep = true;
      }
    }

    if (toKeep) {
      B2DEBUG(100,
              "StoreArray entry " << datastore_key << " will be not pruned from the datastore");
    } else {
      B2DEBUG(100,
              "StoreArray entry " << datastore_key << " will be pruned from the datastore");
    }

    if (!toKeep) {
      // recreate to clear its content
      datastore_item.second.invalidate();
    }
  }

}
