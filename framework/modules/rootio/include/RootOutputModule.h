/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROOTOUTPUTMODULE_H
#define ROOTOUTPUTMODULE_H

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/core/Environment.h>

#include <TFile.h>
#include <TTree.h>

#include <string>
#include <vector>



namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   * \addtogroup framework_modules
   * \ingroup modules
   * @{ RootOutputModule @} @}
   */

  /** Write objects from DataStore into a ROOT file.
   *
   *  To ensure that all objects/arrays are saved correctly, follow the directions
   *  in the documentation of the StoreArray, StoreObjPtr and RelationArray classes
   *  regarding the use of registerPersistent().
   *
   *  @sa EDurability
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
    virtual void initialize();

    /** Make sure the file doesn't contain more than one experiment. */
    virtual void beginRun();

    /** Write data in c_Event DataStore maps.
     *
     *  Loops over all objects in event maps (in the first call of the function) and writes them to event-TTree.
     */
    virtual void event();

    /** Write data in the c_Persistent DataStore maps.
     *
     *  Loops over all objects in persistent maps and writes them to persistent-TTree.
     *  Finally the TTree(s) is/are written out.
     */
    virtual void terminate();

    /** Return the used output file, taking into account -o argument to basf2. */
    inline std::string getOutputFile() const {
      const std::string& outputFileArgument = Environment::Instance().getOutputFileOverride();
      if (!outputFileArgument.empty())
        return outputFileArgument;
      return m_outputFileName;
    }

  protected:


  private:

    /** Fill TTree.
     *
     * Write the objects from the DataStore to the output TTree.
     *
     * @param durability Specifies map and tree to be used.
     */
    void fillTree(DataStore::EDurability durability);

    /** Returns true if and only if 'cl' or one of its bases has I/O streamers.
     *
     *  TObject is not considered to have any.
     */
    static bool hasStreamers(TClass* cl);

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

    /** Array for names of branches that should NOT be written out.
     *
     *  This takes precedence over m_branchNames, so if a branch is in both
     *  m_branchNames[d] and m_excludeBranchNames[d], it is not saved.
     */
    std::vector<std::string> m_excludeBranchNames[DataStore::c_NDurabilityTypes];

    /** TFile compression level.
     *
     *  Set the TFile compression level.
     *  Should be between 1 for low compression and 9 for much compression.
     *  <a href="http://root.cern.ch/root/html/src/TFile.cxx.html#b.RlJE">See the ROOT page for more information</a>.
     */
    int m_compressionLevel;

    /** Branch split level.
     *
     *  Set the branch split level.
     */
    int m_splitLevel;


    //then those for purely internal use:

    /** TFile for output. */
    TFile* m_file;

    /** TTree for output. */
    TTree* m_tree[DataStore::c_NDurabilityTypes];

    /** Vector of DataStore entries that are written to the output. */
    std::vector<DataStore::StoreEntry*> m_entries[DataStore::c_NDurabilityTypes];

    /** Vector of parent file IDs. */
    std::vector<int> m_parents;

    /** Experiment number.
     */
    unsigned long m_experiment;

    /** Lowest run number.
     */
    unsigned long m_runLow;

    /** Lowest event number in lowest run.
     */
    unsigned long m_eventLow;

    /** Highest run number.
     */
    unsigned long m_runHigh;

    /** Highest event number in highest run.
     */
    unsigned long m_eventHigh;
  };
} // end namespace Belle2

#endif
