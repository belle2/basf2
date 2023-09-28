/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <boost/python.hpp>

#include <daq/storage/modules/StorageRootOutputModule.h>

#include <framework/io/RootIOUtilities.h>
#include <framework/core/FileCatalog.h>
#include <framework/core/MetadataService.h>
#include <framework/core/RandomNumbers.h>
#include <framework/database/Database.h>
// needed for complex module parameter
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/utilities/EnvironmentVariables.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <TClonesArray.h>

#include <nlohmann/json.hpp>

#include <memory>
#include <regex>
#include <filesystem>

// For online storage
#include <boost/date_time/posix_time/posix_time.hpp>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/database/DBHandlerException.h>
#include <daq/slc/base/StringUtil.h>


using namespace std;
using namespace Belle2;
using namespace RootIOUtilities;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StorageRootOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

StorageRootOutputModule::StorageRootOutputModule() : Module(), m_file(nullptr), m_tree{0}, m_experimentLow(1), m_runLow(0),
  m_eventLow(0), m_experimentHigh(0), m_runHigh(0), m_eventHigh(0)
{
  //Set module properties
  setDescription("Writes DataStore objects into a .root file. Data is stored in a TTree 'tree' for event-dependent and in 'persistent' for peristent data. You can use RootInput to read the files back into basf2.");
  setPropertyFlags(c_Output);

  //Parameter definition
  addParam("outputFileName", m_outputFileName, "Name of the output file. Can be overridden using the -o argument to basf2.",
           string("RootOutput.root"));
  addParam("ignoreCommandLineOverride", m_ignoreCommandLineOverride,
           "Ignore override of file name via command line argument -o. Useful if you have multiple output modules in one path.", false);
  addParam("compressionLevel", m_compressionLevel,
           "0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by >50%, higher levels have no noticeable effect. On typical hard disks, disabling compression reduces write time by 10-20 %, but almost doubles read time, so you probably should leave this turned on.",
           m_compressionLevel);
  addParam("compressionAlgorithm", m_compressionAlgorithm,
           "Set the Compression algorithm. Recommended values are 0 for default, 1 for zlib and 4 for lz4\n\n"
           ".. versionadded:: release-03-00-00", m_compressionAlgorithm);
  addParam("splitLevel", m_splitLevel,
           "Branch split level: determines up to which depth object members will be saved in separate sub-branches in the tree. For arrays or objects with custom streamers, -1 is used instead to ensure the streamers are used. The default (99) usually gives the highest read performance with RootInput.",
           99);
  addParam("updateFileCatalog", m_updateFileCatalog, R"DOC(
Flag that specifies whether the file metadata catalog is updated or created.
This is only necessary in special cases and can always be done afterwards using
``b2file-catalog-add filename.root``"

(You can also set the ``BELLE2_FILECATALOG`` environment variable to NONE to get
the same effect as setting this to false))DOC", false);

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
If set the filename will end in ``.f{index:05d}.root``. So if for example
``outputFileName`` is set to "RootOutput.root" then the files will be named
``RootOutput.f00000.root``, ``RootOutput.f00001.root``,
``RootOutput.f00002.root``, ...

All created output files are complete and independent files and can
subsequently processed completely independent.

Note:
  The output files will be approximately of the size given by
  ``outputSplitSize`` but they will be slightly larger since
  additional information has to be written at the end of the file. If necessary
  please account for this. Also, using ``buildIndex=False`` might be beneficial
  to reduce the overshoot.

Warning:
  This will set the amount of generated events stored in the file metadata to
  zero as it is not possible to determine which fraction ends up in which
  output file.

.. versionadded:: release-03-00-00
)DOC", m_outputSplitSize);

  // Parameters for online storage
  addParam("runType", m_runType, "Run type", string("null"));
  addParam("HLTName", m_HLTName, "HLT name", string("HLT00"));
  addParam("nDisk", m_nDisk, "The number of paratitions", 3);
  addParam("skipFirstEvent", m_firstEvent, "Boolean to skip the first event or not. "
           "If the module is used inside the hbasf2, like HLT storage, the first event need to be skipped.", m_firstEvent);
  addParam("ramdiskBuffer", m_ramdiskBuffer, "Boolean to make small ramdisk buffer setup. "
           "If this is false, assuming the buffer disks are large SSD.", m_ramdiskBuffer);
}


StorageRootOutputModule::~StorageRootOutputModule() = default;

void StorageRootOutputModule::initialize()
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

  // Now check if the file has a protocol like file:// or http:// in front
  std::regex protocol("^([A-Za-z]*)://");
  if(std::smatch m; std::regex_search(m_outputFileName, m, protocol)) {
    if(m[1] == "file") {
      // file protocol: treat as local and just remove it from the filename
      m_outputFileName = std::regex_replace(m_outputFileName, protocol, "");
    } else {
      // any other protocol: not local, don't create directories
      m_regularFile = false;
    }
  }
  // For online storage
  // Do not open file in basf2 initialize
  // openFile();

  ConfigFile config("slowcontrol");
  PostgreSQLInterface *db = new PostgreSQLInterface(config.get("database.host"),
                                                    config.get("database.dbname"),
                                                    config.get("database.user"),
                                                    config.get("database.password"),
                                                    config.getInt("database.port"));
  m_db = db;
}

void StorageRootOutputModule::openFile()
{
  // Since we open a new file, we also have to reset the number of full events
  m_nFullEvents = 0;
  // Continue with opening the file
  TDirectory* dir = gDirectory;
  std::filesystem::path out{m_outputFileName};
  if (m_outputSplitSize) {
    // Mangle the filename to add the fNNNNN part. However we need to be
    // careful since the file name could be non-local and have some options or
    // anchor information attached (like
    // http://mydomain.org/filename.root?foo=bar#baz). So use "TUrl" *sigh* to
    // do the parsing and only replace the extension of the file part.
    TUrl fileUrl(m_outputFileName.c_str(), m_regularFile);
    std::filesystem::path file{fileUrl.GetFile()};
    file.replace_extension((boost::format("f%05d.root") % m_fileIndex).str());
    fileUrl.SetFile(file.c_str());
    // In case of regular files we don't want the protocol or anything, just the file
    out = m_regularFile? fileUrl.GetFileAndOptions() : fileUrl.GetUrl();
  }

  // For online storage buffer disks
  std::filesystem::path out_notmp = out;
  out = std::filesystem::path{std::string("/buffer") + out.generic_string()};

  m_file = TFile::Open(out.c_str(), "RECREATE", "basf2 Event File");
  if ((!m_file || m_file->IsZombie()) && m_regularFile) {
    //try creating necessary directories since this is a local file
    auto dirpath = out.parent_path();

    if (std::filesystem::create_directories(dirpath)) {
      B2INFO("Created missing directory " << dirpath << ".");
      //try again
      m_file = TFile::Open(out.c_str(), "RECREATE", "basf2 Event File");
    }

  }

  // Retry file open for online storage, with ramdisk, may not be needed
  int openCounter = 0;
  while (!m_file || m_file->IsZombie()) {
    m_file = TFile::Open(out.c_str(), "RECREATE", "basf2 Event File");
    B2INFO("process(" << m_processNumber << "), Try open: " << ++openCounter);
  }

  if (!m_file || m_file->IsZombie()) {
    B2FATAL("Couldn't open file " << out << " for writing!");
  }
  m_file->SetCompressionAlgorithm(m_compressionAlgorithm);
  m_file->SetCompressionLevel(m_compressionLevel);

  for (int durability = 0; durability < DataStore::c_NDurabilityTypes; durability++) {
    DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));
    set<string> branchList;
    for (const auto& pair : map)
      branchList.insert(pair.first);
    //skip branches the user doesn't want
    branchList = filterBranches(branchList, m_branchNames[durability], m_excludeBranchNames[durability], durability);

    //create the tree and branches
    m_tree[durability] = new TTree(c_treeNames[durability].c_str(), c_treeNames[durability].c_str());
    m_tree[durability]->SetAutoFlush(m_autoflush);
    m_tree[durability]->SetAutoSave(m_autosave);
    for (auto & iter : map) {
      const std::string& branchName = iter.first;
      //skip transient entries (allow overriding via branchNames)
      if (iter.second.dontWriteOut
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

      // Warn for anything other than FileMetaData and ProcessStatistics ...
      if(durability == DataStore::c_Persistent and m_outputSplitSize and m_fileIndex==0 and
         (branchName != "FileMetaData" and branchName != "ProcessStatistics")) {
        B2WARNING("Persistent branches might not be stored as expected when splitting the output by size" << LogVar("branch", branchName));
      }

      TClass* entryClass = iter.second.objClass;

      //I want to do this in the input module, but I apparently I cannot disable reading those branches.
      //isabling reading the branch by not calling SetBranchAddress() for it results in the following crashes. Calling SetBranchStatus(..., 0) doesn't help, either.
      //reported to ROOT devs, let's see if it gets fixed.
      //
      //HasDictionary() is a new function in root 6
      //using it instead of GetClassInfo() avoids  having to parse header files (and
      //the associated memory cost)
      if (!entryClass->HasDictionary()) {
        if (m_fileIndex == 0) {
          B2WARNING("No dictionary found, object will not be saved  (This is probably an obsolete class that is still present in the input file.)"
                    << LogVar("class", entryClass->GetName()) << LogVar("branch", branchName));
        }
        continue;
      }

      if (!hasStreamer(entryClass)) {
        B2ERROR("The version number in the ClassDef() macro must be at least 1 to enable I/O!" << LogVar("class", entryClass->GetName()));
      }

      int splitLevel = m_splitLevel;
      if (hasCustomStreamer(entryClass)) {
        B2DEBUG(38, "Class has custom streamer, setting split level -1 for this branch." << LogVar("class", entryClass->GetName()));

        splitLevel = -1;
        if (iter.second.isArray) {
          //for arrays, we also don't want TClonesArray to go around our streamer
          static_cast<TClonesArray*>(iter.second.object)->BypassStreamer(kFALSE);
        }
      }
      m_tree[durability]->Branch(branchName.c_str(), &iter.second.object, m_basketsize, splitLevel);
      m_entries[durability].push_back(&iter.second);
      B2DEBUG(39, "The branch " << branchName << " was created.");

      //Tell DataStore that we are using this entry
      if (m_fileIndex == 0) {
        DataStore::Instance().optionalInput(StoreAccessorBase(branchName, (DataStore::EDurability)durability, entryClass,
                                                              iter.second.isArray));
      }
    }
  }

  dir->cd();
  if (m_outputSplitSize) {
    B2INFO(getName() << ": Opened " << (m_fileIndex > 0 ? "new " : "") << "file for writing" << LogVar("filename", out));
  }

  // Insert file entry into the DAQ DB while open new file, online storage feature
  try {
    m_db->connect();
    m_db->execute("INSERT INTO datafiles_root "
                  "(name, path, host, runtype, expno, runno, compression_level, compression_algorithm, time_open, "
                  "size, nevents, nfullevents, correct_close, renamed, used_for_merge, removed) "
                  "VALUES ('%s', '%s', '%s', '%s', %d, %d, %d, %d, '%s', 0, 0, 0, FALSE, FALSE, FALSE, FALSE);",
                  out_notmp.filename().c_str(), out_notmp.c_str(), m_HLTName.c_str(),
                  m_runType.c_str(), m_expno, m_runno,
                  m_compressionLevel, m_compressionAlgorithm,
                  (boost::posix_time::to_iso_extended_string(boost::posix_time::microsec_clock::universal_time())+std::string("+9")).c_str());
  } catch (const DBHandlerException &e) {
    B2WARNING(e.what());
  } 
}


void StorageRootOutputModule::event()
{
  // Many safety guards and features for online storage
  // Skip the first event which is the ZMQ init message
  if (m_firstEvent) {
    m_firstEvent = false;
    return;
  }

  // Exp/run number should be increased
  if ((m_expno > m_eventMetaData->getExperiment()) ||
      (m_expno == m_eventMetaData->getExperiment() && m_runno > m_eventMetaData->getRun())) { 
    return;
  }

  // Close file and set file index to 0 if exp/run number is changed
  if (m_file) {
    if (m_expno != m_eventMetaData->getExperiment() || m_runno != m_eventMetaData->getRun()) {
      closeFile();
      while (m_file) closeFile(); // I hope that this will not be failed
      m_fileIndex = 0;
    }
  }
  
  // Open file with automatic naming
  if (!m_file) {
    m_expno = m_eventMetaData->getExperiment();
    m_runno = m_eventMetaData->getRun();
    m_processNumber = atoi(getName().substr(0, getName().find(std::string("_"))).c_str());
    m_outputFileName = StringUtil::form("/rawdata/disk%02d/belle/Raw/%4.4d/%5.5d/%s.%4.4d.%5.5d.%s.p%02d.root", 
                                        (m_processNumber%m_nDisk)+1, m_expno, m_runno, m_runType.c_str(), 
                                        m_expno, m_runno, m_HLTName.c_str(), m_processNumber);
  }

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

  // check if the event is a full event or not: if yes, increase the counter
  if (m_eventMetaData->getErrorFlag() == 0) // no error flag -> this is a full event
    m_nFullEvents++;

  // check if we need to split the file
  if (m_outputSplitSize and (uint64_t)m_file->GetEND() > *m_outputSplitSize) {
    // close file and open new one
    B2INFO(getName() << ": Output size limit reached, closing file ...");
    closeFile();
    // Introduce while for online storage (can be removed?)
    while (m_file) closeFile();
  }
}

void StorageRootOutputModule::fillFileMetaData()
{
  bool isMC = (m_fileMetaData) ? m_fileMetaData->isMC() : true;

  // For online storage, force to declareRealData()
  // I wonder why the no file meta data is associated with isMC == true
  isMC = false;

  m_fileMetaData.create(true);
  if (!isMC) m_fileMetaData->declareRealData();

  if (m_tree[DataStore::c_Event]) {
    //create an index for the event tree
    TTree* tree = m_tree[DataStore::c_Event];
    unsigned long numEntries = tree->GetEntries();
    m_fileMetaData->setNFullEvents(m_nFullEvents);
    if (m_buildIndex && numEntries > 0) {
      if (numEntries > 10000000) {
        //10M events correspond to about 240MB for the TTreeIndex object. for more than ~45M entries this causes crashes, broken files :(
        B2WARNING("Not building TTree index because of large number of events. The index object would conflict with ROOT limits on object size and cause problems.");
      } else if (tree->GetBranch("EventMetaData")) {
        tree->SetBranchAddress("EventMetaData", nullptr);
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
  auto mcEvents = Environment::Instance().getNumberOfMCEvents();
  if(m_outputSplitSize and mcEvents > 0) {
    if(m_fileIndex == 0) B2WARNING("Number of MC Events cannot be saved when splitting output files by size, setting to 0");
    mcEvents = 0;
  }
  m_fileMetaData->setMcEvents(mcEvents);
  m_fileMetaData->setDatabaseGlobalTag(Database::Instance().getGlobalTags());
  for (const auto& item : m_additionalDataDescription) {
    m_fileMetaData->setDataDescription(item.first, item.second);
  }
  // Set the LFN to the filename: if it's a URL to directly, otherwise make sure it's absolute
  std::string lfn = m_file->GetName();
  if(m_regularFile) {
    lfn = std::filesystem::absolute(lfn).string();
  }
  // Format LFN if BELLE2_LFN_FORMATSTRING is set
  std::string format = EnvironmentVariables::get("BELLE2_LFN_FORMATSTRING", "");
  if (!format.empty()) {
    auto format_filename = boost::python::import("B2Tools.format").attr("format_filename");
    lfn = boost::python::extract<std::string>(format_filename(format, m_outputFileName, m_fileMetaData->getJsonStr()));
  }
  m_fileMetaData->setLfn(lfn);
  //register the file in the catalog
  if (m_updateFileCatalog) {
    FileCatalog::Instance().registerFile(m_file->GetName(), *m_fileMetaData);
  }
  m_outputFileMetaData = *m_fileMetaData;
}


void StorageRootOutputModule::terminate()
{
  closeFile();
  // Introduce while for online storage (can be removed?)
  while (m_file) closeFile();
}

void StorageRootOutputModule::closeFile()
{
  if(!m_file) return;
  //get pointer to file level metadata
  std::unique_ptr<FileMetaData> old;
  if (m_fileMetaData) old = std::make_unique<FileMetaData>(*m_fileMetaData);

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

  const std::string filename = m_file->GetName();
  if (m_outputSplitSize) {
    B2INFO(getName() << ": Finished writing file." << LogVar("filename", filename));
  }

  // Before deleting m_file, store file list in online storage file list DB table
  const std::filesystem::path filename_path{filename};
  std::string filename_notmp = m_file->GetName();
  filename_notmp.erase(0, 7); // remove "/buffer" in front of full path
  const std::filesystem::path filename_notmp_path{filename_notmp};
  try {
    m_db->connect();
    m_db->execute("UPDATE datafiles_root SET "
                  "correct_close = TRUE, "
                  "will_merge = %s ,"
                  "nevents = %lu, "
                  "nfullevents = %lu, "
                  "size = %lu, "
                  "time_close = '%s' "
                  "WHERE name = '%s' AND host = '%s';",
                  ((boost::optional<uint64_t>)(m_file->GetSize()*2) < m_outputSplitSize || m_ramdiskBuffer) ? "true" : "false",
                  m_outputFileMetaData.getNEvents(), m_outputFileMetaData.getNFullEvents(), m_file->GetSize(),
                  (boost::posix_time::to_iso_extended_string(boost::posix_time::microsec_clock::universal_time())+std::string("+9")).c_str(),
                  filename_path.filename().c_str(), m_HLTName.c_str());
  } catch (const DBHandlerException &e) {
    B2WARNING(e.what());
  }
  m_db->close();

  delete m_file;
  m_file = nullptr;

  // and now add it to the metadata service as it's fully written
  MetadataService::Instance().addRootOutputFile(filename, &m_outputFileMetaData);

  // reset some variables
  for (auto & entry : m_entries) {
    entry.clear();
  }
  m_parentLfns.clear();
  m_experimentLow = 1;
  m_experimentHigh = 0;
  m_runLow = 0;
  m_runHigh = 0;
  m_eventLow = 0;
  m_eventHigh = 0;
  // and increase index of next file
  ++m_fileIndex;

  // Call system(/usr/bin/rsync) for online storage ramdisk buffer cleanup
  if (m_ramdiskBuffer) {
    const std::string rsync_cmd = std::string("/usr/bin/rsync -a --remove-source-files --recursive ") + filename + std::string(" ") + filename_notmp_path.parent_path().generic_string() + std::string("/ &");
    B2INFO(getName() << ": system(" << rsync_cmd << ")");
    system(rsync_cmd.c_str());
  }
}


void StorageRootOutputModule::fillTree(DataStore::EDurability durability)
{
  if (!m_tree[durability]) return;

  TTree& tree = *m_tree[durability];
  for(auto* entry: m_entries[durability]) {
    // Check for entries whose object was not created and mark them as invalid. 
    // We still have to write them in the file due to the structure we have. This could be done better 
    if (!entry->ptr) {
      entry->object->SetBit(kInvalidObject);
    }
    //FIXME: Do we need this? in theory no but it crashes in parallel processing otherwise ¯\_(ツ)_/¯
    tree.SetBranchAddress(entry->name.c_str(), &entry->object);
  }
  tree.Fill();
  for (auto* entry: m_entries[durability]) {
    entry->object->ResetBit(kInvalidObject);
  }

  const bool writeError = m_file->TestBit(TFile::kWriteError);
  if (writeError) {
    //m_file deleted first so we have a chance of closing it (though that will probably fail)
    const std::string filename = m_file->GetName();
    delete m_file;
    B2FATAL("A write error occured while saving '" << filename << "', please check if enough disk space is available.");
  }
}
