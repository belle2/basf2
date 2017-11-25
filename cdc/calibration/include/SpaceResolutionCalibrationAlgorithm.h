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

#include <TH1F.h>
#include <TH2F.h>
#include <TGraphErrors.h>
#include <TProfile.h>
#include <TF1.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include <TSystem.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace CDC {

    /**
     * Class for Space resolution calibration.
     */
    class SpaceResolutionCalibrationAlgorithm :  public CalibrationAlgorithm {
      //      typedef std::array<float, 3> array3; /**< angle bin info. */
    public:

      /// Constructor
      SpaceResolutionCalibrationAlgorithm();

      /// Destructor
      virtual ~SpaceResolutionCalibrationAlgorithm() {}

      /// Set Debug mode.
      virtual void setDebug(bool debug = false) {m_debug = debug; }

      /// minimum NDF required for track
      virtual void setMinimumNDF(double ndf) {m_minNdf = ndf;}

      /// Minimum Pval required
      virtual void setMinimumPval(double pval) {m_minPval = pval;}

      /// Bin width of each slide
      virtual void setBinWidth(double bw) {m_binWidth = bw;}

      /// Work with B field or not;
      virtual void setBField(bool bfield) {m_bField = bfield;}

      /// Store histograms durring the calibration or not
      virtual void setStoreHisto(bool storeHist = false) {m_storeHisto = storeHist;}

    protected:
      /// Run algo on data
      virtual EResult calibrate();
      /// create histogram
      virtual void createHisto();
      /// read sigma bining (alpha, theta bining)
      virtual void readProfile();
      /// store histogram
      virtual void storeHisto();
      /// save calibration, in text file or db
      virtual void write();

    private:
      static const int Max_nalpha = 18; /**< Maximum alpha bin.*/
      static const int Max_ntheta = 7; /**< maximum theta bin  */
      static const unsigned short Max_np = 40; /**< Maximum number of point =1/binwidth */

      double m_minNdf = 5; /**< Minimum NDF  */
      double m_minPval = 0.; /**<  Minimum Prob(chi2) of track*/
      double m_binWidth = 0.05; /**<width of each bin, unit cm*/
      bool m_debug = false;   /**< Debug or not */
      bool m_draw = false;    /**< print out histogram in pdf file or not*/
      bool m_storeHisto = false; /**<  Store histogram or not*/
      bool m_bField = true;                   /**< Work with BField, fit range and initial parameters is different incase B and noB */
      double sigma_new[56][2][18][7][8]; /**<new sigma prameters.*/
      TF1* ffit[56][2][18][7];           /**< fitting function*/
      TGraphErrors* gfit[56][2][18][7];  /**< sigma*sigma graph for fit*/
      TGraphErrors* gr[56][2][18][7];    /**< sigma graph.*/
      TH2F* hist_b[56][2][Max_nalpha][Max_ntheta]; /**< 2D histogram of biased residual */
      TH2F* hist_u[56][2][Max_nalpha][Max_ntheta]; /**< 2D histogram of unbiased residual */
      TH1F* hu_m[56][2][Max_nalpha][Max_ntheta]; /**<  mean histogram biased residual*/
      TH1F* hu_s[56][2][Max_nalpha][Max_ntheta]; /**<  sigma histogram of biased residual*/
      TH1F* hb_m[56][2][Max_nalpha][Max_ntheta]; /**<  mean histogram of unbiased residual*/
      TH1F* hb_s[56][2][Max_nalpha][Max_ntheta]; /**<  sigma histogram of ubiased residual*/
      int m_fitStatus[56][2][Max_nalpha][Max_ntheta] = {{{{0}}}} ; /**< Fit flag; 1:OK ; 0:error*/

      int m_nAlphaBins; /**<number of alpha bins*/
      int m_nThetaBins;/**<number of  theta bins*/
      double m_lowerAlpha[18];/**< Lower boundays of alpha bins. */
      double m_upperAlpha[18];/**< Upper boundays of alpha bins. */
      double m_iAlpha[18]; /**< represented alphas of alpha bins. */
      double m_lowerTheta[7]; /**< Lower boundays of theta bins. */
      double m_upperTheta[7];/**< Upper boundays of theta bins. */
      double m_iTheta[7]; /**< represented alphas of theta bins. */
      unsigned short m_sigmaParamMode = 1; /**< sigma mode for this calibration.*/

      std::string m_outputSigmaFileName = "sigma_new.dat"; /**< Output sigma file name */
    };
  }
}
