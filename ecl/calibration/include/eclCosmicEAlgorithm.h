/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <ecl/calibration/eclCosmicEAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace ECL {

    /** class eclCosmiEAlgorithm. Analyze histograms of normalized energy for each ECL crystal from
     * cosmic ray events. Code can either find most-likely energy deposit for each crystal using CRY MC
     * or calibration constant for each crystal (data)      */
    class eclCosmicEAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor. */
      eclCosmicEAlgorithm();

      /** Destructor. */
      virtual ~eclCosmicEAlgorithm() {}

      /** Parameters to control Novosibirsk fit to signal measured in each crystal */
      int cellIDLo;  /**<  First cellID to be fit */
      int cellIDHi;  /**<  Last cellID to be fit */
      int minEntries;  /**<  All crystals to be fit must have at least minEntries events in the fit range */
      int maxIterations; /**< no more than maxIteration iterations */
      double tRatioMin; /**< Fit range is adjusted so that fit at upper endpoint is between tRatioMin and tRatioMax of peak */
      double tRatioMax; /**< Fit range is adjusted so that fit at upper endpoint is between tRatioMin and tRatioMax of peak */
      bool performFits;  /**<  if false, input histograms are copied to output, but no fits are done. */
      bool findExpValues;  /**< if true, fits are used to find expected energy deposit for each crystal instead of the calibration constant */
      int storeConst; /**< controls which values are written to the database.
                             0 (default): store value found by successful fits, or -|input value| otherwise;
                             -1 : do not store values
                             1 : store values if every fit for [cellIDLo,cellIDHi] was successful */

    protected:

      /** Run algorithm on events */
      virtual EResult calibrate() override;

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


