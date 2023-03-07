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

    /** Calibrate ecl crystals using gamma pair events */
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

      /** Setter for m_minEntries */
      void setMinEntries(int minEntries) {m_minEntries = minEntries;}

      /** Getter for m_minEntries */
      int getMinEntries() {return m_minEntries;}

      /** Setter for m_lowestEnergyFraction */
      void setLowestEnergyFraction(int lowestEnergyFraction) {m_lowestEnergyFraction = lowestEnergyFraction;}

      /** Getter for m_lowestEnergyFraction */
      float getLowestEnergyFraction() {return m_lowestEnergyFraction;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      std::string m_outputName = "eclAutocovarianceCalibrationC1Algorithm.root"; /**< file name for histogram output */
      int m_minEntries = 1000;  /**<  Minimum entries to fit a crystal */
      float m_lowestEnergyFraction; /**<  m_lowestEnergyFraction */

    };
  }
} // namespace Belle2


