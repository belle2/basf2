/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include "string"
#include "TH2D.h"
#include "TH1D.h"
#include "TF1.h"
#include <calibration/CalibrationAlgorithm.h>
#include <cdc/dbobjects/CDCGeometry.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {
  namespace CDC {

    /**
     * Argument LR.
     */
    enum {c_Left = 0, c_Right = 1};

    /**
     * Argument of fitting function.
     */
    enum {c_Polynomial = 0, c_Chebyshev = 1};

    /**
     * Fit Status.
     * =-1: low statitic
     * =1: good
     * =0: Fit failure
     * =2: Error Outer
     * =3: Error Inner part;
     */

    enum FitStatus {c_lowStat = -1, c_fitFailure = 0, c_OK = 1,
                    c_errorOuter = 2, c_errorInner = 3
                   };

    /**
     * Class to perform xt calibration for drift chamber.
     */
    class XTCalibrationAlgorithm :  public CalibrationAlgorithm {
    public:
      /// Constructor
      XTCalibrationAlgorithm();

      /// Destructor
      ~XTCalibrationAlgorithm() {}

      /// set to use BField
      void setBField(bool bfield) {m_bField = bfield;}

      /// Run in debug or silent
      void setDebug(bool debug = false) {m_debug = debug; }

      /// set minimum number of degree of freedom requirement
      void setMinimumNDF(double ndf) {m_minNdf = ndf;}

      /// set minimum Prob(Chi2) requirement
      void setMinimumPval(double pval) {m_minPval = pval;}

      /// set xt mode, 0 is polynimial, 1 is Chebshev polynomial
      void setXtMode(unsigned short mode = c_Chebyshev) {m_xtMode = mode;}

      /// set to store histogram or not.
      void setStoreHisto(bool storeHist = false) {m_storeHisto = storeHist;}

      /// Enable text output of calibration result
      void enableTextOutput(bool output = true) {m_textOutput = output;}

      /// output file name
      void setOutputFileName(std::string outputname) {m_outputFileName.assign(outputname);}

      /// Set name for histogram output
      void setHistFileName(const std::string& name) {m_histName = "histXT_" + name + ".root";}

      /// Set LR separate mode (default is true).
      void setLRSeparate(bool lr = true) {m_LRseparate = lr;}

      /// Set threshold for the fraction of fitted results.
      void setThreshold(double th = 0.6) {m_threshold = th;}


    protected:

      /// Run algo on data
      EResult calibrate() override;

      /// Create histogram for calibration
      void createHisto();

      /// Store calibrated constand
      void write();

      /// Store histogram to file
      void storeHisto();

      /// Prepare the calibration of XT.
      void prepare();

      /// Check the convergence of XT fit.
      EResult checkConvergence();

      /// Check if there are any wrong xt functions.
      void sanitaryCheck();
    private:
      double m_minNdf = 5;    /**< minimum ndf required */
      double m_minPval = 0.;  /**< minimum pvalue required */
      bool m_debug = false;   /**< run in debug or silent*/
      bool m_storeHisto = true;  /**< Store histogram or not*/
      bool m_LRseparate = true; /**< Separate LR in calibration or mix*/
      bool m_bField = true;  /**< with b field or none*/
      double m_threshold = 0.6;  /**< minimal requirement for the fraction of fitted results */

      TProfile* m_hProf[56][2][20][10];     /**< Profile xt histo*/
      TH2F* m_hist2d[56][2][20][10];        /**< 2D histo of xt*/
      TH2F* m_hist2dDraw[56][20][10];       /**< 2d histo for draw*/
      TH1F* m_hist2d_1[56][2][20][10];      /**< 1D xt histo, results of slice fit */
      TF1* m_xtFunc[56][2][20][10];         /**< XTFunction */

      double m_xtPrior[56][2][18][7][8];     /**< paremeters of XT before calibration */

      int m_fitStatus[56][2][20][10];       /**< Fit flag */
      bool m_useSliceFit = false; /**< Use slice fit or profile */
      int m_minEntriesRequired = 1000; /**< minimum number of hit per hitosgram. */
      int m_nAlphaBins; /**<number of alpha bins*/
      int m_nThetaBins; /**<number of  theta bins*/
      int m_xtMode = c_Chebyshev;  /**< Mode of xt; 0 is polynomial;1 is Chebyshev.*/
      int m_xtModePrior;   /**< Mode of xt before calibration; 0 is polynomial;1 is Chebyshev.*/
      float m_lowerAlpha[18];/**< Lower boundays of alpha bins. */
      float m_upperAlpha[18];/**< Upper boundays of alpha bins. */
      float m_iAlpha[18]; /**< Represented alpha in alpha bins. */
      float m_lowerTheta[7];/**< Lower boundays of theta bins. */
      float m_upperTheta[7];/**< Upper boundays of theta bins. */
      float m_iTheta[7]; /**< Represented theta in theta bins. */
      /// boundary parameter for fitting, semi-experiment number
      double m_par6[56] = {89, 91, 94, 99, 104, 107, 110, 117,
                           126, 144, 150, 157, 170, 180,
                           160, 167, 183, 205, 200, 194,
                           177, 189, 192, 206, 224, 234,
                           193, 206, 209, 215, 222, 239,
                           204, 212, 217, 227, 235, 240,
                           215, 222, 230, 239, 246, 253,
                           227, 232, 239, 243, 253, 258,
                           231, 243, 246, 256, 263, 300
                          };

      bool  m_textOutput = false; /**< output text file if true */
      std::string m_outputFileName = "xt_new.dat"; /**< Output xt filename*/
      std::string m_histName = "histXT.root"; /**< root file name */
      DBObjPtr<CDCGeometry> m_cdcGeo; /**< Geometry of CDC */
    };
  }
}
