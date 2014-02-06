/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/

#ifndef PXD_DATA_REDUCTION_ANALYSIS_MODULE_H_
#define PXD_DATA_REDUCTION_ANALYSIS_MODULE_H_

#include <framework/core/Module.h>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraphErrors.h>

namespace Belle2 {

  /** The PXD Data Reduction Analysis Module
   *
   * this module performs the analysis of the PXD data redution module performances
   *
   */

  class PXDDataRedAnalysisModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    PXDDataRedAnalysisModule();

    /**
     * Destructor of the module.
     */
    virtual ~PXDDataRedAnalysisModule();

    /**
     *Initializes the Module.
     */
    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();

  private:

    std::string m_gfTrackCandsColName;
    std::string m_PXDInterceptListName;
    std::string m_ROIListName;

    TFile* m_rootFilePtr; /**< pointer at root file used for storing infos for debugging and validating purposes */
    std::string m_rootFileName;
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */

    int m_rootEvent;   /**<  event number*/

    //graphs & histos - results
    Double_t pt[6]  = {1.5, 0.75, 0.4, 0.25, 0.15, 0.05};
    Double_t ptErr[6]    = { 0.5, 0.25, 0.1, 0.05, 0.05, 0.05};
    TGraphErrors* m_gEff;
    TH1F* m_h1digiIn;
    TH1F* m_h1digiOut2;
    TH1F* m_h1digiOut3;
    TH1F* m_h1digiOut4;
    TH1F* m_h1digiOut5;

    //histograms
    TH1F* m_h1ptAll; /**< distribution of transverse momentum for all PDXDigits*/
    TH1F* m_h1pt; /**< distribution of transverse momentum for PDXDigits contained in a ROI*/
    TH1F* m_h1ptBad; /**< distribution of transverse momentum for PDXDigits  when no ROI exists and intercept has wrong vxdid*/

    TH1F* m_h1GlobalTime; /**< distribution of global time for PDXDigits contained in a ROI*/
    TH1F* m_h1GlobalTimeFail; /**< distribution of global time for PDXDigits not contained in a ROI*/
    TH1F* m_h1GlobalTimeNoROI; /**< distribution of global time for PDXDigits when no ROI exists*/
    TH1F* m_h1GlobalTimeBad; /**< distribution of global time for PDXDigits when no ROI exists and intercept has wrong vxdid*/

    TH1F* m_h1CoorUBad; /**< distribution of U cell positions for PDXDigits when no ROI exists and vxdid intercept is wrong*/
    TH1F* m_h1CoorVBad; /**< distribution of U cell positions for PDXDigits when no ROI exists and vxdid intercept is wrong*/
    TH2F* m_h2CoorUVBad; /**< distribution of U cell positions for PDXDigits when no ROI exists and vxdid intercept is wrong*/

    TH1F* m_h1PullU; /**< distribution of U pulls for PDXDigits contained in a ROI*/
    TH1F* m_h1PullV; /**< distribution of V pulls for PDXDigits contained in a ROI*/
    TH1F* m_h1PullUFail; /**< distribution of U pulls for PDXDigits NOT contained in a ROI*/
    TH1F* m_h1PullVFail; /**< distribution of V pulls for PDXDigits NOT contained in a ROI*/
    TH1F* m_h1PullUNoROI; /**< distribution of U pulls for PDXDigits when no ROI exists*/
    TH1F* m_h1PullVNoROI; /**< distribution of V pulls for PDXDigits when no ROI exists*/

    TH1F* m_h1ResidU; /**< distribution of U resid for intercepts contained in a ROI*/
    TH1F* m_h1ResidV; /**< distribution of V resid for intercepts contained in a ROI*/
    TH1F* m_h1ResidUFail; /**< distribution of U resid for intercepts NOT contained in a ROI*/
    TH1F* m_h1ResidVFail; /**< distribution of V resid for intercepts NOT contained in a ROI*/
    TH1F* m_h1ResidUNoROI; /**< distribution of U resid for intercepts when no ROI exists*/
    TH1F* m_h1ResidVNoROI; /**< distribution of V resid for intercepts when no ROI exists*/

    TH1F* m_h1SigmaU; /**< distribution of sigmaU for intercepts contained in a ROI*/
    TH1F* m_h1SigmaV; /**< distribution of sigmaV for intercepts contained in a ROI*/
    TH1F* m_h1SigmaUFail; /**< distribution of sigmaU for intercepts NOT contained in a ROI*/
    TH1F* m_h1SigmaVFail; /**< distribution of sigmaV for intercepts NOT contained in a ROI*/
    TH1F* m_h1SigmaUNoROI; /**< distribution of sigmaU for intercepts when no ROI exists*/
    TH1F* m_h1SigmaVNoROI; /**< distribution of sigmaV for intercepts when no ROI exists*/

    TH1F* m_h1DistUFail;  /**< distribution of distance between ROI and PXDDigits not contained in it in U direction */
    TH1F* m_h1DistVFail;  /**< distribution of distance between ROI and PXDDigits not contained in it in V direction */
    TH2F* m_h2DistUVFail;  /**< distribution of distance between ROI and PXDDigits not contained in it in U,V plane */

    TH1F* m_h1totROIs; /**< distribution of number of ROIs*/
    TH1F* m_h1nROIs; /**< distribution of number of ROIs*/
    TH1F* m_h1nROIs_all; /**< distribution of number of ROIs*/
    TH1F* m_h1redFactor; /**< distribution of number of ROIs*/

    TH1F* m_h1totarea; /**< distribution of ROI areas*/
    TH1F* m_h1area; /**< distribution of ROI areas*/
    TH1F* m_h1areaFail; /**< distribution of ROI areas when PXDDigit is not contained*/

    TH1F* m_h1Theta; /**< distribution of theta when the PXDDigit is contained in a ROI */
    TH1F* m_h1ThetaBad; /**< distribution of theta when the PXDDigit is contained in a ROI */
    TH1F* m_h1CosTheta; /**< distribution of costheta when the PXDDigit is contained in a ROI */
    TH1F* m_h1CosThetaMCPart; /**< distribution of costheta when the PXDDigit is contained in a ROI */
    TH1F* m_h1CosThetaBad; /**< distribution of costheta when the PXDDigit is contained in a ROI */
    TH1F* m_h1Phi; /**< distribution of phi when the PXDDigit is contained in a ROI */
    TH1F* m_h1Lambda; /**< distribution of lambda when the PXDDigit is contained in a ROI */
    TH1F* m_h1PhiBad; /**< distribution of phi when no ROI and intercept has wrong vxdID */
    TH1F* m_h1PhiBad_L1; /**< distribution of phi when no ROI and intercept has wrong vxdID (Layer1)*/
    TH1F* m_h1PhiBad_L2; /**< distribution of phi when no ROI and intercept has wrong vxdID (Layer1)*/
    TH1F* m_h1PhiBadLambda0_L2; /**< distribution of phi when no ROI and intercept has wrong vxdID (Layer1)*/
    TH1F* m_h1PhiBadLambda0_L1; /**< distribution of phi when no ROI and intercept has wrong vxdID (Layer1)*/
    TH1F* m_h1PhiBadLambdaF_L2; /**< distribution of phi when no ROI and intercept has wrong vxdID (Layer1)*/
    TH1F* m_h1PhiBadLambdaF_L1; /**< distribution of phi when no ROI and intercept has wrong vxdID (Layer1)*/
    TH1F* m_h1LambdaBad; /**< distribution of phi when no ROI and intercept has wrong vxdID */
    TH1F* m_h1LambdaBad_timeL1; /**< distribution of phi when no ROI and intercept has wrong vxdID and GlobalTime<1*/
    TH1F* m_h1LambdaBad_timeG1; /**< distribution of phi when no ROI and intercept has wrong vxdID and GlobalTime<1*/

    TH1F* m_hNhits;
    TH1F* m_hNhitsBad;

    TH2F* m_h2Map;
    TH2F* m_h2MapBad_L1;
    TH2F* m_h2MapBad_L2;

    //variables
    double m_globalTime;
    double m_coorU;
    double m_coorV;
    double m_sigmaU;
    double m_sigmaV;
    int m_vxdID;

    double m_coorUmc;
    double m_coorVmc;
    int m_Uidmc;
    int m_Vidmc;
    int m_vxdIDmc;
    double pT;
    double m_momXmc;
    double m_momYmc;
    double m_momZmc;
    double m_thetamc;
    //    double m_theta;
    double m_costhetamc;
    double m_costhetaMCPart;
    double m_phimc;
    double m_lambdamc;

    unsigned int n_tracks;
    unsigned int n_tracksWithDigits;
    unsigned int npxdDigit[6];
    unsigned int npxdDigitInROI[6];
    unsigned int n_pxdDigit;
    unsigned int n_pxdDigitInROI;
    //    unsigned int n_noHit;
    unsigned int n_noHit2;
    unsigned int n_noHit3;
    unsigned int n_noROI4;
    unsigned int n_noROI5;
    unsigned int nnoHit2[6];
    unsigned int nnoHit3[6];
    unsigned int nnoROI4[6];
    unsigned int nnoROI5[6];

    unsigned int NtrackHit;
    unsigned int NtrackNoHit2;
    unsigned int NtrackNoHit3;
    unsigned int NtrackNoROI4;
    unsigned int NtrackNoROI5;


    int m_nNoMCPart;
  };

}

#endif
