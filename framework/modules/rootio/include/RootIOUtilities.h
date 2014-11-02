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

    /** Names of trees. */
    extern const std::string c_treeNames[];

    /** Steering parameter names for m_branchNames. */
    extern const std::string c_SteerBranchNames[];

    /** Steering parameter names for m_excludeBranchNames. */
    extern const std::string c_SteerExcludeBranchNames[];
  }
}
#endif
