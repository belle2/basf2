/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/core/Environment.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/dataobjects/EventMetaData.h>

#include <string>
#include <vector>
#include <set>

#include <TChain.h>
#include <TFile.h>


namespace Belle2 {
  /** Module to read TTree data from file into the data store.
   *
   *  For more information consult the basf2 Software Portal confluence page.
   *  You can specify different TTrees for different durabilities, and specify
   *  if only specific branches should be read (branchNames), or if some should
   *  be excluded (excludeBranchNames).
   *
   *  The module supports reading from multiple files using TChain, entries will
   *  be read in the order the files are specified.
   *
   *  @sa DataStore::EDurability
  */
  class RootInputModule : public Module {
  public:

    /** Constructor. */
    RootInputModule();

    /** Destructor. */
    virtual ~RootInputModule();

    /** Initialize the Module */
    virtual void initialize() override;

    /** Running over all events */
    virtual void event() override;

    /** Is called at the end of your Module */
    virtual void terminate() override;

    /** Get list of input files, taking -i command line overrides into account. */
    virtual std::vector<std::string> getFileNames(bool outputFiles = false) override
    {
      B2ASSERT("RootInput is not an output module", !outputFiles);
      std::vector<std::string> inputFiles = Environment::Instance().getInputFilesOverride();
      if (!m_ignoreCommandLineOverride and !inputFiles.empty()) {
        return inputFiles;
      }
      inputFiles = m_inputFileNames;
      if (!m_inputFileName.empty())
        inputFiles.push_back(m_inputFileName);
      return inputFiles;
    }

  protected:


  private:
    typedef std::vector<DataStore::StoreEntry*> StoreEntries;   /**< Vector of entries in the data store. */

    /** Actually performs the reading from the tree */
    void readTree();

    /**
     * Connect branches of the given tree to the data store.
     *
     * @param tree The tree to be connected.
     * @param durability The data store durability level.
     * @param storeEntries The store entries to which the branches are connected will be added to this vector.
     * @return True if the branches could be connected successfully.
     */
    bool connectBranches(TTree* tree, DataStore::EDurability durability, StoreEntries* storeEntries);

    /** Connect the parent trees and fill m_parentStoreEntries. */
    bool createParentStoreEntries();

    /** Read data of the current event from the parents. */
    bool readParentTrees();

    /** Loads given entry from persistent tree. */
    void readPersistentEntry(long fileEntry);

    /** Check if we warn the user or abort after an entry was missing after changing files. */
    void entryNotFound(const std::string& entryOrigin, const std::string& name, bool fileChanged = true);

    /** For index files, this creates TEventList/TEntryListArray to enable better cache use. */
    void addEventListForIndexFile(const std::string& parentLfn);

    /** Correct isMC flag for raw data recorded before experiment 8 run 2364. */
    void realDataWorkaround(FileMetaData& metaData);

    //first the steerable variables:
    /** File to read from. Cannot be used together with m_inputFileNames. */
    std::string m_inputFileName;

    /** Files to read from. */
    std::vector<std::string> m_inputFileNames;

    /**
     * The number sequences (e.g. 23:42,101) defining the entries which are processed for each inputFileName.
     */
    std::vector<std::string> m_entrySequences;

    /** Ignore filename override from command line */
    bool m_ignoreCommandLineOverride;

    /**
     * Array for names of branches, that shall be written out.
     *
     * Empty vector results in all branches being read.
     * These vectors can be configured in the steering file.
     */
    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];

    /** Array for names of branches that should NOT be written out.
     *
     *  This takes precedence over m_branchNames, so if a branch is in both
     *  m_branchNames[d] and m_excludeBranchNames[d], it is not saved.
     */
    std::vector<std::string> m_excludeBranchNames[DataStore::c_NDurabilityTypes];


    /** Can be set from steering file to skip the first N events. */
    unsigned int m_skipNEvents;

    /** Level of parent files to be read. */
    int m_parentLevel;

    /** Collect statistics on amount of data read and print statistics (seperate for input & parent files) after processing. */
    bool m_collectStatistics;

    /** experiment, run, event number of first event to load */
    std::vector<int> m_skipToEvent;

    //then those for purely internal use:

    /** Next entry to be read in event tree.  */
    long m_nextEntry;

    /** last entry to be in persistent tree.  */
    long m_lastPersistentEntry;

    /** last parent file LFN seen. (used by addEventListForIndexFile()) */
    std::string m_lastParentFileLFN;


    /**  TTree for event input. */
    TChain* m_tree;

    /**  TTree for persistent input. */
    TChain* m_persistent;

    /** Already connected branches. */
    std::set<std::string> m_connectedBranches[DataStore::c_NDurabilityTypes];

    /** Vector of DataStore entries of event durability that we are supposed to read in. */
    StoreEntries m_storeEntries;
    /** Vector of DataStore entries of persistent durability that we are supposed to read in. */
    StoreEntries m_persistentStoreEntries;

    /** The parent DataStore entries per level */
    std::vector<StoreEntries> m_parentStoreEntries;

    /** Map of file LFNs to trees */
    std::map<std::string, TTree*> m_parentTrees;

    /** for collecting statistics over multiple files. */
    struct ReadStats {
      long calls{0}; /**< number of read calls. */
      long bytesRead{0}; /**< total number of bytes read. */
      long bytesReadExtra{0}; /**< what TFile thinks was the overhead. */
      /** add other stats object. */
      void add(const ReadStats& b)
      {
        calls += b.calls;
        bytesRead += b.bytesRead;
        bytesReadExtra += b.bytesReadExtra;
      }
      /** add current statistics from TFile object. */
      void addFromFile(const TFile* f)
      {
        calls += f->GetReadCalls();
        bytesRead += f->GetBytesRead();
        bytesReadExtra += f->GetBytesReadExtra();
      }
      /** string suitable for printing. */
      std::string getString() const
      {
        std::string s;
        s += "read: " + std::to_string(bytesRead) + " Bytes";
        s += ", overhead: " + std::to_string(bytesReadExtra) + " Bytes";
        s += ", Read() calls: " + std::to_string(calls);
        return s;
      }
    };

    /** some statistics for all files read so far. */
    ReadStats m_readStats;

    /** Input ROOT File Cache size in MB, <0 means default */
    int m_cacheSize{0};

    /** Discard events that have an error flag != 0 */
    bool m_discardErrorEvents{true};
    /** Don't issue a warning when discarding events if the error flag consists exclusively of flags in this mask */
    unsigned int m_discardErrorMask{EventMetaData::c_HLTDiscard};

    /** Set to true if we process the input files completely: No skip events or sequences or -n parameters */
    bool m_processingAllEvents{true};
  };
} // end namespace Belle2
