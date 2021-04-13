/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own include. */
#include <background/modules/BGOverlayInput/BGOverlayInputModule.h>

/* Belle 2 headers. */
#include <framework/dataobjects/BackgroundInfo.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/io/RootFileInfo.h>
#include <framework/io/RootIOUtilities.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TClonesArray.h>
#include <TFile.h>
#include <TRandom.h>

/* C++ headers. */
#include <set>

using namespace std;
using namespace Belle2;

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
  addParam("extensionName", m_extensionName,
           "Name added to default branch names", string("_beamBG"));
  addParam("bkgInfoName", m_BackgroundInfoInstanceName, "Name of the BackgroundInfo StoreObjPtr", string(""));
  addParam("skipExperimentCheck", m_skipExperimentCheck,
           "If true, skip the check on the experiment number consistency between the basf2 process and the beam background files. By default, it is set to false, since the check should be skipped only by experts.",
           false);
}

BGOverlayInputModule::~BGOverlayInputModule()
{
}

void BGOverlayInputModule::initialize()
{
  if (m_skipExperimentCheck)
    B2WARNING(R"RAW(The BGOverlayInput module will skip the check on the experiment number
    consistency between the basf2 process and the beam background files.

    This should be done only if you are extremely sure about what you are doing.

    Be aware that you are not protected by the possible usage of beam background
    files not suitabile for the experiment number you selected.)RAW");

  // expand possible wildcards
  m_inputFileNames = RootIOUtilities::expandWordExpansions(m_inputFileNames);
  if (m_inputFileNames.empty()) {
    B2FATAL("No valid files specified!");
  }

  // get the experiment number from the EventMetaData
  StoreObjPtr<EventMetaData> eventMetaData;
  int experiment{eventMetaData->getExperiment()};

  // check files
  for (const string& fileName : m_inputFileNames) {
    try {
      RootIOUtilities::RootFileInfo fileInfo{fileName};
      const std::set<std::string>& branchNames = fileInfo.getBranchNames(true);
      if (branchNames.count("BackgroundMetaData"))
        B2FATAL("The BG sample used is aimed for BG mixing, not for BG mixing."
                << LogVar("File name", fileName));
      if (not m_skipExperimentCheck) {
        const FileMetaData& fileMetaData = fileInfo.getFileMetaData();
        // we assume lowest experiment number is enough
        if (experiment != fileMetaData.getExperimentLow())
          B2FATAL("The BG sample used is aimed for a different experiment number. Please check what you are doing."
                  << LogVar("File name", fileName)
                  << LogVar("Experiment number of the basf2 process", experiment)
                  << LogVar("Experiment number of the BG file", fileMetaData.getExperimentLow()));
      }
    } catch (const std::invalid_argument& e) {
      B2FATAL("One of the BG files can not be opened."
              << LogVar("File name", fileName));
    } catch (const std::runtime_error& e) {
      B2FATAL("Something went wrong with one of the BG files."
              << LogVar("File name", fileName)
              << LogVar("Issue", e.what()));
    }
  }
  TDirectory* dir = gDirectory;
  dir->cd();

  // get event TTree
  m_tree = new TChain(RootIOUtilities::c_treeNames[DataStore::c_Event].c_str());
  for (const string& fileName : m_inputFileNames) {
    m_tree->AddFile(fileName.c_str());
  }
  m_numEvents = m_tree->GetEntries();
  if (m_numEvents == 0) B2ERROR(RootIOUtilities::c_treeNames[DataStore::c_Event] << " has no entires");
  m_firstEvent = gRandom->Integer(m_numEvents);
  B2INFO("BGOverlayInput: starting with event " << m_firstEvent);
  m_eventCount = m_firstEvent;

  // connect selected branches to DataStore
  bool ok = connectBranches();
  if (!ok) {
    B2ERROR("No branches found to be connected");
  }

  // add BackgroundInfo to persistent tree
  StoreObjPtr<BackgroundInfo> bkgInfo(m_BackgroundInfoInstanceName, DataStore::c_Persistent);
  bkgInfo.registerInDataStore();
  bkgInfo.create();
  bkgInfo->setMethod(BackgroundInfo::c_Overlay);
  BackgroundInfo::BackgroundDescr descr;
  descr.tag = BackgroundMetaData::bg_other;
  descr.type = string("RandomTrigger");
  descr.fileNames = m_inputFileNames;
  descr.numEvents = m_numEvents;
  m_index = bkgInfo->appendBackgroundDescr(descr);
  bkgInfo->setExtensionName(m_extensionName);

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

  if (m_eventCount == m_firstEvent and !m_start) {
    B2INFO("BGOverlayInput: events for BG overlay will be re-used");
    bkgInfo->incrementReusedCounter(m_index);
  }
  m_start = false;

  m_tree->GetEntry(m_eventCount);
  m_eventCount++;
  if (m_eventCount >= m_numEvents) {
    m_eventCount = 0;
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
    std::string objName = branch->GetClassName();

    if (objName == "TClonesArray") {
      TClass* objClass = (static_cast<TClonesArray*>(objectPtr))->GetClass();
      branch->ResetAddress();
      delete objectPtr;

      const std::string className = objClass->GetName();
      if (className.find("Belle2::") == std::string::npos) { // only Belle2 classes
        m_tree->SetBranchStatus(branchName.c_str(), 0);
        continue;
      }

      std::string name = branchName + m_extensionName;
      bool ok = DataStore::Instance().registerEntry(name, durability, objClass,
                                                    true, storeFlags);
      if (!ok) {
        m_tree->SetBranchStatus(branchName.c_str(), 0);
        continue;
      }
      DataStore::StoreEntry& entry = (map.find(name))->second;
      m_tree->SetBranchAddress(branchName.c_str(), &(entry.object));
      m_storeEntries.push_back(&entry);
    } else if (objName == "Belle2::ECLWaveforms" or objName == "Belle2::PXDInjectionBGTiming") {
      std::string name = branchName;// + m_extensionName;
      bool ok = DataStore::Instance().registerEntry(name, durability, objectPtr->IsA(),
                                                    false, storeFlags);
      branch->ResetAddress();
      delete objectPtr;

      if (!ok) {
        m_tree->SetBranchStatus(branchName.c_str(), 0);
        continue;
      }
      DataStore::StoreEntry& entry = (map.find(name))->second;
      m_tree->SetBranchAddress(branchName.c_str(), &(entry.object));
      m_storeEntries.push_back(&entry);

    } else {
      m_tree->SetBranchStatus(branchName.c_str(), 0);
      branch->ResetAddress();
      delete objectPtr;
    }

  }

  return !m_storeEntries.empty();
}

