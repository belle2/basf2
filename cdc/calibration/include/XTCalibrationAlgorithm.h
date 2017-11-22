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
#include "string"
#include "TH2D.h"
#include "TH1D.h"
#include "TF1.h"
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace CDC {

    /**
     * Argument LR
     */
    enum {c_Left = 0, c_Right = 1};

    /**
     * Class to perform xt calibration for drift chamber.
     */
    class XTCalibrationAlgorithm :  public CalibrationAlgorithm {
    public:
      /// Constructor
      XTCalibrationAlgorithm();
      /// Destructor
      virtual ~XTCalibrationAlgorithm() {}
      /// set to use BField
      virtual void BField(bool bfield) {m_BField = bfield;}
      /// Run in debug or silent
      virtual void setDebug(bool debug = false) {m_debug = debug; }
      /// set minimum number of degree of freedom requirement
      virtual void setMinimumNDF(double minndf) {m_ndfmin = minndf;}
      /// set minimum Prob(Chi2) requirement
      virtual void setMinimumPval(double minPval) {m_Pvalmin = minPval;}
      /// set xt mode, 0 is polynimial, 1 is Chebshev polynomial
      virtual void setMode(unsigned short mode = 1) {m_xtmode = mode;}
      /// set to store histogram or not.
      virtual void setStoreHisto(bool storeHist = false) {m_storeHisto = storeHist;}

    protected:
      /// Run algo on data
      virtual EResult calibrate();
      /// Create histogram for calibration
      virtual void createHisto();
      /// read xt paramter (wrap text mode and database mode)
      virtual void readXT();
      /// Store calibrated constand
      virtual void write();
      /// Store histogram to file
      virtual void storeHisto();

    private:
      double m_ndfmin = 5;    /**< minimum ndf required */
      double m_Pvalmin = 0.;  /**< minimum pvalue required */
      bool m_debug = false;   /**< run in debug or silent*/
      bool m_storeHisto = true;  /**< Store histogram or not*/
      bool m_LRseparate = true; /**< Separate LR in calibration or mix*/
      bool m_BField = true;  /**< with b field or none*/

      TProfile* hprof[56][2][20][10];     /**< Profile xt histo*/
      TH2D* hist2d[56][2][20][10];        /**< 2D histo of xt*/
      TH2D* hist2d_draw[56][20][10];       /**< 2d histo for draw*/

      /*********************************
      Fit Flag
      =-1: low statitic
       =1: good
      =0: Fit failure
      =2: Error Outer
      =3: Error Inner part;
      **********************************/

      std::string m_outputXTFileName = "xt_new.dat"; /**< Out put xt filename*/
      int m_nAlphaBins; /**<number of alpha bins*/
      int m_nThetaBins; /**<number of  theta bins*/
      double u_alpha[18];/**< Upper boundays of alpha bins. */
      double ialpha[18]; /**< represented alphas of alpha bins. */
      double u_theta[7];/**< Upper boundays of theta bins. */
      double itheta[7]; /**< represented alphas of theta bins. */
      unsigned short m_xtmode = 1; /**< Mode of xt; 0 is polynomial;1 is Chebyshev.*/
    };
  }
}
