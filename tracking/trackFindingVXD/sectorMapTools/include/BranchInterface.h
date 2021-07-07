/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <TBranch.h>


namespace Belle2 {

  /** simple struct for interfacing the Branch.  */
  template <class ValueType> struct BranchInterface {
    std::string name; /**< carries name of branch. */
    TBranch* branch; /**< carries pointer to branch. */
    ValueType value; /**< carries value of current branchEntry. */

    /** constructor */
    BranchInterface() : name(""), branch(nullptr), value(ValueType(0)) {}

    /** update entry */
    void update(Long64_t entry) { branch->GetEntry(entry); }
  };
}

