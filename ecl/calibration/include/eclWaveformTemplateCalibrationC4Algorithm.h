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

      /** Setter for m_firstCellID */
      void setFirstCellID(int firstCellID) {m_firstCellID = firstCellID;}

      /** Setter for m_lastCellID */
      void setLastCellID(int lastCellID) {m_lastCellID = lastCellID;}

      /** Setter for m_firstCellID */
      void setBatchSize(int batchsize) {m_batchsize = batchsize;}

      /** Setter for m_numBatches */
      void setNumBatches(int numBatches) {m_numBatches = numBatches;}

    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate() override;

    private:

      std::string m_outputName = "eclWaveformTemplateCalibrationC4Algorithm.root"; /**< file name for histogram output */

      int m_firstCellID; /**< First crystal to calibrate */
      int m_lastCellID; /**< Last crystal to calibrate */

      int m_batchsize = 100;  /**< number of crystals in a batch */
      int m_numBatches = 88;  /**< number of batches */

    };
  }
} // namespace Belle2


