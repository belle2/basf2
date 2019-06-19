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

  /** this has to be defined in ParticleBase.cc again otherwise cant be linked */

  /** extern flag for the type of the mass constraint */
  extern bool massConstraintType;

  /** list of pdg codes to mass constrain */
  extern std::vector<int> massConstraintListPDG;

  /** list of pdg codes to mass constrain */
  extern std::vector<int> fixedToMotherVertexListPDG;

  /** list of pdg codes to mass constrain */
  extern std::vector<int> geoConstraintListPDG;

  /** list of constraints not to apply in tree fit */
  extern std::vector<std::string> removeConstraintList;

}
