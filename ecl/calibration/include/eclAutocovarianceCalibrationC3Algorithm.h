/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/calibration/eclAutocovarianceCalibrationC3Algorithm.h>

/* Basf2 headers. */
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate covariance matrix for crystals using delayed Bhabha events */
    class eclAutocovarianceCalibrationC3Algorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclAutocovarianceCalibrationC3Algorithm();

      /**..Destructor */
      virtual ~eclAutocovarianceCalibrationC3Algorithm() {}

      /** Setter for m_outputName */
      void setOutputName(const std::string& outputName) {m_outputName = outputName;}

      /** Getter for m_outputName */
      std::string getOutputName() {return m_outputName;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      const int m_numberofADCPoints = 31; /**< length of ECLDsp waveform */
      std::string m_outputName = "eclAutocovarianceCalibrationC3Algorithm.root"; /**< file name for histogram output */

    };
  }
} // namespace Belle2


