/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLEOUTPUTMODULE_H
#define SIMPLEOUTPUTMODULE_H

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>

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
   * @{ SimpleOutputModule @} @}
   */

  /** Write objects from DataStore into a ROOT file.
   *
   *  Please make sure, that you create the same objects in each call of event,
   *  otherwise the module might not write out everything or has dangling pointers.
   *  For more information consult the TWiki basf2 Software Portal.
   *
   *  @sa EDurability
   *  @author <a href="mailto:martin.heck@kit.edu?subject=Output Module">Martin Heck</a>
   */
  class SimpleOutputModule : public Module {

  public:

    /** Constructor.
     */
    SimpleOutputModule();

    /** Destructor.
     *
     *  Deletion of objects, that were created in the Constructor.
     */
    virtual ~SimpleOutputModule();

    /** Setting up of various stuff.
     *
     *  Opens a file and creates TTree(s)
     */
    virtual void initialize();

    /** Does nothing.
     */
    virtual void beginRun();

    /** Write data in c_Event DataStore maps.
     *
     *  Loops over all objects in event maps (in the first call of the function) and writes them to event-TTree.
     */
    virtual void event();

    /** Write data in c_Run DataStore maps.
     *
     *  Loops over all objects in run maps (in the first call of the function) and writes them to run-TTree.
     */
    virtual void endRun();

    /** Write data in the c_Persistent DataStore maps.
     *
     *  Loops over all objects in persistent maps and writes them to persistent-TTree.
     *  Finally the TTree(s) is/are written out.
     */
    virtual void terminate();


  protected:


  private:
    /** Sorts stringlist alphabetically and removes any duplicates.
     *
     *  @return true, if duplicates are found
     */
    bool makeBranchNamesUnique(std::vector<std::string> &stringlist) const;


    /** Fill TTree.
     *
     * Write the objects from the DataStore to the output TTree.
     *
     * @param durability Specifies map and tree to be used.
     */
    void fillTree(DataStore::EDurability durability);

    //first the steerable variables:

    /** Name for output file.
     *
     *  This string is steerable.
     *  Best practice: Let it end on .root
     */
    std::string m_outputFileName;

    /** Name array for output tree names.
     *
     *  Each element of the array is steerable as separate string.
     */
    std::string m_treeNames[DataStore::c_NDurabilityTypes];

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

    /** Steering parameter names for m_treeNames. */
    const static std::string c_SteerTreeNames[DataStore::c_NDurabilityTypes];

    /** Steering parameter names for m_branchNames. */
    const static std::string c_SteerBranchNames[DataStore::c_NDurabilityTypes];

    /** Steering parameter names for m_excludeBranchNames. */
    const static std::string c_SteerExcludeBranchNames[DataStore::c_NDurabilityTypes];

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

#endif // SIMPLEOUTPUTMODULE_H
