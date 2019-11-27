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
#include <ecl/utility/ECLChannelMapper.h>
#include <string>

class TH2F;


using namespace std ;

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using bhabha events */
    class eclBhabhaTAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclBhabhaTAlgorithm();

      /**..Constructor */
      eclBhabhaTAlgorithm(string colName);

      /**..Constructor */
      eclBhabhaTAlgorithm(bool is_ee_gamma);

      /**..Destructor */
      virtual ~eclBhabhaTAlgorithm() {}

      /*** Parameters ***/

      int cellIDLo; /**< Fit crystals with cellID0 in the inclusive range [cellIDLo,cellIDHi] */
      int cellIDHi; /**< Fit crystals with cellID0 in the inclusive range [cellIDLo,cellIDHi] */
      int crateIDLo; /**< Fit crates with crateID0 in the inclusive range [crateIDLo,crateIDHi] */
      int crateIDHi; /**< Fit crystals with crateID0 in the inclusive range [crateIDLo,crateIDHi] */

      double meanCleanRebinFactor ;  /**< Rebinning factor for mean calculation */
      double meanCleanCutMinFactor
      ;  /**< After rebinning, create a mask for bins that have values less than meanCleanCutMinFactor times the maximum bin value.  Expand mask and apply to non-rebinned histogram. */

      bool debugOutput; /**< Save every histogram and fitted function to debugFilename */
      /** Name of file with debug output, eclBhabhaTAlgorithm.root by default */
      string debugFilenameBase ;

      string collectorName ;


      //string runIndex ;

      ECLChannelMapper* crystalMapper;

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate();

    private:
      /** Number of processed runs */
      unsigned int m_run_count;
    };
  }  // namespace ECL
} // namespace Belle2

