/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULESIMPLEOUTPUT_H
#define MODULESIMPLEOUTPUT_H

#include <framework/fwcore/Module.h>
#include <framework/fwcore/ModuleManager.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreMapIter.h>

#include <TFile.h>
#include <TTree.h>
#include <TProcessID.h>


namespace Belle2 {

  //! Module Simple Output
  /*! This module writes the objects from the DataStore into a root file.
      Please make sure, that you create the same objects in each call of event,
      otherwise the module might not write out everything or has dangling pointers.
      For more information consult the TWiki basf2 Software Portal.
      \sa EDurability
      \author <a href="mailto:martin.heck@kit.edu?subject=Output Module">Martin Heck</a>
  */
  class ModuleSimpleOutput : public Module {

  public:

    //! Macro which adds a method to return a new instance of the module.
    NEW_MODULE(ModuleSimpleOutput)

    //! Constructor
    ModuleSimpleOutput();

    //! Destructor
    virtual ~ModuleSimpleOutput();

    //! Initialize the Module
    /*! Opens a file, creates TTree(s), fetches Iterators over the maps of the DataStore,
        sets m_done to false.
    */
    virtual void initialize();

    /*! Does nothing.
    */
    virtual void beginRun();

    //! Running over all events
    /*! Loops over all objects in event maps and writes to event-TTree.
    */
    virtual void event();

    //! Is called after processing the last event of a run
    /*! Loops over all objects in run maps and writes to run-TTree.
    */
    virtual void endRun();

    //! Is called at the end of your Module
    /*! Loops over all objects in persistent maps and writes to persistent-TTree.
        Writes TTree(s) out.
    */
    virtual void terminate();


  protected:


  private:
    //! Find out, how many objects and arrays, actually are in the datastore.
    /*! Used to fill m_size and m_sizeObj.
        \return Number of elements in the respective map.
        \par mapID mapID is 2 times durability plus one for the array maps.
    */
    size_t getSize(const int& mapID);

    //! Configuring the TFile.
    void setupTFile();

    //! Fill TTree.
    /*! Read the objects from the DataStore and store them in m_objects.
        m_objects is connected with the tree, so the ROOT TTree::Fill() function can be called.
        \par durability Specifies map and tree to be used.
    */
    void fillTree(const EDurability& durability);

    //! Switch branch name meaning function.
    /*! This function reads the elements from the respective durability map, eliminates the members
        that are in the inital branchNames vector and returns the remaining ones.
        \param durability Specifies branchNames to be taken.
    */
    void switchBranchNameMeaning(const EDurability& durability);


    //first the steerable variables:

    //! Name for output file.
    /*! This string is steerable.
        Best practice: Let it end on .root
    */
    std::string m_outputFileName;

    //! Name array for output tree names.
    /*! Each element of the array is steerable as separate string.
    */
    std::string m_treeNames[c_NDurabilityTypes];

    //! Array for names of branches, that shall be written out.
    /*! Empty vectors result in all branches of the specific durability being written.
        These vectors can be configured in the steering file.
    */
    std::vector<std::string> m_branchNames[c_NDurabilityTypes];

    //! Switch branchNames from exclusion to inclusion list.
    /*!
    */
    bool m_switchBranchNameMeaning;

    //! TFile compression level.
    /*! Set the TFile compression level.
        Should be between 1 for low compression and 9 for much compression.
        <a href="http://root.cern.ch/root/html/src/TFile.cxx.html#b.RlJE">See the ROOT page for more information</a>.
    */
    int m_compressionLevel;


    //then those for purely internal use:

    //! TFile for output.
    TFile* m_file;

    //! TTree for output.
    TTree* m_tree[c_NDurabilityTypes];

    //! Has branch creation already happened?
    /*! People will create objects in the event loop.
        Therefore the branch creation can not happen before the event function.
        However, in the event function, the branches should be created just once.
    */
    bool m_done[c_NDurabilityTypes];

    //! Total number of branches.
    size_t m_size[c_NDurabilityTypes];

    //! Number of branches for non-array objects.
    size_t m_sizeObj[c_NDurabilityTypes];

    //! Pointer to pointer, that can be utilised by the TTree.
    TObject** m_objects[c_NDurabilityTypes];

    //! Iterators over the maps.
    StoreIter* m_iter[2*c_NDurabilityTypes];

    //! ROOT Object ID Restore Counter
    int m_nObjID;

    //! String vector with steering parameter Names for m_treeNames.
    std::vector<std::string>  m_steerTreeNames;

    //! String vector with steering parameter Names for m_branchNames.
    std::vector<std::string>  m_steerBranchNames;
  };

} // end namespace Belle2

#endif // MODULEHELLO_H
