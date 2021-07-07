/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <utility>
#include <vector>
#include <string>


namespace Belle2 {

  /** simple logger for CA algorithm*/
  struct CALogger {
    /** counts number of passes executed so far */
    unsigned int nPasses = 0;

    /** collects results for each pass for debugging purposes */
    std::vector< std::pair< int, std::vector<unsigned int> > > passResults;

    /** counts number of fails happened so far */
    unsigned int nFails = 0;

    /** returns string containing number of Cells for each state occured: */
    // small lambda function for printing the results
    std::string getStringCellsOfState(std::vector<unsigned int>& nCellsOfState)
    {
      std::string out = "";
      for (unsigned int i = 0; i < nCellsOfState.size(); i++)
      { out += "had " + std::to_string(nCellsOfState[i]) + " cells of state " + std::to_string(i) + "\n"; }
      return out;
    }
  };

} //Belle2 namespace
