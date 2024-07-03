/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <background/modules/BGOverlayInput/BGOverlayInputModule.h>

/* Basf2 headers. */
#include <framework/dataobjects/BackgroundInfo.h>
#include <framework/dataobjects/BackgroundMetaData.h>
#include <framework/dataobjects/FileMetaData.h>
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

REG_MODULE(BGOverlayInput);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BGOverlayInputModule::BGOverlayInputModule() : Module()

{
  // module description
  setDescription("Input for BG overlay, either in form of Digits or raw data. For run-dependent MC (experiments 1 to 999) "
                 "the overlay samples corresponding to the simulated run are automatically selected from the input list "
                 "at each beginRun(), enabling production of multiple runs in a single job. "
                 "This feature can be turned off by setting ignoreRunNumbers to True.");

  // Add parameters
  addParam("inputFileNames", m_inputFileNames,
           "List of files with measured beam background ");
  addParam("extensionName", m_extensionName,
           "Name added to default branch names", string("_beamBG"));
  addParam("bkgInfoName", m_BackgroundInfoInstanceName, "Name of the BackgroundInfo StoreObjPtr", string(""));
  addParam("skipExperimentCheck", m_skipExperimentCheck,
           "If True, skip the check on the experiment number consistency between the basf2 process and the beam background files; "
           "for experiments 1 to 999 ignore also run numbers. "
           "By default, it is set to false, since the check should be skipped only by experts.",
           false);
  addParam("ignoreRunNumbers", m_ignoreRunNumbers,
           "If True, ignore run numbers in case of run-dependend MC (experiments 1 to 999).",
           false);
}

BGOverlayInputModule::~BGOverlayInputModule()
{
}

void BGOverlayInputModule::initialize()
{
  if (m_skipExperimentCheck) {
    B2WARNING(R"RAW(The BGOverlayInput module will skip the check on the experiment number
    consistency between the basf2 process and the beam background files.
    It will also ingnore run numbers in case of run-dependent MC.

    This should be done only if you are extremely sure about what you are doing.

    Be aware that you are not protected by the possible usage of beam background
    files not suitabile for the experiment number you selected.)RAW");
  }

  // expand possible wildcards
  m_inputFileNames = RootIOUtilities::expandWordExpansions(m_inputFileNames);
  if (m_inputFileNames.empty()) {
    B2FATAL("No valid files specified!");
  }

  // get the experiment number from the EventMetaData
  int experiment = m_eventMetaData->getExperiment();

  // check files
  for (const string& fileName : m_inputFileNames) {
    try {
      RootIOUtilities::RootFileInfo fileInfo{fileName};
      const std::set<std::string>& branchNames = fileInfo.getBranchNames(true);
      if (branchNames.count("BackgroundMetaData")) {
        B2FATAL("The BG sample used is aimed for BG mixing, not for BG overlay."
                << LogVar("File name", fileName));
      }
      if (not m_skipExperimentCheck) {
        const FileMetaData& fileMetaData = fileInfo.getFileMetaData();
        // we assume lowest experiment/run numbers are enough
        if (experiment != fileMetaData.getExperimentLow()) {
          B2FATAL("The BG sample used is aimed for a different experiment number. Please check what you are doing."
                  << LogVar("File name", fileName)
                  << LogVar("Experiment number of the basf2 process", experiment)
                  << LogVar("Experiment number of the BG file", fileMetaData.getExperimentLow()));
        }
        if (not m_ignoreRunNumbers) {
          m_runFileNamesMap[fileMetaData.getRunLow()].push_back(fileName);
        }
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

  // make a chain including all overlay files; get number of entries
  m_tree = new TChain(RootIOUtilities::c_treeNames[DataStore::c_Event].c_str());
  for (const string& fileName : m_inputFileNames) {
    m_tree->AddFile(fileName.c_str());
  }
  m_numEvents = m_tree->GetEntries();
  if (m_numEvents == 0) B2ERROR(RootIOUtilities::c_treeNames[DataStore::c_Event] << " has no entries");

  // register selected branches to DataStore
  bool ok = registerBranches();
  if (!ok) {
    B2ERROR("No branches found to be registered and connected");
  }

  // add BackgroundInfo to persistent tree
  StoreObjPtr<BackgroundInfo> bkgInfo(m_BackgroundInfoInstanceName, DataStore::c_Persistent);
  bkgInfo.registerInDataStore();
  bkgInfo.create();
  bkgInfo->setMethod(BackgroundInfo::c_Overlay);
  bkgInfo->setExtensionName(m_extensionName);

  // set flag for steering between run-independent (false) and run-dependent (true) overlay
  m_runByRun = experiment > 0 and experiment < 1000 and not m_runFileNamesMap.empty();
  if (m_runByRun) return;

  // run-independent overlay: choose randomly the first event and connect branches
  m_firstEvent = gRandom->Integer(m_numEvents);
  B2INFO("BGOverlayInput: run-independent overlay starting with event " << m_firstEvent << " of " << m_numEvents);
  m_eventCount = m_firstEvent;
  m_start = true;
  connectBranches();

  // add description to BackgroundInfo
  BackgroundInfo::BackgroundDescr descr;
  descr.tag = BackgroundMetaData::bg_other;
  descr.type = std::string(BackgroundMetaData::getDefaultBackgroundOverlayType());
  descr.fileNames = m_inputFileNames;
  descr.numEvents = m_numEvents;
  m_index = bkgInfo->appendBackgroundDescr(descr);
}


void BGOverlayInputModule::beginRun()
{
  if (not m_runByRun) return;

  // run-dependent overlay: delete the old one and make new chain with overlay files for the current run; get number of entries
  if (m_tree) delete m_tree;
  m_tree = new TChain(RootIOUtilities::c_treeNames[DataStore::c_Event].c_str());
  int run = m_eventMetaData->getRun();
  std::vector<std::string> inputFileNames;
  for (const string& fileName : m_runFileNamesMap[run]) {
    auto status = m_tree->AddFile(fileName.c_str());
    if (status > 0) inputFileNames.push_back(fileName);
  }
  if (inputFileNames.empty()) B2FATAL("No overlay files for run " << run);

  m_numEvents = m_tree->GetEntries();
  if (m_numEvents == 0) B2FATAL(RootIOUtilities::c_treeNames[DataStore::c_Event] << " has no entries for run " << run);

  // choose randomly the first event and connect branches
  m_firstEvent = gRandom->Integer(m_numEvents);
  B2INFO("BGOverlayInput: run " << run << " starting with event " << m_firstEvent << " of " << m_numEvents);
  m_eventCount = m_firstEvent;
  m_start = true;
  connectBranches();

  // add description for this run to BackgroundInfo
  BackgroundInfo::BackgroundDescr descr;
  descr.tag = BackgroundMetaData::bg_other;
  descr.type = std::string(BackgroundMetaData::getDefaultBackgroundOverlayType());
  descr.fileNames = inputFileNames;
  descr.numEvents = m_numEvents;
  descr.runNumber = run;
  StoreObjPtr<BackgroundInfo> bkgInfo(m_BackgroundInfoInstanceName, DataStore::c_Persistent);
  m_index = bkgInfo->appendBackgroundDescr(descr);
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

  if (m_tree) delete m_tree;

}


bool BGOverlayInputModule::registerBranches()
{

  auto durability = DataStore::c_Event;
  auto storeFlags = DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered;
  auto& map = DataStore::Instance().getStoreEntryMap(durability);

  // StoreObjPointers have to be included explicitly
  const std::set<std::string> objPtrNames = {"Belle2::ECLWaveforms", "Belle2::PXDInjectionBGTiming", "Belle2::EventLevelTriggerTimeInfo",
                                             "Belle2::TRGSummary"
                                            };

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
        m_otherBranchNames.push_back(branchName);
        continue;
      }

      std::string name = branchName + m_extensionName;
      bool ok = DataStore::Instance().registerEntry(name, durability, objClass,
                                                    true, storeFlags);
      if (!ok) {
        m_otherBranchNames.push_back(branchName);
        continue;
      }
      DataStore::StoreEntry& entry = (map.find(name))->second;
      m_branchNames.push_back(branchName);
      m_storeEntries.push_back(&entry);
    } else if (objPtrNames.find(objName) != objPtrNames.end()) {
      std::string name = branchName;
      if (objName == "Belle2::TRGSummary") name += m_extensionName; // to distinguish it from the one provided by simulation
      bool ok = DataStore::Instance().registerEntry(name, durability, objectPtr->IsA(),
                                                    false, storeFlags);
      branch->ResetAddress();
      delete objectPtr;

      if (!ok) {
        m_otherBranchNames.push_back(branchName);
        continue;
      }
      DataStore::StoreEntry& entry = (map.find(name))->second;
      m_branchNames.push_back(branchName);
      m_storeEntries.push_back(&entry);
    } else {
      m_otherBranchNames.push_back(branchName);
      branch->ResetAddress();
      delete objectPtr;
    }

  }

  return !m_storeEntries.empty();
}


void BGOverlayInputModule::connectBranches()
{
  for (size_t i = 0; i < m_storeEntries.size(); i++) {
    m_tree->SetBranchAddress(m_branchNames[i].c_str(), &(m_storeEntries[i]->object));
  }

  for (const auto& branchName : m_otherBranchNames) {
    m_tree->SetBranchStatus(branchName.c_str(), 0);
  }
}
