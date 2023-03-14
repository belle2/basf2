/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <map>

namespace Belle2 {

  /**
   * Helper class for converting strings into a ROOT-friendly format
   * (e.g.: removing special characters in branch names, etc.)
   */
  class MakeROOTCompatible {

  public:

    /**
     * Remove special characters that ROOT dislikes in branch names, e.g. (, ), :, etc
     */
    static std::string makeROOTCompatible(std::string str);

    /**
     * Invert makeROOTCompatible operation
     */
    static std::string invertMakeROOTCompatible(std::string str);

    /**
     * Substituation map for makeROOTCompatible
     */
    static std::map<std::string, std::string> getSubstitutionMap();

  };

}
