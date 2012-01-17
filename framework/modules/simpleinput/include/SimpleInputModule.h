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

    /*! Initialize the Module */
    /*!
    */
    virtual void initialize();

    /*! Called when entering a new run */
    /*! Nothing so far.
    */
    virtual void beginRun();

    /*! Running over all events */
    /*!
    */
    virtual void event();

    /*! Is called after processing the last event of a run */
    /*! Nothing so far.
    */
    virtual void endRun();

    /*! Is called at the end of your Module */
    /*!
    */
    virtual void terminate();


  protected:


  private:
    /*! Returns a pointer to the i'th branch of specified durability if valid and not disabled via branchNames, NULL otherwise */
    TBranch* validBranch(int ibranch, DataStore::EDurability durability) const;

    /*! Function that actually performs the reading from the tree into m_objects. */
    void readTree(const DataStore::EDurability& durability);


    //first the steerable variables:

    /*! File to read from. */
    /*! This string is steerable.
    */
    std::string m_inputFileName;

    /*! Name array for input tree names. */
    /*! Each element of the array is steerable as separate string.
    */
    std::string m_treeNames[DataStore::c_NDurabilityTypes];

    /*! Array for names of branches, that shall be written out. */
    /*! Empty vector results in all branches being read.
        These vectors can be configured in the steering file.
    */
    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];

    /*! Event Number. */
    /*! Steerable number of events to be skipped before start.
    */
    int m_counterNumber[DataStore::c_NDurabilityTypes];


    //then those for purely internal use:

    /*! Name of */
    std::vector<std::string> m_objectNames[DataStore::c_NDurabilityTypes];

    /*! TFile for input. */
    TFile* m_file;

    /*!  TTree for input. */
    TTree* m_tree[DataStore::c_NDurabilityTypes];

    /*! Total number of branches. */
    int m_size[DataStore::c_NDurabilityTypes];

    /*! Number of branches from non-array objects. */
    int m_sizeObj[DataStore::c_NDurabilityTypes];

    /*! Pointer to pointer, that can be utilised by the TTree. */
    TObject** m_objects[DataStore::c_NDurabilityTypes];

    /*! String vector with steering parameter Names for m_treeNames. */
    std::vector<std::string>  m_steerTreeNames;

    /*! String vector with steering parameter Names for m_branchNames. */
    std::vector<std::string>  m_steerBranchNames;

  };

} // end namespace Belle2

#endif // SIMPLEINPUTMODULE_H
