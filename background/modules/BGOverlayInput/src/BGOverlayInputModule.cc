/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BGOverlayInput/BGOverlayInputModule.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// MetaData
#include <background/dataobjects/BackgroundInfo.h>

// root
#include <framework/io/RootIOUtilities.h>
#include <TClonesArray.h>
#include <TFile.h>


#include <iostream>

using namespace std;

namespace Belle2 {

  using namespace RootIOUtilities;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(BGOverlayInput)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  BGOverlayInputModule::BGOverlayInputModule() : Module()

  {
    // module description
    setDescription("Input for BG overlay, either in form of Digits or raw data.");

    // Add parameters
    addParam("inputFileNames", m_inputFileNames,
             "List of files with measured beam background ");

  }

  BGOverlayInputModule::~BGOverlayInputModule()
  {
  }

  void BGOverlayInputModule::initialize()
  {

    loadDictionaries();

    // expand possible wildcards
    m_inputFileNames = expandWordExpansions(m_inputFileNames);
    if (m_inputFileNames.empty()) {
      B2FATAL("No valid files specified!");
    }

    // check files
    TDirectory* dir = gDirectory;
    for (const string& fileName : m_inputFileNames) {
      TFile* f = TFile::Open(fileName.c_str(), "READ");
      if (!f or !f->IsOpen()) {
        B2FATAL("Couldn't open input file " + fileName);
      }
      delete f;
    }
    dir->cd();

    // get event TTree
    m_tree = new TChain(c_treeNames[DataStore::c_Event].c_str());
    for (const string& fileName : m_inputFileNames) {
      m_tree->AddFile(fileName.c_str());
    }
    m_numEvents = m_tree->GetEntries();
    if (m_numEvents == 0) B2ERROR(c_treeNames[DataStore::c_Event] << " has no entires");
    m_eventCount = 0;

    // connect selected branches to DataStore
    bool ok = connectBranches();
    if (!ok) {
      B2ERROR("No branches found to be connected");
    }

    // add BackgroundInfo to persistent tree
    StoreObjPtr<BackgroundInfo> bkgInfo("", DataStore::c_Persistent);
    bkgInfo.registerInDataStore();
    bkgInfo.create();
    bkgInfo->setMethod(BackgroundInfo::c_Overlay);
    BackgroundInfo::BackgroundDescr descr;
    descr.tag = SimHitBase::bg_other;
    descr.type = string("RandomTrigger");
    descr.fileNames = m_inputFileNames;
    descr.numEvents = m_numEvents;
    m_index = bkgInfo->appendBackgroundDescr(descr);

  }


  void BGOverlayInputModule::beginRun()
  {
  }


  void BGOverlayInputModule::event()
  {
    StoreObjPtr<BackgroundInfo> bkgInfo("", DataStore::c_Persistent);

    for (auto entry : m_storeEntries) {
      entry->resetForGetEntry();
    }

    m_tree->GetEntry(m_eventCount);
    m_eventCount++;
    if (m_eventCount >= m_numEvents) {
      m_eventCount = 0;
      B2INFO("BGOverlayInput: events for BG overlay will be re-used");
      bkgInfo->incrementReusedCounter(m_index);
    }

    for (auto entry : m_storeEntries) {
      if (entry->object) {
        entry->ptr = entry->object;
      } else {
        entry->recoverFromNullObject();
        entry->ptr = 0;
      }
    }

  }


  void BGOverlayInputModule::endRun()
  {
  }

  void BGOverlayInputModule::terminate()
  {

    delete m_tree;

  }


  bool BGOverlayInputModule::connectBranches()
  {

    auto durability = DataStore::c_Event;
    auto storeFlags = DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered;
    auto& map = DataStore::Instance().getStoreEntryMap(durability);

    const TObjArray* branches = m_tree->GetListOfBranches();
    if (!branches) return false;

    for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
      TBranch* branch = static_cast<TBranch*>(branches->At(jj));
      if (!branch) continue;
      const std::string branchName = branch->GetName();

      TObject* objectPtr = 0;
      branch->SetAddress(&objectPtr);
      branch->GetEntry();
      bool array = (string(branch->GetClassName()) == "TClonesArray");
      if (!array) {               // only arrays will be read-in
        m_tree->SetBranchStatus(branchName.c_str(), 0);
        branch->ResetAddress();
        delete objectPtr;
        continue;
      }
      TClass* objClass = (static_cast<TClonesArray*>(objectPtr))->GetClass();
      branch->ResetAddress();
      delete objectPtr;

      const std::string className = objClass->GetName();
      if (className.find("Belle2::") == std::string::npos) { // only Belle2 classes
        m_tree->SetBranchStatus(branchName.c_str(), 0);
        continue;
      }

      std::string name = branchName + "_BG";
      bool ok = DataStore::Instance().registerEntry(name, durability, objClass,
                                                    array, storeFlags);
      if (!ok) {
        m_tree->SetBranchStatus(branchName.c_str(), 0);
        continue;
      }
      DataStore::StoreEntry& entry = (map.find(name))->second;
      m_tree->SetBranchAddress(branchName.c_str(), &(entry.object));
      m_storeEntries.push_back(&entry);

    }

    return !m_storeEntries.empty();
  }


} // end Belle2 namespace

