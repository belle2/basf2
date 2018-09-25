/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Calculate time offsets from bhabha events.                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Mikhail Remnev                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <ecl/calibration/eclBhabhaTAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>

class TH2F;

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using bhabha events */
    class eclBhabhaTAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclBhabhaTAlgorithm();

      /**..Destructor */
      virtual ~eclBhabhaTAlgorithm() {}

      /*** Parameters ***/

      int cellIDLo; /**< Fit crystals with cellID0 in the inclusive range [cellIDLo,cellIDHi] */
      int cellIDHi; /**< Fit crystals with cellID0 in the inclusive range [cellIDLo,cellIDHi] */
      int maxIterations; /**< Fit is always stopped after maxIterations */

      bool debugOutput; /**< Save every histogram and fitted function to debugFilename */
      /** Name of file with debug output, eclBhabhaTAlgorithm.root by default */
      std::string debugFilename;

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate();
    };
  }
} // namespace Belle2

