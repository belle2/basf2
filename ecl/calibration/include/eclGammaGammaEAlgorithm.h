/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Analyze histograms of amplitudes for each ECL crystal from gamma pair  *
 * events. Code can either find most-likely energy deposit for each       *
 * crystal (MC) or calibration constant for each crystal (data)           *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <ecl/calibration/eclGammaGammaEAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using gamma pair events */
    class eclGammaGammaEAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclGammaGammaEAlgorithm();

      /**..Destructor */
      virtual ~eclGammaGammaEAlgorithm() {}

      /**..Parameters to control Novosibirsk fit to energy deposited in each crystal by mu+mu- events */
      std::string outputName; /**< file name for histogram output (eclGammaGammaEAlgorithm.root)*/
      int cellIDLo;  /**<  First cellID to be fit (1) */
      int cellIDHi;  /**<  Last cellID to be fit (8736) */
      int minEntries;  /**<  Minimum entries to fit a crystal (150) */
      int maxIterations; /**< no more than maxIteration iterations (10) */
      double tRatioMin;  /**< Fit range is adjusted so that fit at upper endpoint is between tRatioMin and tRatioMax of peak (0.45) */
      double tRatioMax; /**< Fit range is adjusted so that fit at upper endpoint is between tRatioMin and tRatioMax of peak (0.60) */
      double upperEdgeThresh; /**< Upper edge is where the fit = upperEdgeThresh * peak value (0.02) */
      bool performFits;  /**<  if false, input histograms are copied to output, but no fits are done. (true) */
      bool findExpValues;  /**< if true, fits are used to find expected energy deposit for each crystal instead of the calibration constant (false) */
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
      int atLimit = 4; /**< a parameter is at the limit; upper edge is found from histogram, not fit */
      int poorFit = 3; /**< low chi square; upper edge is found from histogram, not fit */
      int noPeak = 2; /**< Novosibirsk component of fit is negligible; upper edge is found from histogram, not fit */
      int notFit = -1; /**< no fit performed; no constants found for this crystal */

    };
  }
} // namespace Belle2


