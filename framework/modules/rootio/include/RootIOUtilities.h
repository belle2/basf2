/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROOTIOUTILITIES_H
#define ROOTIOUTILITIES_H

#include <string>
#include <vector>

class TTree;
class TClass;

namespace Belle2 {
  /** Some constants and helpers common to the RootInput and RootOutput modules. */
  namespace RootIOUtilities {
    /** Sorts stringlist alphabetically and removes any duplicates.
     *
     *  @return true, if duplicates are found
     */
    bool makeBranchNamesUnique(std::vector<std::string>& stringlist);

    /** return entry number with given (event, run, experiment) from tree. Returns -1 on error. */
    long getEntryNumberWithEvtRunExp(TTree* tree, long event, long run, long experiment);

    /** Build TTreeIndex on tree (assumes EventMetaData branch exists there). */
    void buildIndex(TTree* tree);

    /** Returns true if and only if 'cl' or one of its bases has I/O streamers.
     *
     *  TObject is not considered to have any.
     */
    bool hasStreamer(TClass* cl);

    /** Returns true if and only if 'cl' has a user-defined streamer
     *
     * In that case, TClonesArrays of this type should be written with BypassStreamer(false) and
     * split-level -1 (no splitting).
     */
    bool hasCustomStreamer(TClass* cl);

    /** Load dictionaries for classes found in input files. */
    void loadDictionaries();


    /** Names of trees. */
    extern const std::string c_treeNames[];

    /** Steering parameter names for m_branchNames. */
    extern const std::string c_SteerBranchNames[];

    /** Steering parameter names for m_excludeBranchNames. */
    extern const std::string c_SteerExcludeBranchNames[];
  }
}
#endif
