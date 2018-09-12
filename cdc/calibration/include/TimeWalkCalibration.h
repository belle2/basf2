/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  CDC Group                                               *
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
     * Time walk is calibrated for each board as function of ADC
     */
    class TimeWalkCalibration {
    public:
      /// Constructor
      TimeWalkCalibration();
      /// Destructor
      virtual ~TimeWalkCalibration() {}
      /// setting for use database or text mode
      virtual void setUseDB(bool useDB) {m_useDB = useDB; }
      /// Store Histogram or not
      virtual void setStoreHisto(bool storeHist) {m_storeHisto = storeHist;}
      /// minimum number of degree freedom requirement for track
      virtual void setMinimumNDF(double minndf) {m_ndfmin = minndf;}
      /// minimum chi2 prob requirement for track
      virtual void setMinimumPval(double minPval) {m_Pvalmin = minPval;}
      /// input root file names, output of collector
      virtual void InputFileNames(std::string inputname) {m_InputRootFileName.assign(inputname);}
      /// Input tw file name, incase use text mode
      virtual void InputTWFileName(std::string tw_name) {m_InputTWFileName.assign(tw_name);}
      /// output tw file name, for text mode
      virtual void OutputTWFileName(std::string tw_out_name) {m_outputTWFileName.assign(tw_out_name);}
      /// output t0 file name, for text mode
      virtual void OutputT0FileName(std::string t0_out_name) {m_outputT0FileName.assign(t0_out_name);}
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
      /// function to run algorithm
      virtual void execute()
      {
        calibrate();
      }

    protected:
      /// Run algorithm
      virtual bool calibrate();
      /// Create histo for calibrate
      virtual void CreateHisto();
      /// save calibration
      virtual void Write();
      /// update constant term to t0 database.
      virtual void updateT0();
      /// read tw from database
      virtual void readTW();
      /// fit tw histogram
      virtual void fitToExponentialFunc(TH1D* h1);

    private:
      TH1D* m_h1[300];                               /**<Mean of residual as function of ADC of each board*/
      TH2D* m_h2[300];                               /**<2D histogram of residual vs ADC for each board*/

      double m_xmin = 0.07;                          /**< minimum value cut of drift length. */
      double m_ndfmin = 20;                          /**< minimum number of degree of freedom required for track. */
      double m_Pvalmin = 0.;                         /**< minimum number of Prob(chi2) of fitted track. */
      //      double m_oldTW[300][2] = {{0.}};       /**< Time Walk params of previous iteration*/
      //      double m_tw[300][3] = {{0.}};          /**< Time Walk params, result after fitting*
      double m_constTerm[300] = {0.};                /**<const term in fitting, it will be added to T0 instead tw*/
      std::vector<float> m_tw_old[300];              /**< tw list old. */
      std::vector<float> m_tw_new[300];              /**< tw list new. */
      bool m_storeHisto = true;                      /**< Store all Histogram or not*/
      bool m_useDB = false;                          /**< flag to switch btw text mode and database. */
      unsigned short m_flag[300];                    /**< flag for fit status*/
      std::string m_InputTWFileName = "tw.dat";      /**< Old tw file name. */
      std::string m_InputT0FileName = "t0.dat";      /**< Old t0 file name. */
      std::string m_outputTWFileName = "tw_new.dat"; /**< Output tw file name for time walk. */
      std::string m_outputT0FileName = "t0.dat";     /**< Output tw file name for time walk. */
      std::string m_InputRootFileName = "rootfile/output*"; /**<root input file name. */
      int m_firstExperiment = 0;                     /**< First experiment. */
      int m_firstRun = 0;                            /**< First run. */
      int m_lastExperiment = -1;                     /**< Last experiment */
      int m_lastRun = -1;                            /**< Last run. */
      unsigned short m_twParamMode_old;              /**< =0 for P0/Sqrt(ADC); =1 for P0*Exp(-P1*ADC). */
      unsigned short m_twParamMode_new = 1;          /**< =0 for P0/Sqrt(ADC); =1 for P0*Exp(-P1*ADC). */
      unsigned short m_nTwParams_new = 2;            /**< No. of tw parameters. for new database*/
      unsigned short m_nTwParams_old;                /**< No. of tw parameters. in old database */
      ClassDef(TimeWalkCalibration, 0);              /**< Time-walk calibration algorithm */
    };
  }//namespace CDC
} // namespace Belle2

