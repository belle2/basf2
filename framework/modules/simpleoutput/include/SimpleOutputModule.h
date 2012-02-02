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


#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreMapIter.h>

#include <TFile.h>
#include <TTree.h>
#include <TProcessID.h>


namespace Belle2 {

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
     *  Opens a file, creates TTree(s), fetches Iterators over the maps of the DataStore,
     *  sets m_done to false.
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
    /** Find out, how many objects and arrays, actually are in the datastore.
     *
     *  Used to fill m_size and m_sizeObj.
     *
     *  @return    Number of elements in the respective map.
     *  @par mapID mapID is 2 times durability plus one for the array maps.
     */
    size_t getSize(const int& mapID);

    /** Configuring the TFile.
     */
    void setupTFile();

    /** Fill TTree.
     *
     *  Read the objects from the DataStore and store them in m_objects.
     *  m_objects is connected with the tree, so the ROOT TTree::Fill() function can be called.
     *
     *  @par durability Specifies map and tree to be used.
     */
    void fillTree(const DataStore::EDurability& durability);

    /** Switch branch name meaning function.
     *
     *  This function reads the elements from the respective durability map, eliminates the members
     *  that are in the inital branchNames vector and returns the remaining ones.
     *  This way you can give a list of names that should NOT be written out, instead
     *  of a list of names to be written out.
     *
     *  @param durability Specifies branchNames to be taken.
     */
    void switchBranchNameMeaning(const DataStore::EDurability& durability);


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

    /** Array for names of branches, that shall be written out.
     *
     *  Empty vectors result in all branches of the specific durability being written.
     *  These vectors can be configured in the steering file.
     */
    std::vector<std::string> m_branchNames[DataStore::c_NDurabilityTypes];

    /** Switch branchNames from inclusion to exclusion lists.
     *
     * Switching happens, if the variable is true.
     */
    bool m_switchBranchNameMeaning;

    /** TFile compression level.
     *
     *  Set the TFile compression level.
     *  Should be between 1 for low compression and 9 for much compression.
     *  <a href="http://root.cern.ch/root/html/src/TFile.cxx.html#b.RlJE">See the ROOT page for more information</a>.
     */
    int m_compressionLevel;


    //then those for purely internal use:

    /** TFile for output. */
    TFile* m_file;

    /** TTree for output. */
    TTree* m_tree[DataStore::c_NDurabilityTypes];

    /** Has branch creation already happened?
     *
     *  People will create objects in the event loop.
     *  Therefore the branch creation can not happen before the event function.
     *  However, in the event function, the branches should be created just once.
     */
    bool m_done[DataStore::c_NDurabilityTypes];

    /** Total number of branches. */
    size_t m_size[DataStore::c_NDurabilityTypes];

    /** Number of branches for non-array objects. */
    size_t m_sizeObj[DataStore::c_NDurabilityTypes];

    /** Pointer to pointer, that can be utilised by the TTree.*/
    TObject** m_objects[DataStore::c_NDurabilityTypes];

    /** Iterators over the maps.*/
    StoreIter* m_iter[2 * DataStore::c_NDurabilityTypes];

    /** ROOT Object ID Restore Counter.*/
    int m_nObjID;

    /** String vector with steering parameter Names for m_treeNames. */
    std::vector<std::string>  m_steerTreeNames;

    /** String vector with steering parameter Names for m_branchNames. */
    std::vector<std::string>  m_steerBranchNames;

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
