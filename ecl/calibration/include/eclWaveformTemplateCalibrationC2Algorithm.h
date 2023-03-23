/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/calibration/eclWaveformTemplateCalibrationC2Algorithm.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>

/* Basf2 headers. */
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

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

      /** Setter for m_minEntries */
      void setMinEntries(int minEntries) {m_minEntries = minEntries;}

      /** Getter for m_minEntries */
      int getMinEntries() {return m_minEntries;}

      /** Setter for m_lowestEnergyFraction */
      void setLowestEnergyFraction(int lowestEnergyFraction) {m_lowestEnergyFraction = lowestEnergyFraction;}

      /** Getter for m_lowestEnergyFraction */
      float getLowestEnergyFraction() {return m_lowestEnergyFraction;}

      void setFirstCellID(int firstCellID) {m_firstCellID = firstCellID;}
      void setLastCellID(int lastCellID) {m_lastCellID = lastCellID;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      std::string m_outputName = "eclWaveformTemplateCalibrationC2Algorithm.root"; /**< file name for histogram output */
      int m_minEntries = 1000;  /**<  Minimum entries to fit a crystal */
      float m_lowestEnergyFraction; /**<  m_lowestEnergyFraction */
      int m_firstCellID;
      int m_lastCellID;

      /** Waveform parameters. */
      DBObjPtr<ECLDigitWaveformParameters> m_WaveformParameters;
    };
  }
} // namespace Belle2


