/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/calibration/eclNOptimalAlgorithm.h>

/* Basf2 headers. */
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace ECL {

    /** Algorithm that works with eclNOptimalCollector to find the number of
     * crystals to be summed to get the best energy resolution for each test
     * energy and for each group of crystals (8 groups per thetaID in the barrel).
     * Also finds the corresponding energy bias from beam backgrounds, and the
     * peak fraction of energy contained in the crystals.
     *
     * Contact Chris Hearty hearty@physics.ubc.ca for questions or concerns */

    /** Find optimal number of crystals to sum for cluster energy */
    class eclNOptimalAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclNOptimalAlgorithm();

      /**..Destructor */
      virtual ~eclNOptimalAlgorithm() {}


    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

    };
  }
} // namespace Belle2


