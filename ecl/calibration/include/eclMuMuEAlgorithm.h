/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <ecl/calibration/eclMuMuEAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Test class implementing calibration algorithm
   */
  class eclMuMuEAlgorithm : public CalibrationAlgorithm {
  public:

    //..Constructor set the prefix to eclMuMuEAlgorithm
    eclMuMuEAlgorithm();

    //..Destructor
    virtual ~eclMuMuEAlgorithm() {}

    //..Parameters to control Novosibirsk fit to energy deposited in each crystal by mu+mu- events
    int cellIDLo;  /*<  Fit crystals with cellID0 in the inclusive range [cellIDLo,cellIDHi] */
    int cellIDHi;
    int minEntries;  /*<  All crystals to be fit must have at least minEntries events in the fit range */
    int maxIterations; /*<  Adjust upper fit range so that upper endpoint of Novosibirsk fit is */
    double tRatioMin;  /*<  between tRatioMin and tRatioMax of peak, but no more than maxIteration */
    double tRatioMax;  /*<  iterations. */
    bool performFits;  /*<  if false, input histograms are copied to output, but no fits are done. */

  protected:

    //..Run algorithm on data
    virtual EResult calibrate();

  private:

  };
} // namespace Belle2


