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
//#include <cdc/geometry/CDCGeometryPar.h>
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
      /// change flag for debug
      virtual void setDebug(bool debug) {m_debug = debug; }
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
      /// output tw file nam, for text mode
      virtual void OutputTWFileName(std::string tw_out_name) {m_OutputTWFileName.assign(tw_out_name);}
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

    private:
      TH1D* m_h1[300]; /**<Mean of residual as function of ADC of each board*/
      TH2D* m_h2[300]; /**<2D histogram of residual vs ADC for each board*/

      double m_xmin = 0.07; /**< minimum value cut of drift length. */
      double m_ndfmin = 5;  /**< minimum number of degree of freedom required for track. */
      double m_Pvalmin = 0.; /**< minimum number of Prob(chi2) of fitted track. */
      double m_tw[300] = {0.}; /**< Time Walk params, result after fitting*/
      bool m_debug = false; /**< run debug or not.*/
      bool m_storeHisto = true;/**< Store all Histogram or not*/
      bool m_useDB = false;     /**< flag to switch btw text mode and database. */
      std::string m_InputTWFileName = "tw.dat";  /**< Input tw file name for time walk. */
      std::string m_OutputTWFileName = "tw_new.dat";  /**< Output tw file name for time walk. */
      std::string m_InputRootFileName = "rootfile/output*"; /**<root input file name. */
      int m_firstExperiment = 0; /**< First experiment. */
      int m_firstRun = 0; /**< First run. */
      int m_lastExperiment = -1; /**< Last experiment */
      int m_lastRun = -1; /**< Last run. */
      ClassDef(TimeWalkCalibration, 0); /**< Test class implementing calibration algorithm */
    };
  }//namespace CDC
} // namespace Belle2

