/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Analyze histograms of amplitudes for each ECL crystal from moun pair   *
 * events. Code can either find most-likely energy deposit for each       *
 * crystal (MC) or calibration constant for each crystal (data)           *
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
  namespace ECL {

    /** Calibrate ecl crystals using muon pair events */
    class eclMuMuEAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclMuMuEAlgorithm();

      /**..Destructor */
      virtual ~eclMuMuEAlgorithm() {}

      /**..Parameters to control Novosibirsk fit to energy deposited in each crystal by mu+mu- events */
      int cellIDLo;  /**<  First cellID to be fit */
      int cellIDHi;  /**<  Last cellID to be fit */
      int minEntries;  /**<  All crystals to be fit must have at least minEntries events in the fit range */
      int maxIterations; /**< no more than maxIteration iterations */
      double tRatioMin;  /**< Fit range is adjusted so that fit at upper endpoint is between tRatioMin and tRatioMax of peak */
      double tRatioMax; /**< Fit range is adjusted so that fit at upper endpoint is between tRatioMin and tRatioMax of peak */
      bool performFits;  /**<  if false, input histograms are copied to output, but no fits are done. */
      bool findExpValues;  /**< if true, fits are used to find expected energy deposit for each crystal instead of the calibration constant */
      int storeConst; /**< controls which values are written to the database.
                             0 (default): store value found by successful fits, or -|input value| otherwise;
                             -1 : do not store values
                             1 : store values if every fit for [cellIDLo,cellIDHi] was successful */

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate();

    private:
      int fitOK = 16; /**< fit is OK */
      int iterations = 8; /**< fit reached max number of iterations, but is useable */
      int atLimit = 4; /**< a parameter is at the limit; fit not useable */
      int poorFit = 3; /**< low chi square; fit not useable */
      int noPeak = 2; /**< Novosibirsk component of fit is negligible; fit not useable */
      int notFit = -1; /**< no fit performed */

    };
  }
} // namespace Belle2


