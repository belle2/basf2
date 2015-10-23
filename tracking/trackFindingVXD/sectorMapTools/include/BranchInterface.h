/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

