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

      /// output tw file name, for text mode
      virtual void outputTWFileName(std::string tw_out_name) {m_outputTWFileName.assign(tw_out_name);}
      /// output t0 file name, for text mode
      virtual void outputT0FileName(std::string t0_out_name) {m_outputT0FileName.assign(t0_out_name);}
      /// Set time walk mode
      virtual void setMode(unsigned short mode)
      {
        m_twParamMode_new = mode;
        if (mode == 0)
          m_nTwParams_new = 1;
        else if (mode == 1)
          m_nTwParams_new = 2;
        else
          B2FATAL("Mode hasn't implemented yet");
      }

    protected:
      /// Run algo on data.
      virtual EResult calibrate();

      /// Create histo for calibrate.
      virtual void createHisto();

      /// save calibration.
      virtual void write();

      /// update constant term to t0 database.
      virtual void updateT0();

      /// fit tw histogram
      virtual void fitToExponentialFunc(TH1D* h1);

      /// prepare calibration.
      virtual void prepare();

    private:

      TH1D* m_h1[300]; /**< Mean of residual as function of ADC of each board */
      TH2D* m_h2[300]; /**< 2D histogram of residual vs ADC for each board */

      double m_xmin = 0.07;                          /**< minimum value cut of drift length. */
      double m_minNdf = 20;                          /**< minimum number of degree of freedom required for track. */
      double m_minPval = 0.;                         /**< minimum number of Prob(chi2) of fitted track. */
      double m_constTerm[300] = {0.};                /**<const term in fitting, it will be added to T0 instead tw*/
      unsigned short m_flag[300];                    /**< flag for fit status */
      std::vector<float> m_tw_old[300];             /**< prior tw list. */
      std::vector<float> m_tw_new[300];               /**< new tw list. */
      bool m_storeHisto = true;                      /**< Store all Histogram or not*/
      std::string m_inputTWFileName = "tw.dat";      /**< Old tw file name. */
      std::string m_inputT0FileName = "t0.dat";      /**< Old t0 file name. */
      std::string m_outputTWFileName = "tw_new.dat"; /**<  tw file name after calibration. */
      std::string m_outputT0FileName = "t0_new.dat"; /**< t0 file name after calibration. */
      double m_tw[300] = {0.};                       /**< Time Walk params, result after fitting*/
      bool m_debug = false;                          /**< run debug or not.*/
      bool  m_textOutput = false;                    /**< output text file if true */
      unsigned short m_twParamMode_old;              /**< =0 for P0/Sqrt(ADC); =1 for P0*Exp(-P1*ADC). */
      unsigned short m_twParamMode_new = 1;          /**< =0 for P0/Sqrt(ADC); =1 for P0*Exp(-P1*ADC). */
      unsigned short m_nTwParams_new = 2;            /**< No. of tw parameters. for new database*/
      unsigned short m_nTwParams_old;                /**< No. of tw parameters. in old database */
    };
  }
}

