/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Makoto Uchida                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <TH1D.h>
#include <TH2D.h>
#include "string"

namespace Belle2 {
  namespace CDC {
    /**
     * Class for Time walk calibration.
     * Time walk is calibrated for each board as a function of ADC
     */
    class TimeWalkCalibrationAlgorithm  :  public CalibrationAlgorithm {
    public:
      /// Constructor
      TimeWalkCalibrationAlgorithm();

      /// Destructor
      virtual ~TimeWalkCalibrationAlgorithm() {}

      /// change flag for debug
      virtual void setDebug(bool debug) {m_debug = debug; }

      /// Store Histogram or not
      virtual void setStoreHisto(bool storeHist) {m_storeHisto = storeHist;}

      /// minimum number of degree freedom requirement for track
      virtual void setMinimumNDF(double ndf) {m_minNdf = ndf;}

      /// minimum chi2 prob requirement for track
      virtual void setMinimumPval(double pval) {m_minPval = pval;}

      /// Enable text output of calibration result
      void enableTextOutput(bool output = true) {m_textOutput = output;}

      /// output file name
      void setOutputFileName(std::string outputname) {m_outputFileName.assign(outputname);}


    protected:
      /// Run algo on data.
      virtual EResult calibrate();
      /// Create histo for calibrate.
      virtual void createHisto();
      /// save calibration.
      virtual void write();

      /// prepare calibration.
      virtual void prepare(StoreObjPtr<EventMetaData>& evtPtr);
      /// Apply slice fit.
      void doSliceFitY(int boardId, int minHitCut);

    private:
      TH1D* m_h1[300] = {nullptr}; /**<Mean of residual as function of ADC of each board*/
      TH2D* m_h2[300] = {nullptr}; /**<2D histogram of residual vs ADC for each board*/

      double m_xmin = 0.07; /**< minimum value cut of drift length. */
      double m_minNdf = 5;  /**< minimum number of degree of freedom required for track. */
      double m_minPval = 0.; /**< minimum number of Prob(chi2) of fitted track. */
      double m_tw[300] = {0.}; /**< Time Walk params, result after fitting*/
      double m_twPost[300] = {0.}; /**< Time Walk params before calibration */
      bool m_debug = false; /**< run debug or not.*/
      bool m_storeHisto = true;/**< Store all Histogram or not*/
      bool  m_textOutput = false; /**< output text file if true */
      std::string m_outputFileName = "tw_new.dat";  /**< Output tw file name for time walk. */
    };
  }
}

