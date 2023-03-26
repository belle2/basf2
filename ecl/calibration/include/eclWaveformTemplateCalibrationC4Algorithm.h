/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/calibration/eclWaveformTemplateCalibrationC4Algorithm.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>

/* Basf2 headers. */
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using gamma pair events */
    class eclWaveformTemplateCalibrationC4Algorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclWaveformTemplateCalibrationC4Algorithm();

      /**..Destructor */
      virtual ~eclWaveformTemplateCalibrationC4Algorithm() {}

      /** Setter for m_outputName */
      void setOutputName(const std::string& outputName) {m_outputName = outputName;}

      /** Getter for m_outputName */
      std::string getOutputName() {return m_outputName;}

      void setFirstCellID(int firstCellID) {m_firstCellID = firstCellID;}

      void setLastCellID(int lastCellID) {m_lastCellID = lastCellID;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      std::string m_outputName = "eclWaveformTemplateCalibrationC4Algorithm.root"; /**< file name for histogram output */

      int m_firstCellID;
      int m_lastCellID;

      //DBObjPtr<ECLDigitWaveformParameters> m_existingPhotonWaveformParameters;
      //DBObjPtr<ECLDigitWaveformParameters> m_existingHadronDiodeWaveformParameters;
      /** Waveform parameters. */
      //DBObjPtr<ECLDigitWaveformParameters> m_PhotonHadronDiodeWaveformParameters;
    };
  }
} // namespace Belle2


