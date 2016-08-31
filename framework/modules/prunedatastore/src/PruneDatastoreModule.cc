/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 **************************************************************************/

#include <framework/modules/prunedatastore/PruneDatastoreModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>
#include <algorithm>
#include <boost/regex.hpp>

using namespace std;
using namespace Belle2;

REG_MODULE(PruneDatastore)

PruneDatastoreModule::PruneDatastoreModule() :
  Module()
{
  setDescription(
    "Clears the content of the DataStore while it keeps entries listed in the keepEntries option");
  addParam("keepEntries", m_keepEntries,
           "name of all entries (with regex wildcard ) to not remove from the DataStore. For example, you can use 'Raw.*' to keep all Raw-Objects.",
           m_keepEntries);
  setPropertyFlags(c_ParallelProcessingCertified);

}

void PruneDatastoreModule::event()
{
  auto& storemap = DataStore::Instance().getStoreEntryMap(DataStore::c_Event);

  // prepare the regex_matchers, otherwise this nede to be done for each DataStore item
  std::vector < boost::regex > compiled_regex;

  for (auto& kEntry : m_keepEntries) {
    compiled_regex.push_back(boost::regex(kEntry));
  }

  // iterate through all StoreEntries and check the Regex expression for each eintry
  for (auto& datastore_item : storemap) {
    std::string const& datastore_key = datastore_item.first;

    // check if this entry is in our to keep list
    bool toKeep = false;
    for (auto const& regx : compiled_regex) {
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
      datastore_item.second.recreate();
    }
  }

}
