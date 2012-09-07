/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLEINPUTMODULE_H
#define SIMPLEINPUTMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>

#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>


namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   * \addtogroup framework_modules
   * \ingroup modules
   * @{ SimpleInputModule @} @}
   */

  /** Module Simple Input.
   *
   *  This module reads objects from a root file and writes them into the DataStore.
   *  For more information consult the TWiki basf2 Software Portal.
   *  You can specify different TTrees for different durabilities, and give lists, which objects
   *  shall be read from the specific trees.
   *
   *  @sa EDurability
   *  @author <a href="mailto:martin.heck@kit.edu?subject=Input Module">Martin Heck</a>
  */
  class SimpleInputModule : public Module {

  public:

    /** Constructor. */
    SimpleInputModule();

    /** Destructor. */
    virtual ~SimpleInputModule();

    /** Initialize the Module */
    virtual void initialize();

    /** Running over all events */
    virtual void event();

    /** Is called at the end of your Module */
    virtual void terminate();

  protected:


  private:
    /** Sorts stringlist alphabetically and removes any duplicates.
     *
     *  @return true, if duplicates are found
     */
    bool makeBranchNamesUnique(std::vector<std::string> &stringlist) const;

    /** Actually performs the reading from the tree into m_objects. */
    void readTree(DataStore::EDurability durability);


    //first the steerable variables:

    /** File to read from. */
    /** This string is steerable.
    */
    std::string m_inputFileName;

    /** Name array for input tree names. */
    /** Each element of the array is steerable as separate string.
    */
    std::string m_treeNames[DataStore::c_NDurabilityTypes];

    /** Array for names of branches, that shall be written out. */
    /** Empty vector results in all branches being read.
        These vectors can be configured in the steering file.
    */
    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];

    /** Array for names of branches that should NOT be written out.
     *
     *  This takes precedence over m_branchNames, so if a branch is in both
     *  m_branchNames[d] and m_excludeBranchNames[d], it is not saved.
     */
    std::vector<std::string> m_excludeBranchNames[DataStore::c_NDurabilityTypes];

    /** Next entry to be read in event/persistent tree.
     *
     * Can be set from steering file for event durability to skip some events.
     */
    int m_counterNumber[DataStore::c_NDurabilityTypes];


    //then those for purely internal use:

    /** TFile for input. */
    TFile* m_file;

    /**  TTree for input. */
    TTree* m_tree[DataStore::c_NDurabilityTypes];

    /** Vector of DataStore entries that are written to the output. */
    std::vector<DataStore::StoreEntry*> m_entries[DataStore::c_NDurabilityTypes];

    /** Steering parameter names for m_treeNames. */
    const static std::string c_SteerTreeNames[DataStore::c_NDurabilityTypes];

    /** Steering parameter names for m_branchNames. */
    const static std::string c_SteerBranchNames[DataStore::c_NDurabilityTypes];

    /** Steering parameter names for m_excludeBranchNames. */
    const static std::string c_SteerExcludeBranchNames[DataStore::c_NDurabilityTypes];
  };
} // end namespace Belle2

#endif // SIMPLEINPUTMODULE_H
