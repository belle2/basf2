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

  class PXDROIFinderAnalysisModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    PXDROIFinderAnalysisModule();

    /**
     * Destructor of the module.
     */
    virtual ~PXDROIFinderAnalysisModule();

    /**
     *Initializes the Module.
     */
    void initialize() override;

    void beginRun() override;

    void event() override;

    void endRun() override;

    /**
     * Termination action.
     */
    void terminate() override;

  private:

    std::string m_recoTrackListName; /**< Track list name */
    std::string m_PXDInterceptListName; /**< Intercept list name */
    std::string m_ROIListName; /**< ROI list name */

    TFile* m_rootFilePtr; /**< pointer at root file used for storing infos for debugging and validating purposes */
    std::string m_rootFileName; /**< root file name */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */

    int m_rootEvent;   /**<  event number*/

    //pxd sensor number
    Int_t m_nSensorsL1; /**<  number of sensors on L1*/
    Int_t m_nSensorsL2; /**<  number of sensors on L2*/

    //graphs & histos - results
    Double_t pt[6]  = {0.05, 0.15, 0.25, 0.4, 0.75, 1.5}; /**< bin edges (in pt = transverse momentum)*/
    Double_t ptErr[6] = { 0.05, 0.05, 0.05, 0.1, 0.25, 0.5}; /**< bin widths (transverse momentum) */

    TGraphErrors* m_gEff2; /**< efficiency graph */
    TGraphErrors* m_gEff; /**< efficiency graph */

    TH1F* m_h1DigitsPerParticle;
    TH1F* m_h1RecoTracksPerParticle;

    TH1F* m_h1digiIn; /**< digits contained in ROI histogram*/
    TH1F* m_h1digiOut2; /**< lost digit: ROI exist with right vxdID */
    TH1F* m_h1digiOut3; /**< lost digit: ROI exist with wrong vxdID */
    TH1F* m_h1digiOut4; /**< lost digit: ROI does not exist, intercept with right vxdID */
    TH1F* m_h1digiOut5; /**< lost digit: ROI does not exist, intercept with wrong vxdID */

    TH1F* m_h1TrackOneDigiIn; /**<tracks with at least digit contained in ROI*/
    TH1F* m_h1nnotINtrack2; /**< tracks with lost digit: ROI exist with right vxdID */
    TH1F* m_h1nnotINtrack3; /**< lost digit: ROI exist with wrong vxdID */
    TH1F* m_h1nnotINtrack4; /**< lost digit: ROI does not exist, intercept with right vxdID */
    TH1F* m_h1nnotINtrack5; /**< lost digit: ROI does not exist, intercept with wrong vxdID */

    TH1F* m_h1Track; /**< denominator track  */
    TH1F* m_h1Track_pt; /**< denominator track pT*/
    TH1F* m_h1Track_phi; /**< denominator track phi*/
    TH1F* m_h1Track_lambda; /**< denominator track lambda*/
    TH1F* m_h1Track_cosTheta; /**< denominator track cosTheta*/
    TH1F* m_h1Track_pVal; /**< denominator track pVal*/
    TH1F* m_h1Track_nSVDhits; /**< denominator track pVal*/
    TH1F* m_h1Track_nCDChits; /**< denominator track pVal*/

    TH1F* m_h1INtrack1; /**< track with no intercept  */
    TH1F* m_h1INtrack1_pt; /**<  track with no intercept pT*/
    TH1F* m_h1INtrack1_phi; /**< track with no intercept phi*/
    TH1F* m_h1INtrack1_lambda; /**< track with no intercept lambda*/
    TH1F* m_h1INtrack1_cosTheta; /**< track with no intercept costheta*/
    TH1F* m_h1INtrack1_pVal; /**< denominator track pVal*/
    TH1F* m_h1INtrack1_nSVDhits; /**< denominator track pVal*/
    TH1F* m_h1INtrack1_nCDChits; /**< denominator track pVal*/

    TH1F* m_h1notINtrack5; /**< track with no intercept  */
    TH1F* m_h1notINtrack5_pt; /**<  track with no intercept pT*/
    TH1F* m_h1notINtrack5_phi; /**< track with no intercept phi*/
    TH1F* m_h1notINtrack5_lambda; /**< track with no intercept lambda*/
    TH1F* m_h1notINtrack5_cosTheta; /**< track with no intercept costheta*/
    TH1F* m_h1notINtrack5_pVal; /**< denominator track pVal*/
    TH1F* m_h1notINtrack5_nSVDhits; /**< denominator track pVal*/
    TH1F* m_h1notINtrack5_nCDChits; /**< denominator track pVal*/


    //fill digits inside ROIs
    TH1F* m_h1PullU; /**< distribution of U pulls for PDXDigits contained in a ROI*/
    TH1F* m_h1PullV; /**< distribution of V pulls for PDXDigits contained in a ROI*/
    TH2F* m_h2sigmaUphi; /**< distribution of sigmaU VS phi for PDXDigits contained in a ROI*/
    TH2F* m_h2sigmaVphi; /**< distribution of sigmaV VS phi for PDXDigits contained in a ROI*/
    TH1F* m_h1ResidU; /**< distribution of U resid for PXDDigits contained in a ROI*/
    TH1F* m_h1ResidV; /**< distribution of V resid for PXDDigits contained in a ROI*/
    TH2F* m_h2ResidUV; /**< distribution of V resid for PXDDigits contained in a ROI*/
    TH1F* m_h1SigmaU; /**< distribution of sigmaU for PXDDigits contained in a ROI*/
    TH1F* m_h1SigmaV; /**< distribution of sigmaV for PXDDigits contained in a ROI*/
    TH1F* m_h1GlobalTime; /**< distribution of global time for PDXDigits contained in a ROI*/
    TH2F* m_h2Mapglob; /**<sensor perp,phi */
    TH2F* m_h2MaplocL1; /**< L1 ladder u,v */
    TH2F* m_h2MaplocL2; /**< L2 ladder u,v */

    //fill digits outside2 ROIs
    TH2F* m_h2sigmaUphi_out2; /**< distribution of sigmaU VS phi for PDXDigits not contained in a ROI*/
    TH2F* m_h2sigmaVphi_out2; /**< distribution of sigmaV VS phi for PDXDigits not contained in a ROI*/
    TH1F* m_h1ResidU_out2; /**< distribution of U resid for PXDDigits not contained in a ROI*/
    TH1F* m_h1ResidV_out2; /**< distribution of V resid for PXDDigits not contained in a ROI*/
    TH2F* m_h2ResidUV_out2; /**< distribution of V resid for PXDDigits not contained in a ROI*/
    TH1F* m_h1SigmaU_out2; /**< distribution of sigmaU for PXDDigits not contained in a ROI*/
    TH1F* m_h1SigmaV_out2; /**< distribution of sigmaV for PXDDigits not contained in a ROI*/
    TH1F* m_h1GlobalTime_out2; /**< distribution of global time for PDXDigits not contained in a ROI*/
    TH2F* m_h2Mapglob_out2; /**<sensor perp,phi */
    TH2F* m_h2MaplocL1_out2; /**< L1 ladder u,v */
    TH2F* m_h2MaplocL2_out2; /**< L2 ladder u,v */

    //fill digits outside3 ROIs
    TH2F* m_h2sigmaUphi_out3; /**< distribution of sigmaU VS phi for PDXDigits not contained in a ROI*/
    TH2F* m_h2sigmaVphi_out3; /**< distribution of sigmaV VS phi for PDXDigits not contained in a ROI*/
    TH1F* m_h1ResidU_out3; /**< distribution of U resid for PXDDigits not contained in a ROI*/
    TH1F* m_h1ResidV_out3; /**< distribution of V resid for PXDDigits not contained in a ROI*/
    TH2F* m_h2ResidUV_out3; /**< distribution of V resid for PXDDigits not contained in a ROI*/
    TH1F* m_h1SigmaU_out3; /**< distribution of sigmaU for PXDDigits not contained in a ROI*/
    TH1F* m_h1SigmaV_out3; /**< distribution of sigmaV for PXDDigits not contained in a ROI*/
    TH1F* m_h1GlobalTime_out3; /**< distribution of global time for PDXDigits not contained in a ROI*/
    TH2F* m_h2Mapglob_out3; /**<sensor perp,phi */
    TH2F* m_h2MaplocL1_out3; /**< L1 ladder u,v */
    TH2F* m_h2MaplocL2_out3; /**< L2 ladder u,v */

    //fill digits outside4 ROIs
    TH2F* m_h2sigmaUphi_out4; /**< distribution of sigmaU VS phi for PDXDigits not contained in a ROI*/
    TH2F* m_h2sigmaVphi_out4; /**< distribution of sigmaV VS phi for PDXDigits not contained in a ROI*/
    TH1F* m_h1SigmaU_out4; /**< distribution of sigmaU for PXDDigits not contained in a ROI*/
    TH1F* m_h1SigmaV_out4; /**< distribution of sigmaV for PXDDigits not contained in a ROI*/
    TH1F* m_h1GlobalTime_out4; /**< distribution of global time for PDXDigits not contained in a ROI*/
    TH2F* m_h2Mapglob_out4; /**<sensor perp,phi */
    TH2F* m_h2MaplocL1_out4; /**< L1 ladder u,v */
    TH2F* m_h2MaplocL2_out4; /**< L2 ladder u,v */

    //fill digits outside5 ROIs
    TH1F* m_h1GlobalTime_out5; /**< distribution of global time for PDXDigits not contained in a ROI*/
    TH2F* m_h2Mapglob_out5; /**<sensor perp,phi */
    TH2F* m_h2MaplocL1_out5; /**< L1 ladder u,v */
    TH2F* m_h2MaplocL2_out5; /**< L2 ladder u,v */

    //BOH
    //    TH2F* m_h2_VXDhitsPR_xy;
    //    TH2F* m_h2_VXDhitsPR_rz;

    //ROI stuff
    TH2F* m_h2ROIbottomLeft;
    TH2F* m_h2ROItopRight;
    TH2F* m_h2ROIuMinMax;
    TH2F* m_h2ROIvMinMax;
    TH1F* m_h1totROIs; /**< distribution of number of all ROIs*/
    TH1F* m_h1okROIs; /**< distribution of number of ROIs containin a PXDDigit*/
    TH1F* m_h1okROIfrac; /**< distribution of number of ROIsreduction factor*/
    TH1F* m_h1redFactor; /**< distribution of number of ROIsreduction factor*/
    TH1F* m_h1redFactor_L1; /**< distribution of number of ROIsreduction factor*/
    TH1F* m_h1redFactor_L2; /**< distribution of number of ROIsreduction factor*/
    TH1F* m_h1totArea; /**< distribution of Area of all ROIs*/
    TH1F* m_h1okArea; /**< distribution of Area of ROIs containing a PXDDigit*/

    TH1F* m_h1effPerTrack;



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
    double m_pTmc; /**< transverse momentum */
    double m_momXmc; /**< true p along X */
    double m_momYmc;/**< true p along Y */
    double m_momZmc; /**< true p along Z */
    double m_thetamc; /**< true theta*/
    double m_costhetamc; /**< true cos theta */
    double m_phimc;  /**< true phi */
    double m_lambdamc;  /**< true lambda = pi/2 - theta*/

    unsigned int Ntrack; /**< nuner of tracks with pxd digits */
    unsigned int NtrackHit; /**< nuner of tracks with hits */
    unsigned int n_notINtrack2; /**< nuner of tracks with no digits in ROI (correct vxdID) */
    unsigned int n_notINtrack3; /**< nuner of tracks with no digits in ROI (wrong vxdID) */
    unsigned int n_notINtrack4; /**< nuner of tracks with no ROI (intercept with correct vxdID) */
    unsigned int n_notINtrack5; /**< nuner of tracks with no ROI (intercept with wrong vxdID) */


    unsigned int n_rois;
    unsigned int n_intercepts;
    unsigned int n_tracks; /**< number of tracks */
    unsigned int n_tracksWithDigits; /**< number of tracks with digits */
    unsigned int n_tracksWithDigitsInROI; /**< number of tracks with digits in ROI */
    unsigned int n_pxdDigit; /**< number of pxd digits*/
    unsigned int n_pxdDigitInROI; /**< number of pxd digits in ROIs*/

    unsigned int n_notINdigit2; /**< number of lost digits: no hit, correct vxdID*/
    unsigned int n_notINdigit3;/**< number of lost digits: no hit, wrong vxdID*/
    unsigned int n_notINdigit4; /**< number of lost digits: no ROI, intercepts with correct vxdID*/
    unsigned int n_notINdigit5; /**< number of lost digits: no ROI, intercepts with wrong vxdID*/

    unsigned int npxdDigit[6]; /**< number of pxd digits in bins of pt*/
    unsigned int npxdDigitInROI[6]; /**< number of pxd digits inside ROI in bins of pt*/
    unsigned int nnotINdigit2[6]; /**< number of lost digits in bins of pt: no hit, correct vxdID*/
    unsigned int nnotINdigit3[6]; /**< number of lost digits in bins of pt: no hit, wrong vxdID*/
    unsigned int nnotINdigit4[6];  /**< number of lost digits in bins of pt: no ROI, intercepts with correct vxdID*/
    unsigned int nnotINdigit5[6];  /**< number of lost digits in bins of pt: no ROI, intercepts with wrong vxdID*/
    unsigned int TrackOneDigiIn[6];
    unsigned int nnotINtrack2[6];
    unsigned int nnotINtrack3[6];
    unsigned int nnotINtrack4[6];
    unsigned int nnotINtrack5[6];

  };

}

#endif
