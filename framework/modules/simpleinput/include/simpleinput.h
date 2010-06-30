/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULESIMPLEINPUT_H
#define MODULESIMPLEINPUT_H

#include <framework/fwcore/Module.h>
#include <framework/fwcore/ModuleManager.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>

#include <TFile.h>
#include <TTree.h>


namespace Belle2 {

  //! Module Simple Input
  /*! This module reads objects from a root file and writes them into the DataStore.
      For more information consult the TWiki basf2 Software Portal.
      You can specify different TTrees for different durabilities, and give lists, which objects
      shall be read from the specific trees.
      \sa EDurability
      \author <a href="mailto:martin.heck@kit.edu?subject=Input Module">Martin Heck</a>
  */
  class ModuleSimpleInput : public Module {

  public:

    //! Macro which adds a method to return a new instance of the module.
    NEW_MODULE(ModuleSimpleInput)

    //! Constructor
    ModuleSimpleInput();

    //! Destructor
    virtual ~ModuleSimpleInput();

    //! Initialize the Module
    /*!
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! Nothing so far.
    */
    virtual void beginRun();

    //! Running over all events
    /*!
    */
    virtual void event();

    //! Is called after processing the last event of a run
    /*! Nothing so far.
    */
    virtual void endRun();

    //! Is called at the end of your Module
    /*!
    */
    virtual void terminate();


  protected:


  private:
    //! gives back Null, if the branch isn't good for read out.
    TBranch* validBranch(int& ibranch, TObjArray* branches);

    //! Some TFile setup.
    void setupTFile();

    //! Function that actually performs the reading from the tree into m_objects.
    void readTree(const EDurability& durability);


    //first the steerable variables:

    //! File to read from.
    /*! This string is steerable.
    */
    std::string m_inputFileName;

    //! Name array for input tree names.
    /*! Each element of the array is steerable as separate string.
    */
    std::string m_treeNames[c_NDurabilityTypes];

    //! Array for names of branches, that shall be written out.
    /*! Empty vector results in all branches being read.
        These vectors can be configured in the steering file.
    */
    std::vector<std::string> m_branchNames[c_NDurabilityTypes];

    //! Event Number.
    /*! Steerable number of events to be skipped before start.
    */
    int m_eventNumber;


    //then those for purely internal use:

    //! Name of
    std::vector<std::string> m_objectNames[c_NDurabilityTypes];

    //! TFile for input.
    TFile* m_file;

    //!  TTree for input.
    TTree* m_tree[c_NDurabilityTypes];

    //! Total number of branches.
    int m_size[c_NDurabilityTypes];

    //! Number of branches from non-array objects.
    int m_sizeObj[c_NDurabilityTypes];

    //! Pointer to pointer, that can be utilised by the TTree.
    TObject** m_objects[c_NDurabilityTypes];

    //! String vector with steering parameter Names for m_treeNames.
    std::vector<std::string>  m_steerTreeNames;

    //! String vector with steering parameter Names for m_branchNames.
    std::vector<std::string>  m_steerBranchNames;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
