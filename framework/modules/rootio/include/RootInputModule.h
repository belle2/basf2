/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Christian Pulvermacher                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROOTINPUTMODULE_H
#define ROOTINPUTMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>

#include <string>
#include <vector>

#include <TChain.h>


namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   * \addtogroup framework_modules
   * \ingroup modules
   * @{ RootInputModule @} @}
   */

  /** Module to read TTree data from file into the data store.
   *
   *  For more information consult the TWiki basf2 Software Portal.
   *  You can specify different TTrees for different durabilities, and specify
   *  if only specific branches should be read (branchNames), or if some should
   *  be excluded (excludeBranchNames).
   *
   *  The module supports reading from multiple files using TChain, entries will
   *  be read in the order the files are specified.
   *
   *  @sa EDurability
  */
  class RootInputModule : public Module {
  public:

    /** Constructor. */
    RootInputModule();

    /** Destructor. */
    virtual ~RootInputModule();

    /** Initialize the Module */
    virtual void initialize();

    /** Running over all events */
    virtual void event();

    /** Is called at the end of your Module */
    virtual void terminate();

  protected:


  private:
    /** Actually performs the reading from the tree into m_objects. */
    void readTree(DataStore::EDurability durability);


    //first the steerable variables:
    /** File to read from. Cannot be used together with m_inputFileNames. */
    std::string m_inputFileName;

    /** Files to read from. */
    std::vector<std::string> m_inputFileNames;

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

    /**  TTree for input. */
    TChain* m_tree[DataStore::c_NDurabilityTypes];

    /** Vector of DataStore entries that we are supposed to read in. */
    std::vector<DataStore::StoreEntry*> m_entries[DataStore::c_NDurabilityTypes];
  };
} // end namespace Belle2

#endif
