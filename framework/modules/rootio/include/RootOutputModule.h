/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/core/Environment.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TFile.h>
#include <TTree.h>

#include <string>
#include <vector>

#include <boost/optional.hpp>

namespace Belle2 {
  /** Write objects from DataStore into a ROOT file.
   *
   * You can use the RootInputModule to read the data back in.
   *
   *  @sa DataStore::EDurability
   */
  class RootOutputModule : public Module {

  public:

    /** Constructor.
     */
    RootOutputModule();

    /** Destructor.
     *
     *  Deletion of objects, that were created in the Constructor.
     */
    virtual ~RootOutputModule();

    /** Setting up of various stuff.
     *
     *  Opens a file and creates TTree(s)
     */
    virtual void initialize() override;

    /** Write data in c_Event DataStore maps.
     *
     *  Loops over all objects in event maps (in the first call of the function) and writes them to event-TTree.
     */
    virtual void event() override;

    /** Write data in the c_Persistent DataStore maps.
     *
     *  Loops over all objects in persistent maps and writes them to persistent-TTree.
     *  Finally the TTree(s) is/are written out.
     */
    virtual void terminate() override;

    /** Set the used output file, taking into account -o argument to basf2. */
    virtual std::vector<std::string> getFileNames(bool outputFiles = true) override
    {
      B2ASSERT("RootOutput is not an input module", outputFiles);
      if (!m_ignoreCommandLineOverride) {
        // This will warn if already consumed which is what we want
        std::string outputFileArgument = Environment::Instance().consumeOutputFileOverride(getName());
        if (!outputFileArgument.empty())
          m_outputFileName = outputFileArgument;
        m_ignoreCommandLineOverride = true;
      }
      return {m_outputFileName};
    }

  private:

    /** Finalize the output file */
    void closeFile();

    /** Open the next output file */
    void openFile();

    /** Fill TTree.
     *
     * Write the objects from the DataStore to the output TTree.
     *
     * @param durability Specifies map and tree to be used.
     */
    void fillTree(DataStore::EDurability durability);

    /** Create and fill FileMetaData object. */
    void fillFileMetaData();

    //first the steerable variables:

    /** Name for output file.
     *
     *  This string is steerable.
     *  Best practice: Let it end on .root
     */
    std::string m_outputFileName;

    /** Array for names of branches that should be written out.
     *
     *  Empty vectors result in all branches of the specific durability being written.
     *  These vectors can be configured in the steering file.
     */
    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];

    /** Array of names of branches that should be written out although they are not flagged for writeout.
     *  And usually ignored, use to writeout RestOfEvent.
     */
    std::vector<std::string> m_additionalBranchNames[DataStore::c_NDurabilityTypes];

    /** Array for names of branches that should NOT be written out.
     *
     *  This takes precedence over m_branchNames, so if a branch is in both
     *  m_branchNames[d] and m_excludeBranchNames[d], it is not saved.
     */
    std::vector<std::string> m_excludeBranchNames[DataStore::c_NDurabilityTypes];

    /** TFile compression algorithm.  */
    int m_compressionAlgorithm{0};

    /** TFile compression level.  */
    int m_compressionLevel{1};

    /** Branch split level.
     *
     *  Set the branch split level.
     */
    int m_splitLevel;

    /** Number of entries (if >0) or number of bytes (if <0) after which to flush all baskets to disk */
    int m_autoflush;

    /** Number of entries (if >0) or number of bytes (if <0) after which write the tree metadata to disk */
    int m_autosave;

    /** basket size for each branch in the file in bytes */
    int m_basketsize;

    /** Flag to enable or disable the update of the metadata catalog */
    bool m_updateFileCatalog;

    /** Ignore filename override from command line
     */
    bool m_ignoreCommandLineOverride;

    /** Maximum output file size in MB. If not set we don't split. Otherwise we split
     * if the event tree in output file has reached the given size in MB */
    boost::optional<uint64_t> m_outputSplitSize{boost::none};

    //then those for purely internal use:

    /** Keep track of the file index: if we split files than we add '.f{fileIndex:05d}' in front of the ROOT extension */
    int m_fileIndex{0};

    /** TFile for output. */
    TFile* m_file;

    /** TTree for output. */
    TTree* m_tree[DataStore::c_NDurabilityTypes];

    /** Vector of DataStore entries that are written to the output. */
    std::vector<DataStore::StoreEntry*> m_entries[DataStore::c_NDurabilityTypes];

    /** Vector of parent file LFNs. */
    std::vector<std::string> m_parentLfns;

    /** Map of additional metadata to be added to the output file */
    std::map<std::string, std::string> m_additionalDataDescription;

    /** Lowest experiment number.
     */
    unsigned long m_experimentLow;

    /** Lowest run number.
     */
    unsigned long m_runLow;

    /** Lowest event number in lowest run.
     */
    unsigned long m_eventLow;

    /** Highest experiment number.
     */
    unsigned long m_experimentHigh;

    /** Highest run number.
     */
    unsigned long m_runHigh;

    /** Highest event number in highest run.
     */
    unsigned long m_eventHigh;

    /** Whether or not we want to build an event index */
    bool m_buildIndex{false};

    /** Whether to keep parents same as that of input file */
    bool m_keepParents{false};

    /** Whether this is a regular, local file where we can actually create directories */
    bool m_regularFile{true};

    /** Pointer to the event meta data */
    StoreObjPtr<EventMetaData> m_eventMetaData;
    /** Pointer to the file meta data */
    StoreObjPtr<FileMetaData> m_fileMetaData{"", DataStore::c_Persistent};
    /** File meta data finally stored in the output file */
    FileMetaData m_outputFileMetaData;
  };
} // end namespace Belle2
