/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/calibration/eclWaveformTemplateCalibrationC1Algorithm.h>

/* Basf2 headers. */
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using gamma pair events */
    class eclWaveformTemplateCalibrationC1Algorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclWaveformTemplateCalibrationC1Algorithm();

      /**..Destructor */
      virtual ~eclWaveformTemplateCalibrationC1Algorithm() {}

      /** Setter for m_outputName */
      void setOutputName(const std::string& outputName) {m_outputName = outputName;}

      /** Getter for m_outputName */
      std::string getOutputName() {return m_outputName;}

      /** Setter for m_fractionLimitGeneral*/
      void setfractionLimitGeneral(float fractionLimitGeneral) {m_fractionLimitGeneral = fractionLimitGeneral;}

      /** Getter for m_fractionLimitGeneral*/
      float fractionLimitGeneral() {return m_fractionLimitGeneral;}

      /** Setter for m_fractionLimitLowCounts*/
      void setfractionLimitLowCounts(float fractionLimitLowCounts) {m_fractionLimitLowCounts = fractionLimitLowCounts;}

      /** Getter for m_fractionLimitLowCounts*/
      float fractionLimitLowCounts() {return m_fractionLimitLowCounts;}

      /** Setter for m_minWaveformLimit*/
      void setminWaveformLimit(int minWaveformLimit) {m_minWaveformLimit = minWaveformLimit;}

      /** Getter for m_minWaveformLimit*/
      int minWaveformLimit() {return m_minWaveformLimit;}

      /** Setter for m_LowCountThreshold*/
      void setLowCountThreshold(int LowCountThreshold) {m_LowCountThreshold = LowCountThreshold;}

      /** Getter for m_LowCountThreshold*/
      int LowCountThreshold() {return m_LowCountThreshold;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      std::string m_outputName = "eclWaveformTemplateCalibrationC1Algorithm.root"; /**< file name for histogram output */
      float m_fractionLimitGeneral = 0.5; /**<  fraction of waveforms used */
      float m_fractionLimitLowCounts = 0.9; /**<  fraction of waveforms used for low count crystals*/
      int m_LowCountThreshold = 20; /**<  min number of waveforms required per crystal */
      int m_minWaveformLimit = 10; /**<  min number of waveforms required per crystal to be considered low counts */

    };
  }
} // namespace Belle2


