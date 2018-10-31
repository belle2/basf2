/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/rootio/RootOutputModule.h>

#include <framework/io/RootIOUtilities.h>
#include <framework/core/FileCatalog.h>
#include <framework/core/RandomNumbers.h>
#include <framework/database/Database.h>
// needed for complex module parameter
#include <framework/core/ModuleParam.templateDetails.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <TClonesArray.h>

#include <ctime>


using namespace std;
using namespace Belle2;
using namespace RootIOUtilities;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RootOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RootOutputModule::RootOutputModule() : Module(), m_file(0), m_experimentLow(1), m_runLow(0), m_eventLow(0),
  m_experimentHigh(0), m_runHigh(0), m_eventHigh(0)
{
  //Set module properties
  setDescription("Writes DataStore objects into a .root file. Data is stored in a TTree 'tree' for event-dependent and in 'persistent' for peristent data. You can use RootInput to read the files back into basf2.");
  setPropertyFlags(c_Output);

  //Initialization of some member variables
  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_tree[jj] = 0;
  }

  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "Name of the output file. Can be overridden using the -o argument to basf2.",
           string("RootOutput.root"));
  addParam("ignoreCommandLineOverride"  , m_ignoreCommandLineOverride,
           "Ignore override of file name via command line argument -o. Useful if you have multiple output modules in one path.", false);
  addParam("compressionLevel", m_compressionLevel,
           "0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by >50%, higher levels have no noticeable effect. On typical hard disks, disabling compression reduces write time by 10-20 %, but almost doubles read time, so you probably should leave this turned on.",
           m_compressionLevel);
  addParam("compressionAlgorithm", m_compressionAlgorithm,
           "Set the Compression algorithm. Recommended values are 0 for default, 1 for zlib and 4 for lz4", m_compressionAlgorithm);
  addParam("splitLevel", m_splitLevel,
           "Branch split level: determines up to which depth object members will be saved in separate sub-branches in the tree. For arrays or objects with custom streamers, -1 is used instead to ensure the streamers are used. The default (99) usually gives the highest read performance with RootInput.",
           99);
  addParam("updateFileCatalog", m_updateFileCatalog,
           "Flag that specifies whether the file metadata catalog (Belle2FileCatalog.xml) is updated. (You can also set the BELLE2_FILECATALOG environment variable to NONE to get the same effect.)",
           true);

  vector<string> emptyvector;
  addParam(c_SteerBranchNames[0], m_branchNames[0],
           "Names of event durability branches to be saved. Empty means all branches. Objects with c_DontWriteOut flag added here will also be saved. (EventMetaData is always saved)",
           emptyvector);
  addParam(c_SteerBranchNames[1], m_branchNames[1],
           "Names of persistent durability branches to be saved. Empty means all branches. Objects with c_DontWriteOut flag added here will also be saved. (FileMetaData is always saved)",
           emptyvector);
  addParam(c_SteerAdditionalBranchNames[0], m_additionalBranchNames[0],
           "Add additional event branch names without the need to specify all branchnames.",
           emptyvector);
  addParam(c_SteerAdditionalBranchNames[1], m_additionalBranchNames[1],
           "Add additional persistent branch names without the need to specify all branchnames.",
           emptyvector);
  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0],
           "Names of event durability branches NOT to be saved. Branches also in branchNames are not saved.", emptyvector);
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1],
           "Names of persistent durability branches NOT to be saved. Branches also in branchNamesPersistent are not saved.", emptyvector);
  addParam("autoFlushSize", m_autoflush,
           "Value for TTree SetAutoFlush(): a positive value tells ROOT to flush all baskets to disk after n entries, a negative value to flush after -n bytes",
           -10000000);
  addParam("autoSaveSize", m_autosave,
           "Value for TTree SetAutoSave(): a positive value tells ROOT to write the TTree metadata after n entries, a negative value to write the metadata after -n bytes",
           -10000000);
  addParam("basketSize", m_basketsize, "Basketsize for Branches in the Tree in bytes", 32000);
  addParam("additionalDataDescription", m_additionalDataDescription, "Additional dictionary of "
           "name->value pairs to be added to the file metadata to describe the data",
           m_additionalDataDescription);
  addParam("buildIndex", m_buildIndex, "Build Event Index for faster finding of events by exp/run/event number", m_buildIndex);
  addParam("keepParents", m_keepParents, "Keep parents files of input files, input files will not be added as output file's parents",
           m_keepParents);
  addParam("outputSplitSize", m_outputSplitSize, R"DOC(
If given split the output file once the file has reached the given size in MB.
If set the filename will end in ``.f{index:05d}.root``. So if ``outputFileName="RootOutput.root"``
then the files will be named ``RootOutput.f00000.root, RootOutput.f00001.root,
RootOutput.f00002.root, ...``.

All created output files are complete and independent files and can
subsequently processed completely independent.

Note:
  The output files will be approximately of the size given by
  ``outputSplitSize`` but they will be slightly larger since
  additional information has to be written at the end of the file. If necessary
  please account for this. Also, using `buildIndex=False` might be beneficial
  to reduce the overshoot.
)DOC");
}


RootOutputModule::~RootOutputModule() { }

void RootOutputModule::initialize()
{
  //ROOT has a default maximum size of 100GB for trees??? For larger trees it creates a new file and does other things that finally produce crashes.
  //Let's set this to 100PB, that should last a bit longer.
  TTree::SetMaxTreeSize(1000 * 1000 * 100000000000LL);

  //create a file level metadata object in the data store
  m_fileMetaData.registerInDataStore();
  //and make sure we have event meta data
  m_eventMetaData.isRequired();

  //check outputSplitSize
  if (m_outputSplitSize) {
    if (*m_outputSplitSize == 0) B2ERROR("outputSplitSize must be set to a positive value");
    // Warn is splitsize is >= 1TB ... because this seems weirdly like size was given in bytes
    if (*m_outputSplitSize >= 1024*1024) B2WARNING("outputSplitSize set to " << *m_outputSplitSize << " MB, please make sure the units are correct");
    // convert to bytes
    *m_outputSplitSize *= 1024 * 1024;
  }

  getFileNames();
  openFile();
}

void RootOutputModule::openFile()
{
  TDirectory* dir = gDirectory;
  boost::filesystem::path out{m_outputFileName};
  if (m_outputSplitSize) {
    // mangle path: replace extension with fNNNNN
    out.replace_extension((boost::format("f%05d.root") % m_fileIndex).str());
  }
  m_file = TFile::Open(out.c_str(), "RECREATE", "basf2 Event File");
  if (m_file->IsZombie()) {
    //try creating necessary directories
    auto dirpath = out.parent_path();

    if (boost::filesystem::create_directories(dirpath)) {
      B2INFO("Created missing directory " << dirpath << ".");
      //try again
      m_file = TFile::Open(out.c_str(), "RECREATE", "basf2 Event File");
    }

    if (m_file->IsZombie()) {
      B2FATAL("Couldn't open file '" << m_file->GetName() << "' for writing!");
    }
  }
  m_file->SetCompressionAlgorithm(m_compressionAlgorithm);
  m_file->SetCompressionLevel(m_compressionLevel);

  for (int durability = 0; durability < DataStore::c_NDurabilityTypes; durability++) {
    DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));
    set<string> branchList;
    for (auto pair : map)
      branchList.insert(pair.first);
    //skip branches the user doesn't want
    branchList = filterBranches(branchList, m_branchNames[durability], m_excludeBranchNames[durability], durability);

    //create the tree and branches
    m_tree[durability] = new TTree(c_treeNames[durability].c_str(), c_treeNames[durability].c_str());
    m_tree[durability]->SetAutoFlush(m_autoflush);
    m_tree[durability]->SetAutoSave(m_autosave);
    for (auto iter = map.begin(); iter != map.end(); ++iter) {
      const std::string& branchName = iter->first;
      //skip transient entries (allow overriding via branchNames)
      if (iter->second.dontWriteOut
          && find(m_branchNames[durability].begin(), m_branchNames[durability].end(), branchName) == m_branchNames[durability].end()
          && find(m_additionalBranchNames[durability].begin(), m_additionalBranchNames[durability].end(),
                  branchName) ==  m_additionalBranchNames[durability].end())
        continue;
      //skip branches the user doesn't want
      if (branchList.count(branchName) == 0) {
        //make sure FileMetaData and EventMetaData are always included in the output
        if (((branchName != "FileMetaData") || (durability == DataStore::c_Event)) &&
            ((branchName != "EventMetaData") || (durability == DataStore::c_Persistent))) {
          continue;
        }
      }

      TClass* entryClass = iter->second.objClass;

      //I want to do this in the input module, but I apparently I cannot disable reading those branches.
      //isabling reading the branch by not calling SetBranchAddress() for it results in the following crashes. Calling SetBranchStatus(..., 0) doesn't help, either.
      //reported to ROOT devs, let's see if it gets fixed.
      //
      //HasDictionary() is a new function in root 6
      //using it instead of GetClassInfo() avoids  having to parse header files (and
      //the associated memory cost)
      if (!entryClass->HasDictionary()) {
        if (m_fileIndex == 0) {
          B2WARNING("No dictionary found for class " << entryClass->GetName() << ", branch '" << branchName <<
                    "' will not be saved. (This is probably an obsolete class that is still present in the input file.)");
        }
        continue;
      }

      if (!hasStreamer(entryClass)) {
        B2ERROR("The version number in the ClassDef() macro for class " << entryClass->GetName() << " must be at least 1 to enable I/O!");
      }

      int splitLevel = m_splitLevel;
      if (hasCustomStreamer(entryClass)) {
        B2DEBUG(38, entryClass->GetName() << " has custom streamer, setting split level -1 for this branch.");

        splitLevel = -1;
        if (iter->second.isArray) {
          //for arrays, we also don't want TClonesArray to go around our streamer
          static_cast<TClonesArray*>(iter->second.object)->BypassStreamer(kFALSE);
        }
      }
      m_tree[durability]->Branch(branchName.c_str(), &iter->second.object, m_basketsize, splitLevel);
      m_tree[durability]->SetBranchAddress(branchName.c_str(), &iter->second.object);
      m_entries[durability].push_back(&iter->second);
      B2DEBUG(39, "The branch " << branchName << " was created.");

      //Tell DataStore that we are using this entry
      if (m_fileIndex == 0) {
        DataStore::Instance().optionalInput(StoreAccessorBase(branchName, (DataStore::EDurability)durability, entryClass,
                                                              iter->second.isArray));
      }
    }
  }

  dir->cd();
  if (m_outputSplitSize) {
    B2INFO(getName() << ": Opened " << (m_fileIndex > 0 ? "new " : "") << "file for writing" << LogVar("filename", out));
  }
}


void RootOutputModule::event()
{
  // if we closed after last event ... make a new one
  if (!m_file) openFile();

  if (!m_keepParents) {
    if (m_fileMetaData) {
      m_eventMetaData->setParentLfn(m_fileMetaData->getLfn());
    }
  }

  //fill Event data
  fillTree(DataStore::c_Event);

  if (m_fileMetaData) {
    if (m_keepParents) {
      for (int iparent = 0; iparent < m_fileMetaData->getNParents(); iparent++) {
        string lfn = m_fileMetaData->getParent(iparent);
        if (!lfn.empty() && (m_parentLfns.empty() || (m_parentLfns.back() != lfn))) {
          m_parentLfns.push_back(lfn);
        }
      }
    } else {
      string lfn = m_fileMetaData->getLfn();
      if (!lfn.empty() && (m_parentLfns.empty() || (m_parentLfns.back() != lfn))) {
        m_parentLfns.push_back(lfn);
      }
    }
  }

  // keep track of file level metadata
  unsigned long experiment =  m_eventMetaData->getExperiment();
  unsigned long run =  m_eventMetaData->getRun();
  unsigned long event = m_eventMetaData->getEvent();
  if (m_experimentLow > m_experimentHigh) { //starting condition
    m_experimentLow = m_experimentHigh = experiment;
    m_runLow = m_runHigh = run;
    m_eventLow = m_eventHigh = event;
  } else {
    if ((experiment < m_experimentLow) || ((experiment == m_experimentLow) && ((run < m_runLow) || ((run == m_runLow)
                                           && (event < m_eventLow))))) {
      m_experimentLow = experiment;
      m_runLow = run;
      m_eventLow = event;
    }
    if ((experiment > m_experimentHigh) || ((experiment == m_experimentHigh) && ((run > m_runHigh) || ((run == m_runHigh)
                                            && (event > m_eventHigh))))) {
      m_experimentHigh = experiment;
      m_runHigh = run;
      m_eventHigh = event;
    }
  }

  // check if we need to split the file
  if (m_outputSplitSize and (uint64_t)m_file->GetEND() > *m_outputSplitSize) {
    // close file and open new one
    B2INFO(getName() << ": Output size limit reached, closing file ...");
    closeFile();
  }
}

void RootOutputModule::fillFileMetaData()
{
  m_fileMetaData.create(true);

  if (m_tree[DataStore::c_Event]) {
    //create an index for the event tree
    TTree* tree = m_tree[DataStore::c_Event];
    unsigned long numEntries = tree->GetEntries();
    if (m_buildIndex && numEntries > 0) {
      if (numEntries > 10000000) {
        //10M events correspond to about 240MB for the TTreeIndex object. for more than ~45M entries this causes crashes, broken files :(
        B2WARNING("Not building TTree index because of large number of events. The index object would conflict with ROOT limits on object size and cause problems.");
      } else if (tree->GetBranch("EventMetaData")) {
        tree->SetBranchAddress("EventMetaData", 0);
        RootIOUtilities::buildIndex(tree);
      }
    }

    m_fileMetaData->setNEvents(numEntries);
    if (m_experimentLow > m_experimentHigh) {
      //starting condition so apparently no events at all
      m_fileMetaData->setLow(-1, -1, 0);
      m_fileMetaData->setHigh(-1, -1, 0);
    } else {
      m_fileMetaData->setLow(m_experimentLow, m_runLow, m_eventLow);
      m_fileMetaData->setHigh(m_experimentHigh, m_runHigh, m_eventHigh);
    }
  }

  //fill more file level metadata
  m_fileMetaData->setParents(m_parentLfns);
  RootIOUtilities::setCreationData(*m_fileMetaData);
  m_fileMetaData->setRandomSeed(RandomNumbers::getSeed());
  m_fileMetaData->setSteering(Environment::Instance().getSteering());
  m_fileMetaData->setMcEvents(Environment::Instance().getNumberOfMCEvents());
  m_fileMetaData->setDatabaseGlobalTag(Database::getGlobalTag());
  for (const auto& item : m_additionalDataDescription) {
    m_fileMetaData->setDataDescription(item.first, item.second);
  }
  //register the file in the catalog
  if (m_updateFileCatalog) {
    FileCatalog::Instance().registerFile(m_file->GetName(), *m_fileMetaData);
  }

}


void RootOutputModule::terminate()
{
  closeFile();
}

void RootOutputModule::closeFile()
{
  //get pointer to file level metadata
  std::unique_ptr<FileMetaData> old;
  if (m_fileMetaData) old.reset(new FileMetaData(*m_fileMetaData));

  fillFileMetaData();

  //fill Persistent data
  fillTree(DataStore::c_Persistent);

  // restore old file meta data if it existed
  if (old) *m_fileMetaData = *old;
  old.reset();

  //write the trees
  TDirectory* dir = gDirectory;
  m_file->cd();
  for (int durability = 0; durability < DataStore::c_NDurabilityTypes; ++durability) {
    if (m_tree[durability]) {
      B2DEBUG(30, "Write TTree " << c_treeNames[durability]);
      m_tree[durability]->Write(c_treeNames[durability].c_str(), TObject::kWriteDelete);
      delete m_tree[durability];
    }
    m_tree[durability] = nullptr;
  }
  dir->cd();

  delete m_file;
  m_file = nullptr;

  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_entries[jj].clear();
  }
  m_parentLfns.clear();

  // reset some variables
  m_experimentLow = 1;
  m_experimentHigh = 0;
  m_runLow = 0;
  m_runHigh = 0;
  m_eventLow = 0;
  m_eventHigh = 0;
  // and increase index of next file
  ++m_fileIndex;
}


void RootOutputModule::fillTree(DataStore::EDurability durability)
{
  if (!m_tree[durability]) return;

  //Check for entries whose object was not created.
  for (unsigned int i = 0; i < m_entries[durability].size(); i++) {
    if (!m_entries[durability][i]->ptr) {
      //create object owned and deleted by branch
      m_tree[durability]->SetBranchAddress(m_entries[durability][i]->name.c_str(), 0);
    } else {
      m_tree[durability]->SetBranchAddress(m_entries[durability][i]->name.c_str(), &m_entries[durability][i]->object);
    }
  }
  m_tree[durability]->Fill();

  const bool writeError = m_file->TestBit(TFile::kWriteError);
  if (writeError) {
    //m_file deleted first so we have a chance of closing it (though that will probably fail)
    delete m_file;
    B2FATAL("A write error occured while saving '" << m_file->GetName()  << "', please check if enough disk space is available.");
  }
}
