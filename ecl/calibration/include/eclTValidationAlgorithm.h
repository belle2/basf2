/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/**************************************************************************
 * Description:                                                           *
 * Validate the ecl timing calibrations using a hadronic event            *
 * selection or bhabha event selection.                                   *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <string>

namespace Belle2 {
  namespace ECL {

    /** Validate the ecl timing calibrations using a hadronic event selection */
    class eclTValidationAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclTValidationAlgorithm();

      /**..Constructor - main one as it allows user to choose which collector data to analyse*/
      explicit eclTValidationAlgorithm(std::string physicsProcessCollectorName);

      /**..Destructor */
      ~eclTValidationAlgorithm() {}

      /*** Parameters ***/

      int cellIDLo;     /**< Fit crystals with cellID0 in the inclusive range [cellIDLo,cellIDHi] */
      int cellIDHi;     /**< Fit crystals with cellID0 in the inclusive range [cellIDLo,cellIDHi] */
      bool readPrevCrysPayload; /**< Read the previous crystal payload values for comparison */
      double meanCleanRebinFactor;  /**< Rebinning factor for mean calculation */
      double meanCleanCutMinFactor;  /**< After rebinning, create a mask for bins that have values
                                          less than meanCleanCutMinFactor times the maximum bin value.
                                          Expand mask and apply to non-rebinned histogram. */
      double clusterTimesFractionWindow_maxtime;  /**< Maximum time for window to calculate cluster
                                                       time fraction, in ns*/
      bool debugOutput; /**< Save every histogram and fitted function to debugFilename */
      /** Name of file with debug output, eclTValidationAlgorithm.root by default */
      std::string debugFilenameBase;

    protected:

      /**..Run algorithm on events */
      EResult calibrate() override;

    };
  }  // namespace ECL
} // namespace Belle2

