/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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

      /** Setter for RegularizationParams*/
      void setRegularizationParams(double u0, double u1, double u2)
      {
        m_u0 = u0;
        m_u1 = u1;
        m_u2 = u2;
      }

      /** Getter for m_outputName */
      std::string getOutputName() {return m_outputName;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      const int m_numberofADCPoints = 31; /**< length of ECLDsp waveform */

      std::string m_outputName = "eclAutocovarianceCalibrationC3Algorithm.root"; /**< file name for histogram output */

      int m_TotalCountsThreshold = 10000; /**< min number of counts needed to compute calibration */

      double m_u0 = 13.0;   /**< Regularization Function Parameter 0 */
      double m_u1 = 4.0;   /**< Regularization Function Parameter 1 */
      double m_u2 = 1.0;   /**< Regularization Function Parameter 2 */

    };
  }
} // namespace Belle2



