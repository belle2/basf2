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

    std::string m_recoTrackListName; /**< Track list name */
    std::string m_PXDInterceptListName; /**< Intercept list name */
    std::string m_ROIListName; /**< ROI list name */

    TFile* m_rootFilePtr; /**< pointer at root file used for storing infos for debugging and validating purposes */
    std::string m_rootFileName; /**< root file name */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */

    int m_rootEvent;   /**<  event number*/

    //graphs & histos - results
    Double_t pt[6]  = {0.05, 0.15, 0.25, 0.4, 0.75, 1.5}; /**< bin edges (in pt = transverse momentum)*/
    Double_t ptErr[6] = { 0.05, 0.05, 0.05, 0.1, 0.25, 0.5}; /**< bin widths (transverse momentum) */

    TGraphErrors* m_gEff; /**< efficiency graph */
    TH1F* m_h1digiIn; /**< digits contained in ROI histogram*/
    TH1F* m_h1digiOut2; /**< lost digit: ROI exist with right vxdID */
    TH1F* m_h1digiOut3; /**< lost digit: ROI exist with wrong vxdID */
    TH1F* m_h1digiOut4; /**< lost digit: ROI does not exist, intercept with right vxdID */
    TH1F* m_h1digiOut5; /**< lost digit: ROI does not exist, intercept with wrong vxdID */

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

    TH1F* m_hNhits; /**< number of hits per candidate*/

    TH2F* m_h2Map; /**<sensor perp,phi */
    TH2F* m_h2MapBad_L1; /**<sensor perp,phi - no digit in - layer2*/
    TH2F* m_h2MapBad_L2; /**<sensor perp,phi - no digit in - layer1*/

    //variables
    double m_globalTime; /**< global hit time */
    double m_coorU; /**< intercept U coordinate*/
    double m_coorV; /**< intercept V coordinate*/
    double m_sigmaU; /**< intercept U stat error*/
    double m_sigmaV; /**< intercept V stat error*/
    int m_vxdID; /**< VXD ID*/

    double m_coorUmc; /**< true intercept U coordinate*/
    double m_coorVmc; /**< true intercept V coordinate*/
    int m_Uidmc; /**< true intercept U id  */
    int m_Vidmc; /**< true intercept V id  */
    int m_vxdIDmc; /**< true intercept VXD id  */
    double pT; /**< transverse momentum */
    double m_momXmc; /**< true p along X */
    double m_momYmc;/**< true p along Y */
    double m_momZmc; /**< true p along Z */
    double m_thetamc; /**< true theta*/
    //    double m_theta;
    double m_costhetamc; /**< true cos theta */
    //    double m_costhetaMCPart; /**< cos theta of MC particle */
    double m_phimc;  /**< true phi */
    double m_lambdamc;  /**< true lambda = pi/2 - theta*/

    unsigned int n_tracks; /**< number of tracks */
    unsigned int n_tracksWithDigits; /**< number of tracks with digits */
    unsigned int npxdDigit[6]; /**< number of pxd digits in bins of pt*/
    unsigned int npxdDigitInROI[6]; /**< number of pxd digits inside ROI in bins of pt*/
    unsigned int n_pxdDigit; /**< number of pxd digits*/
    unsigned int n_pxdDigitInROI; /**< number of pxd digits*/
    //    unsigned int n_noHit;
    unsigned int n_noHit2; /**< number of lost digits: no hit, correct vxdID*/
    unsigned int n_noHit3;/**< number of lost digits: no hit, wrong vxdID*/
    unsigned int n_noROI4; /**< number of lost digits: no ROI, intercepts with correct vxdID*/
    unsigned int n_noROI5; /**< number of lost digits: no ROI, intercepts with wrong vxdID*/
    unsigned int nnoHit2[6]; /**< number of lost digits in bins of pt: no hit, correct vxdID*/
    unsigned int nnoHit3[6]; /**< number of lost digits in bins of pt: no hit, wrong vxdID*/
    unsigned int nnoROI4[6];  /**< number of lost digits in bins of pt: no ROI, intercepts with correct vxdID*/
    unsigned int nnoROI5[6];  /**< number of lost digits in bins of pt: no ROI, intercepts with wrong vxdID*/

    unsigned int NtrackHit; /**< nuner of tracks with hits */
    unsigned int NtrackNoHit2; /**< nuner of tracks with no digits in ROI (correct vxdID) */
    unsigned int NtrackNoHit3; /**< nuner of tracks with no digits in ROI (wrong vxdID) */
    unsigned int NtrackNoROI4; /**< nuner of tracks with no ROI (intercept with correct vxdID) */
    unsigned int NtrackNoROI5; /**< nuner of tracks with no ROI (intercept with wrong vxdID) */

  };

}

#endif
