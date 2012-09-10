#ifndef ROOTIOUTILITIES_H
#define ROOTIOUTILITIES_H

#include <string>
#include <vector>

namespace Belle2 {
  /** Some constants and helpers common to the RootInput and RootOutput modules. */
  namespace RootIOUtilities {
    /** Sorts stringlist alphabetically and removes any duplicates.
     *
     *  @return true, if duplicates are found
     */
    bool makeBranchNamesUnique(std::vector<std::string> &stringlist);


    /** Steering parameter names for m_treeNames. */
    extern const std::string c_SteerTreeNames[];

    /** Steering parameter names for m_branchNames. */
    extern const std::string c_SteerBranchNames[];

    /** Steering parameter names for m_excludeBranchNames. */
    extern const std::string c_SteerExcludeBranchNames[];
  }
}
#endif
