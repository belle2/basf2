/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

namespace Belle2 {


  /** validation tool for CA algorithm */
  template<class CellType>
  struct CAValidator {
    /** checks if CA is still in valid area */
    bool isValidRound(int caRound) { return !(caRound > nMaxIterations) and !(caRound < 0); }

    /** function to check the cell if seed-conditions are met. */
    bool checkSeed(const CellType& aCell) { return !(aCell.getState() < seedThreshold); }

    /** determines max number of iterations allowed by user, standard value is (number of VXD layers x 4) + 1. */
    int nMaxIterations = 25;

    /** threshold for being seed, standard value is 1 */
    unsigned int seedThreshold = 1;
  };

} //Belle2 namespace
