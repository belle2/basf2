/**************************************************************************
- * basf2 (Belle II Analysis Software Framework)                           *
- * Author: The Belle II Collaboration                                     *
- *                                                                        *
- * See git log for contributors and copyright holders.                    *
- * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
- **************************************************************************/
#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <TH1F.h>
#include <cdc/dbobjects/CDCGeometry.h>
#include <framework/database/DBObjPtr.h>
#include "string"
namespace Belle2 {
  namespace CDC {
    /**
     * Class for T0 Correction .
     */
    class T0CalibrationAlgorithm: public CalibrationAlgorithm {
    public:
      /// Constructor.
      T0CalibrationAlgorithm();
      /// Destructor
      ~T0CalibrationAlgorithm() {}
      /// store Hisotgram or not.
      void storeHisto(bool storeHist = false) {m_storeHisto = storeHist;}
      /// minimum ndf require for track.
      void setMinimumNDF(double minndf) {m_ndfmin = minndf;}
      /// minimum pvalue requirement.
      void setMinimumPval(double minPval) {m_Pvalmin = minPval;}
      /// Maximum RMS of dt of all channels distribution, condition to stop iterating
      void setMaxRMSDt(double maxRMSDt) {m_maxRMSDt = maxRMSDt;}
      /// Maximum mean of dt of all channels distribution, condition to stop iterating
      void setMaxMeanDt(double maxMeanDt) {m_maxMeanDt = maxMeanDt;}
      /// Maximum mean of dt of all channels distribution, condition to stop iterating
      void setOffsetMeanDt(double offsetMeanDt) {m_offsetMeanDt = offsetMeanDt;}
      /// Maximum channel in which T0 is still need to be calibrated
      void setMaxBadChannel(double max_bad_channel) {m_maxBadChannel = max_bad_channel;}

      /// set common T0
      void setCommonT0(double commonT0) {m_commonT0 = commonT0;}

      /// Enable text output of calibration result
      void enableTextOutput(bool output = true) {m_textOutput = output;}

      /// output xt T0 file name (for text mode)
      void setOutputFileName(std::string outputname) {m_outputT0FileName.assign(outputname);}

      /// Set name for histogram output
      void setHistFileName(const std::string& name) {m_histName = "histT0_" + name + ".root";}

    protected:
      /// Run algo on data
      EResult calibrate() override;
      ///create histo for each channel
      void createHisto();
      /// write outut or store db
      void write();
      /// calculate mean of the T0 distribution
      double getMeanT0(TH1F* h1);
    private:
      TH1F* m_hTotal;       /**< 1D histogram of delta T whole channel */
      TH1F* m_h1[56][385];    /**<1D histogram for each channel*/
      TH1F* m_hT0b[300];      /**<1D histogram for each board*/
      double m_xmin = 0.07;   /**< minimum drift length*/
      double m_ndfmin = 5;    /**< minimum ndf required */
      double m_Pvalmin = 0.;  /**< minimum pvalue required */
      /*Condition to stop iterate minDt <m_maxDt and rmsDt<m_maxRMS*/
      double m_maxMeanDt = 0.2;   /**< Mean of dT distribution  of all channels;*/
      double m_offsetMeanDt = -0.4;   /**< offset dT distribution caused by event timing extraction;*/
      double m_maxRMSDt = 0.22;   /**< RMS of dT distribution  of all channels*/
      double m_maxBadChannel = 50;   /**< Number of channels which has DeltaT0 larger then 0.5*/
      double dt[56][385] = {{0.}};     /**< dt of each channel */
      double err_dt[56][385] = {{0.}}; /**< error of dt of each channel*/
      double dtb[300] = {0.};        /**< dt of each board*/
      double err_dtb[300] =  {0.};    /**< error of dt of board*/
      double m_commonT0 = 4825.;  /**< A common T0 of all channels*/

      bool m_storeHisto = false; /**< store histo or not*/
      bool  m_textOutput = false; /**< output text file if true */
      std::string m_outputT0FileName = "t0_new.dat"; /**<output t0 file name for text file*/
      std::string m_histName = "histT0.root"; /**< root file name */
      DBObjPtr<CDCGeometry> m_cdcGeo; /**< Geometry of CDC */
    };
  }// name space CDC
} // namespace Belle2

