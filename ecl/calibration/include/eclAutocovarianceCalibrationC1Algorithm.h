/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/calibration/eclAutocovarianceCalibrationC1Algorithm.h>

/* Basf2 headers. */
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate covariance matrix for crystals using delayed Bhabha events */
    class eclAutocovarianceCalibrationC1Algorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclAutocovarianceCalibrationC1Algorithm();

      /**..Destructor */
      virtual ~eclAutocovarianceCalibrationC1Algorithm() {}

      /** Setter for m_outputName */
      void setOutputName(const std::string& outputName) {m_outputName = outputName;}

      /** Getter for m_outputName */
      std::string getOutputName() {return m_outputName;}

      /** Setter for m_lowestEnergyFraction */
      void setLowestEnergyFraction(float lowestEnergyFraction) {m_lowestEnergyFraction = lowestEnergyFraction;}

      /** Getter for m_lowestEnergyFraction */
      float getLowestEnergyFraction() {return m_lowestEnergyFraction;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      std::string m_outputName = "eclAutocovarianceCalibrationC1Algorithm.root"; /**< file name for histogram output */

      float m_lowestEnergyFraction = 0.75;  /**< Fraction of noise waveforms to use */

    };
  }
} // namespace Belle2


