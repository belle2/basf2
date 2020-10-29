/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Calculate time offsets from bhabha events.                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ewan Hill        (ehill@mail.ubc.ca)                     *
 *               Mikhail Remnev                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <ecl/calibration/eclBhabhaTAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>
#include <string>


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

      int cellIDLo;     /**< Fit crystals with cellID0 in the inclusive range [cellIDLo,cellIDHi] */
      int cellIDHi;     /**< Fit crystals with cellID0 in the inclusive range [cellIDLo,cellIDHi] */
      double meanCleanRebinFactor;  /**< Rebinning factor for mean calculation */
      double meanCleanCutMinFactor;  /**< After rebinning, create a mask for bins that have values
                                          less than meanCleanCutMinFactor times the maximum bin value.
                                          Expand mask and apply to non-rebinned histogram. */
      int crateIDLo;    /**< Fit crates with crateID0 in the inclusive range [crateIDLo,crateIDHi] */
      int crateIDHi;    /**< Fit crates with crateID0 in the inclusive range [crateIDLo,crateIDHi] */
      bool debugOutput; /**< Save every histogram and fitted function to debugFilename */
      /** Name of file with debug output, eclBhabhaTAlgorithm.root by default */
      std::string debugFilenameBase;
      std::string collectorName;  /**< Name of the collector */

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

//    private:
//      /** Number of processed runs */
//      unsigned int m_runCount;
    };
  }  // namespace ECL
} // namespace Belle2

