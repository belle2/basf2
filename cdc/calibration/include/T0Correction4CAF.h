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
     * Class for T0 Correction .
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
      virtual void setUseDB(bool useDB = true) {m_useDB = useDB; }
      /// store Hisotgram or not.
      virtual void storeHisto(bool storeHist = false) {m_storeHisto = storeHist;}
      /// minimum ndf require for track.
      void setMinimumNDF(double minndf) {m_ndfmin = minndf;}
      /// minimum pvalue requirement.
      void setMinimumPval(double minPval) {m_Pvalmin = minPval;}
      /// Maximum RMS of dt of all channels distribution, condition to stop iterating
      void setMaxRMSDt(double maxRMSDt) {m_maxRMSDt = maxRMSDt;}
      /// Maximum mean of dt of all channels distribution, condition to stop iterating
      void setMaxMeanDt(double maxMeanDt) {m_maxMeanDt = maxMeanDt;}

      /// output xt T0 file name (for text mode)
      void outputT0FileName(std::string outputname) {m_outputT0FileName.assign(outputname);}

    protected:
      /// Run algo on data
      virtual EResult calibrate();
      ///create histo for each channel
      virtual void createHisto();
      /// write outut or store db
      virtual void write();
    private:
      TH1F* m_hTotal;       /**< 1D histogram of delta T whole channel */
      TH1F* m_h1[56][385];    /**<1D histogram for each channel*/
      TH1F* m_hT0b[300];      /**<1D histogram for each board*/
      double m_xmin = 0.07;   /**< minimum drift length*/
      double m_ndfmin = 5;    /**< minimum ndf required */
      double m_Pvalmin = 0.;  /**< minimum pvalue required */
      /*Condition to stop iterate minDt <m_maxDt and rmsDt<m_maxRMS*/
      double m_maxMeanDt = 0.2;   /**< Mean of dT distribution  of all channels;*/
      double m_maxRMSDt = 1;   /**< RMS of dT distribution  of all channels*/
      double t0[56][385] = {{0.}};     /**< t0 */
      double dt[56][385] = {{0.}};     /**< dt of each channel */
      double err_dt[56][385] = {{0.}}; /**< error of dt of each channel*/
      double dtb[300] = {0.};        /**< dt of each board*/
      double err_dtb[300] =  {0.};    /**< error of dt of board*/

      bool m_debug;   /**< debug. */
      bool m_storeHisto; /**< store histo or not*/
      bool m_useDB; /**< use DB or text mode*/
      std::string m_outputT0FileName = "t0_new.dat"; /**<output t0 file name for text file*/
      //      ClassDef(T0Correction4CAF, 0); /**< class implementing T0 correction algorithm */
    };
  }// name space CDC
} // namespace Belle2

