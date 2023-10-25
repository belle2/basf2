/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/calibration/eclWaveformTemplateCalibrationC3Algorithm.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>

/* Basf2 headers. */
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using gamma pair events */
    class eclWaveformTemplateCalibrationC3Algorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclWaveformTemplateCalibrationC3Algorithm();

      /**..Destructor */
      virtual ~eclWaveformTemplateCalibrationC3Algorithm() {}

      /** Setter for m_outputName */
      void setOutputName(const std::string& outputName) {m_outputName = outputName;}

      /** Getter for m_outputName */
      std::string getOutputName() {return m_outputName;}

      /** Setter for m_firstCellID */
      void setFirstCellID(int firstCellID) {m_firstCellID = firstCellID;}

      /** Setter for m_lastCellID */
      void setLastCellID(int lastCellID) {m_lastCellID = lastCellID;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      std::string m_outputName = "eclWaveformTemplateCalibrationC3Algorithm.root"; /**< file name for histogram output */
      int m_firstCellID;  /**< first cell id to fit */
      int m_lastCellID;  /**< last cell id to fit */

      DBObjPtr<ECLDigitWaveformParameters> m_WaveformParameters;  /**< Waveform parameters. */
    };
  }
} // namespace Belle2


