/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 **************************************************************************/

#include <framework/modules/core/PruneDataStoreModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

REG_MODULE(PruneDataStore)

PruneDataStoreModule::PruneDataStoreModule() :
  Module()
{
  setDescription(
    "Clears the content of the DataStore while it keeps entries listed in the matchEntries option."
    "The EventMetaData object will always be kept, as it is required by the framework to properly "
    "work with the DataStore. Note: Also all Relations will be cleared if they are not matched by "
    "one entry in the matchEntries list. You have to ensure the objects referenced by kept relations "
    "are also matched by one entry in the keepEntries list so a relation does not point to nirvana."
    "This logic can be inverted to remove only the entries matched by the regex in the matchEntries "
    "parameter by setting the parameter to keepMatchedEntries to False.");
  addParam("matchEntries", m_matchEntries,
           "name of all DataStore entries (with regex syntax) to match. For example, you can use 'Raw.*' to match all Raw-Objects.",
           m_matchEntries);
  addParam("keepMatchedEntries", m_keepMatchedEntries,
           "If true, all entries matched by the RegEx expression are kept. If false, matched entries will be removed.",
           m_keepMatchedEntries);
  setPropertyFlags(c_ParallelProcessingCertified);

}

void PruneDataStoreModule::initialize()
{
  // prepare the regex_matchers, otherwise this nede to be done for each DataStore item
  for (auto& kEntry : m_matchEntries) {
    m_compiled_regex.push_back(compileAndCatch(kEntry));
  }
  // also get the regex for the implicit keeps
  for (auto& kEntry : m_keepEntriesImplicit) {
    m_compiled_regex_implicit.push_back(compileAndCatch(kEntry));
  }
}

std::regex PruneDataStoreModule::compileAndCatch(std::string& regexString) const
{
  try {
    return std::regex(regexString);
  } catch (const std::regex_error& e) {
    B2FATAL("Regex '" << regexString << "' cannot be compiled: " <<  e.what());
    // keep the compiler happy and return something
    return std::regex();
  }
}

void PruneDataStoreModule::event()
{
  auto& storemap = DataStore::Instance().getStoreEntryMap(DataStore::c_Event);

  // iterate through all StoreEntries and check the Regex expression for each entry
  for (auto& datastore_item : storemap) {
    std::string const& datastore_key = datastore_item.first;

    // check if this entry is in our to keep list
    bool toKeep = !m_keepMatchedEntries;
    for (auto const& regx : m_compiled_regex) {
      if (std::regex_match(datastore_key, regx)) {
        toKeep = m_keepMatchedEntries;
      }
    }

    // check for implicit keeps
    for (auto const& regx : m_compiled_regex_implicit) {
      if (std::regex_match(datastore_key, regx)) {
        // always keep, no matter of the keepMatchedEntries configuration
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
