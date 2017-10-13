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
#include <TH1F.h>
#include "vector"
#include "string"
namespace Belle2 {
  namespace CDC {
    /**
     * Class for T0 correction algorithm.
     */
    class T0Correction4CAF: public CalibrationAlgorithm {
    public:
      /// Constructor.
      T0Correction4CAF();
      /// Destructor
      virtual ~T0Correction4CAF() {}
      /// turn on/off debug.
      virtual void setDebug(bool debug = false) {m_debug = debug; }
      /// use DB or text mode.
      virtual void setUseDB(bool useDB = false) {m_useDB = useDB; }
      /// store Hisotgram or not.
      virtual void storeHisto(bool storeHist = false) {m_storeHisto = storeHist;}
      /// minimum ndf require for track.
      void setMinimumNDF(double minndf) {m_ndfmin = minndf;}
      /// minimum pvalue requirement.
      void setMinimumPval(double minPval) {m_Pvalmin = minPval;}
      /// input root file name.
      void inputFileNames(std::string inputname) {m_inputRootFileName.assign(inputname);}
      /// output xt T0 file name (for text mode).
      void outputFileName(std::string outputname) {m_outputT0FileName.assign(outputname);}
      /// run t0 correction.
      //      void execute()
      //{
      //  calibrate();
      //}

    protected:
      /// Run algo on data
      virtual EResult calibrate();
      /// write outut or store db
      virtual void Write();
    private:
      TH1F m_hTotal;       /**< 1D histogram of delta T whole channel */
      TH1F m_h1[56][385];    /**<1D histogram for each channel*/
      TH1F m_hT0b[300];      /**<1D histogram for each board*/
      double m_xmin = 0.07;   /**< minimum drift length*/
      double m_ndfmin = 5;    /**< minimum ndf required */
      double m_Pvalmin = 0.;  /**< minimum pvalue required */
      double t0[56][385];     /**< t0 */
      double dt[56][385];     /**< dt of each channel */
      double err_dt[56][385]; /**< error of dt of each channel*/
      double dtb[300];        /**< dt of each board*/
      double err_dtb[300];    /**< error of dt of board*/

      bool m_debug;   /**< debug. */
      bool m_storeHisto; /**< store histo or not*/
      bool m_useDB; /**< use DB or text mode*/
      std::string m_outputT0FileName = "t0_new.dat"; /**<output t0 file name for text file*/
      std::string m_inputRootFileName = "rootfile/output*"; /**< input file names*/
      int m_firstExperiment = 0; /**< First experiment. */
      int m_firstRun = 0; /**< First run. */
      int m_lastExperiment = -1; /**< Last experiment */
      int m_lastRun = -1; /**< Last run. */
      ClassDef(T0Correction4CAF, 0); /**< class implementing T0 correction algorithm */
    };
  }// name space CDC
} // namespace Belle2

