/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <cdc/dbobjects/CDCGeometry.h>
#include <framework/database/DBObjPtr.h>
#include "string"
namespace Belle2 {
  namespace CDC {
    /**
     * Class for CDC fudge factor calibration .
     */
    class FudgeFactorCalibrationAlgorithm: public CalibrationAlgorithm {
    public:
      /// Constructor.
      FudgeFactorCalibrationAlgorithm();
      /// Destructor
      ~FudgeFactorCalibrationAlgorithm() {}
      /// store Hisotgram or not.
      //      void storeHisto(bool storeHist = false) {m_storeHisto = storeHist;}
      /// Set name for histogram output
      void setHistFileName(const std::string& name) {m_histName = name;}
    protected:
      /// Run algo on data
      EResult calibrate() override;
    private:
      std::string m_histName = "histo_mumu.root"; /**< root file name */
    };
  }// name space CDC
} // namespace Belle2

