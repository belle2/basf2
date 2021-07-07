/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/io/RootFileInfo.h>
#include <framework/io/RootIOUtilities.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/datastore/DataStore.h>

#include <TTree.h>
#include <TFile.h>

namespace Belle2::RootIOUtilities {
  RootFileInfo::RootFileInfo(const std::string& filename)
  {
    // make sure TDirectory is reset after this function
    TDirectory::TContext directoryGuard;
    // Open the file
    m_file.reset(TFile::Open(filename.c_str(), "READ"));
    if (!m_file || !m_file->IsOpen()) throw std::invalid_argument("Could not open file");
    // We want to get two trees the same way but only need that here ... lambda it is
    auto getTree = [&file = this->m_file](const std::string & label, const std::string & name, std::unique_ptr<TTree>& destination) {
      TTree* tmp{nullptr};
      file->GetObject(name.c_str(), tmp);
      if (!tmp) throw std::runtime_error("No " + label + " tree found");
      destination.reset(tmp);
    };
    // So get the trees from the file and store them in the members
    getTree("persistent", c_treeNames[DataStore::c_Persistent], m_persistent);
    getTree("event", c_treeNames[DataStore::c_Event], m_events);
    // And finally check the number of entries in the persistent tree
    if (auto npersistent = m_persistent->GetEntries(); npersistent != 1) {
      throw std::runtime_error("Expected exactly one entry in persistent tree, found " + std::to_string(npersistent));
    }
  }

  const FileMetaData& RootFileInfo::getFileMetaData()
  {
    if (!m_metadata) {
      // object not set yet, get it from file
      FileMetaData* metadata{nullptr};
      auto branchStatus = m_persistent->SetBranchAddress("FileMetaData", &metadata);
      if (branchStatus < 0) throw std::runtime_error("Error retrieving FileMetaData branch: " + std::to_string(branchStatus));
      if (m_persistent->GetEntry(0) <= 0 || !metadata) throw std::runtime_error("Cannot read FileMetaData");
      // and remembber
      m_metadata.reset(metadata);
    }
    return *m_metadata;
  }

  const std::set<std::string>& RootFileInfo::getBranchNames(bool persistent)
  {
    // which set of branches are we looking at?
    std::optional<std::set<std::string>>& cache{persistent ? m_persistentBranches : m_eventBranches};
    // if we didn't create the list already do it now
    if (!cache) {
      TTree& tree{persistent ? *m_persistent :* m_events};
      std::set<std::string> result;
      for (const TObject* obj : * (tree.GetListOfBranches())) {
        const auto* branch = dynamic_cast<const TBranch*>(obj);
        if (!branch) throw std::runtime_error("Entry in list of branches is no branch");
        result.emplace(branch->GetName());
      }
      // and remember
      cache.emplace(std::move(result));
    }
    // and just return the content
    return *cache;
  }

  void RootFileInfo::checkMissingBranches(const std::set<std::string>& required, bool persistent)
  {
    // So let's get the list of existing branches
    const auto& existing = getBranchNames(persistent);
    // and fill a list of branches which are in required but not existing
    std::vector<std::string> missing;
    std::set_difference(required.begin(), required.end(), existing.begin(), existing.end(),
                        std::inserter(missing, missing.begin()));
    // and if that is not empty we complain
    if (!missing.empty()) {
      std::stringstream message;
      message << "Branches missing from event tree: ";
      for (const auto& name : missing) {
        message << name << ", ";
      }
      throw std::runtime_error(message.str());
    }
  }

  /* Destructor. Declared here to allow forward declaration of TFile,TTree,... in header */
  RootFileInfo::~RootFileInfo() = default;
}
