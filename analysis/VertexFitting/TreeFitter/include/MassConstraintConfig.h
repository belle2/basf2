/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * right(C) 2018 - Belle II Collaboration                                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Jo-Frederik Krohn                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>

namespace TreeFitter {

  /** extern flag for the type of the cosntraint */
  extern bool massConstraintType;

  /** list of pdg codes to mass cosntraint */
  extern std::vector<int> massConstraintListPDG;

}
