/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <map>

namespace Belle2 {

  /**
   * Remove special characters that ROOT dislikes in branch names, e.g. (, ), :, etc
   */
  std::string makeROOTCompatible(std::string str);

  /**
   * Invert makeROOTCompatible operation
   */
  std::string invertMakeROOTCompatible(std::string str);

  /**
   * Substituation map for makeROOTCompatible
   */
  std::map<std::string, std::string> getSubstitutionMap();

}
