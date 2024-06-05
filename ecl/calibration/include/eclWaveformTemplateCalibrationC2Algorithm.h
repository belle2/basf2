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

    /** Calibrate ecl crystals using gamma pair events */
    class eclWaveformTemplateCalibrationC2Algorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclWaveformTemplateCalibrationC2Algorithm();

      /**..Destructor */
      virtual ~eclWaveformTemplateCalibrationC2Algorithm() {}

      /** Setter for m_outputName */
      void setOutputName(const std::string& outputName) {m_outputName = outputName;}

      /** Getter for m_outputName */
      std::string getOutputName() {return m_outputName;}

      /** Setter for m_firstCellID */
      void setFirstCellID(int firstCellID) {m_firstCellID = firstCellID;}

      /** Getter for m_firstCellID */
      void setLastCellID(int lastCellID) {m_lastCellID = lastCellID;}

      /** Setter for m_AttemptLimit */
      void setAttemptLimit(int AttemptLimit) {m_AttemptLimit = AttemptLimit;}

      /** Setter for m_ParamLimitFactor */
      void setParamLimitFactor(int ParamLimitFactor) {m_ParamLimitFactor = ParamLimitFactor;}

      /** Setter for m_CollectorLimit */
      void setCollectorLimit(int CollectorLimit) {m_CollectorLimit = CollectorLimit;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      std::string m_outputName = "eclWaveformTemplateCalibrationC2Algorithm.root"; /**< file name for histogram output */

      int m_firstCellID; /**< First crystal to calibrate */
      int m_lastCellID; /**< Last crystal to calibrate */
      int m_CollectorLimit = 6; /**< max number of waveforms to simultaneously fit */
      double m_ParamLimitFactor = 0.25; /**< Factor to determine parameter limits in fit */
      int m_AttemptLimit = 10;  /**< Number of attempts before increasing parameter limits or resLimt */
      const int m_NumberofADCPoints = 31;  /**< Number of adc points in waveform */
      const int m_SimutaniousFitLimit = 3;  /**< Min number waveforms required for simultaneous fit */
      const int m_TotalCountsThreshold = 3;  /**< Min number waveforms required per crystal */
      double m_ParLimitFactorIterator = 0.5; /**< Amount to increase m_ParamLimitFactor */
      double m_ParLimitFactorLimit = 2.1; /**< Max limit to increase m_ParamLimitFactor */
      double m_ResLimitIterator = 1.5; /**< Factor to increase resLimit*/
      const double m_BaseParamLimitFactor = 0.25; /**< Reset value of m_ParamLimitFactor */

    };
  }
} // namespace Belle2


