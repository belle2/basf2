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

      /// Enable text output of calibration result
      void enableTextOutput(bool output = true) {m_textOutput = output;}

      /// output file name
      void setOutputFileName(std::string outputname) {m_outputFileName.assign(outputname);}


    protected:
      /// Run algo on data
      virtual EResult calibrate();
      /// create histogram
      virtual void createHisto();
      /// store histogram
      virtual void storeHisto();
      /// save calibration, in text file or db
      virtual void write();
      /// Prepare the calibration of space resolution.
      void prepare(StoreObjPtr<EventMetaData>& evtPtr);

    private:
      static const int Max_nalpha = 18; /**< Maximum alpha bin.*/
      static const int Max_ntheta = 7; /**< maximum theta bin  */
      static const unsigned short Max_np = 40; /**< Maximum number of point =1/binwidth */

      double m_minNdf = 5; /**< Minimum NDF  */
      double m_minPval = 0.; /**<  Minimum Prob(chi2) of track*/
      double m_binWidth = 0.05; /**<width of each bin, unit cm*/
      bool m_debug = false;   /**< Debug or not */
      bool m_storeHisto = false; /**<  Store histogram or not*/
      bool m_bField = true;                   /**< Work with BField, fit range and initial parameters is different incase B and noB */
      double m_sigma[56][2][18][7][8]; /**<new sigma prameters.*/
      TGraphErrors* m_gFit[56][2][18][7];  /**< sigma*sigma graph for fit*/
      TGraphErrors* m_graph[56][2][18][7];    /**< sigma graph.*/
      TH2F* m_hBiased[56][2][Max_nalpha][Max_ntheta]; /**< 2D histogram of biased residual */
      TH2F* m_hUnbiased[56][2][Max_nalpha][Max_ntheta]; /**< 2D histogram of unbiased residual */
      TH1F* m_hMeanBiased[56][2][Max_nalpha][Max_ntheta]; /**<  mean histogram biased residual*/
      TH1F* m_hSigmaBiased[56][2][Max_nalpha][Max_ntheta]; /**<  sigma histogram of biased residual*/
      TH1F* m_hMeanUnbiased[56][2][Max_nalpha][Max_ntheta]; /**<  mean histogram of unbiased residual*/
      TH1F* m_hSigmaUnbiased[56][2][Max_nalpha][Max_ntheta]; /**<  sigma histogram of ubiased residual*/
      int m_fitStatus[56][2][Max_nalpha][Max_ntheta] = {{{{0}}}} ; /**< Fit flag; 1:OK ; 0:error*/

      int m_nAlphaBins; /**<number of alpha bins*/
      int m_nThetaBins;/**<number of  theta bins*/
      float m_lowerAlpha[18];/**< Lower boundays of alpha bins. */
      float m_upperAlpha[18];/**< Upper boundays of alpha bins. */
      float m_iAlpha[18]; /**< represented alphas of alpha bins. */
      float m_lowerTheta[7]; /**< Lower boundays of theta bins. */
      float m_upperTheta[7];/**< Upper boundays of theta bins. */
      float m_iTheta[7]; /**< represented alphas of theta bins. */
      unsigned short m_sigmaParamMode = 0; /**< sigma mode for this calibration.*/

      double m_sigmaPost[56][2][18][7][8]; /**< sigma prameters before calibration */
      unsigned short m_sigmaParamModePost; /**< sigma mode before this calibration.*/

      bool  m_textOutput = false; /**< output text file if true */
      std::string m_outputFileName = "sigma_new.dat"; /**< Output sigma filename*/
    };
  }
}
