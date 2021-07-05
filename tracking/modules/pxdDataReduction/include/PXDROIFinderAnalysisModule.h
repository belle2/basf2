/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/

#pragma once

#include <framework/core/Module.h>
#include <string>
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


  private:
    /**
     *Initializes the Module.
     */
    void initialize() override final;

    void beginRun() override final;

    void event() override final;

    /**
     * Termination action.
     */
    void terminate() override final;

    std::string m_recoTrackListName; /**< Track list name */
    std::string m_PXDInterceptListName; /**< Intercept list name */
    std::string m_ROIListName; /**< ROI list name */

    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing infos for debugging and validating purposes */
    std::string m_rootFileName; /**< root file name */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */

    int m_rootEvent;   /**<  event number*/

    //pxd sensor number
    Int_t m_nSensorsL1; /**<  number of sensors on L1*/
    Int_t m_nSensorsL2; /**<  number of sensors on L2*/

    //graphs & histos - results
    Double_t pt[6]  = {0.05, 0.15, 0.25, 0.4, 0.75, 1.5}; /**< bin edges (in pt = transverse momentum)*/
    Double_t ptErr[6] = { 0.05, 0.05, 0.05, 0.1, 0.25, 0.5}; /**< bin widths (transverse momentum) */

    TGraphErrors* m_gEff2 = nullptr; /**< efficiency graph */
    TGraphErrors* m_gEff = nullptr; /**< efficiency graph */

    TH1F* m_h1DigitsPerParticle = nullptr; /**< number of digits per particle */
    TH1F* m_h1RecoTracksPerParticle = nullptr; /**< number of RecoTracks per particle */

    TH1F* m_h1digiIn = nullptr; /**< digits contained in ROI histogram*/
    TH1F* m_h1digiOut2 = nullptr; /**< lost digit: ROI exist with right vxdID */
    TH1F* m_h1digiOut3 = nullptr; /**< lost digit: ROI exist with wrong vxdID */
    TH1F* m_h1digiOut4 = nullptr; /**< lost digit: ROI does not exist, intercept with right vxdID */
    TH1F* m_h1digiOut5 = nullptr; /**< lost digit: ROI does not exist, intercept with wrong vxdID */

    TH1F* m_h1TrackOneDigiIn = nullptr; /**<tracks with at least digit contained in ROI*/
    TH1F* m_h1nnotINtrack2 = nullptr; /**< tracks with lost digit: ROI exist with right vxdID */
    TH1F* m_h1nnotINtrack3 = nullptr; /**< lost digit: ROI exist with wrong vxdID */
    TH1F* m_h1nnotINtrack4 = nullptr; /**< lost digit: ROI does not exist, intercept with right vxdID */
    TH1F* m_h1nnotINtrack5 = nullptr; /**< lost digit: ROI does not exist, intercept with wrong vxdID */

    TH1F* m_h1Track = nullptr; /**< denominator track  */
    TH1F* m_h1Track_pt = nullptr; /**< denominator track pT*/
    TH1F* m_h1Track_phi = nullptr; /**< denominator track phi*/
    TH1F* m_h1Track_lambda = nullptr; /**< denominator track lambda*/
    TH1F* m_h1Track_cosTheta = nullptr; /**< denominator track cosTheta*/
    TH1F* m_h1Track_pVal = nullptr; /**< denominator track pVal*/
    TH1F* m_h1Track_nSVDhits = nullptr; /**< denominator track pVal*/
    TH1F* m_h1Track_nCDChits = nullptr; /**< denominator track pVal*/

    TH1F* m_h1INtrack1 = nullptr; /**< track with no intercept  */
    TH1F* m_h1INtrack1_pt = nullptr; /**<  track with no intercept pT*/
    TH1F* m_h1INtrack1_phi = nullptr; /**< track with no intercept phi*/
    TH1F* m_h1INtrack1_lambda = nullptr; /**< track with no intercept lambda*/
    TH1F* m_h1INtrack1_cosTheta = nullptr; /**< track with no intercept costheta*/
    TH1F* m_h1INtrack1_pVal = nullptr; /**< denominator track pVal*/
    TH1F* m_h1INtrack1_nSVDhits = nullptr; /**< denominator track pVal*/
    TH1F* m_h1INtrack1_nCDChits = nullptr; /**< denominator track pVal*/

    TH1F* m_h1notINtrack5 = nullptr; /**< track with no intercept  */
    TH1F* m_h1notINtrack5_pt = nullptr; /**<  track with no intercept pT*/
    TH1F* m_h1notINtrack5_phi = nullptr; /**< track with no intercept phi*/
    TH1F* m_h1notINtrack5_lambda = nullptr; /**< track with no intercept lambda*/
    TH1F* m_h1notINtrack5_cosTheta = nullptr; /**< track with no intercept costheta*/
    TH1F* m_h1notINtrack5_pVal = nullptr; /**< denominator track pVal*/
    TH1F* m_h1notINtrack5_nSVDhits = nullptr; /**< denominator track pVal*/
    TH1F* m_h1notINtrack5_nCDChits = nullptr; /**< denominator track pVal*/


    //fill digits inside ROIs
    TH1F* m_h1PullU = nullptr; /**< distribution of U pulls for PDXDigits contained in a ROI*/
    TH1F* m_h1PullV = nullptr; /**< distribution of V pulls for PDXDigits contained in a ROI*/
    TH2F* m_h2sigmaUphi = nullptr; /**< distribution of sigmaU VS phi for PDXDigits contained in a ROI*/
    TH2F* m_h2sigmaVphi = nullptr; /**< distribution of sigmaV VS phi for PDXDigits contained in a ROI*/
    TH1F* m_h1ResidU = nullptr; /**< distribution of U resid for PXDDigits contained in a ROI*/
    TH1F* m_h1ResidV = nullptr; /**< distribution of V resid for PXDDigits contained in a ROI*/
    TH2F* m_h2ResidUV = nullptr; /**< distribution of V resid for PXDDigits contained in a ROI*/
    TH1F* m_h1SigmaU = nullptr; /**< distribution of sigmaU for PXDDigits contained in a ROI*/
    TH1F* m_h1SigmaV = nullptr; /**< distribution of sigmaV for PXDDigits contained in a ROI*/
    TH1F* m_h1GlobalTime = nullptr; /**< distribution of global time for PDXDigits contained in a ROI*/
    TH2F* m_h2Mapglob = nullptr; /**<sensor perp,phi */
    TH2F* m_h2MaplocL1 = nullptr; /**< L1 ladder u,v */
    TH2F* m_h2MaplocL2 = nullptr; /**< L2 ladder u,v */

    //fill digits outside2 ROIs
    TH2F* m_h2sigmaUphi_out2 = nullptr; /**< distribution of sigmaU VS phi for PDXDigits not contained in a ROI*/
    TH2F* m_h2sigmaVphi_out2 = nullptr; /**< distribution of sigmaV VS phi for PDXDigits not contained in a ROI*/
    TH1F* m_h1ResidU_out2 = nullptr; /**< distribution of U resid for PXDDigits not contained in a ROI*/
    TH1F* m_h1ResidV_out2 = nullptr; /**< distribution of V resid for PXDDigits not contained in a ROI*/
    TH2F* m_h2ResidUV_out2 = nullptr; /**< distribution of V resid for PXDDigits not contained in a ROI*/
    TH1F* m_h1SigmaU_out2 = nullptr; /**< distribution of sigmaU for PXDDigits not contained in a ROI*/
    TH1F* m_h1SigmaV_out2 = nullptr; /**< distribution of sigmaV for PXDDigits not contained in a ROI*/
    TH1F* m_h1GlobalTime_out2 = nullptr; /**< distribution of global time for PDXDigits not contained in a ROI*/
    TH2F* m_h2Mapglob_out2 = nullptr; /**<sensor perp,phi */
    TH2F* m_h2MaplocL1_out2 = nullptr; /**< L1 ladder u,v */
    TH2F* m_h2MaplocL2_out2 = nullptr; /**< L2 ladder u,v */

    //fill digits outside3 ROIs
    TH2F* m_h2sigmaUphi_out3 = nullptr; /**< distribution of sigmaU VS phi for PDXDigits not contained in a ROI*/
    TH2F* m_h2sigmaVphi_out3 = nullptr; /**< distribution of sigmaV VS phi for PDXDigits not contained in a ROI*/
    TH1F* m_h1ResidU_out3 = nullptr; /**< distribution of U resid for PXDDigits not contained in a ROI*/
    TH1F* m_h1ResidV_out3 = nullptr; /**< distribution of V resid for PXDDigits not contained in a ROI*/
    TH2F* m_h2ResidUV_out3 = nullptr; /**< distribution of V resid for PXDDigits not contained in a ROI*/
    TH1F* m_h1SigmaU_out3 = nullptr; /**< distribution of sigmaU for PXDDigits not contained in a ROI*/
    TH1F* m_h1SigmaV_out3 = nullptr; /**< distribution of sigmaV for PXDDigits not contained in a ROI*/
    TH1F* m_h1GlobalTime_out3 = nullptr; /**< distribution of global time for PDXDigits not contained in a ROI*/
    TH2F* m_h2Mapglob_out3 = nullptr; /**<sensor perp,phi */
    TH2F* m_h2MaplocL1_out3 = nullptr; /**< L1 ladder u,v */
    TH2F* m_h2MaplocL2_out3 = nullptr; /**< L2 ladder u,v */

    //fill digits outside4 ROIs
    TH2F* m_h2sigmaUphi_out4 = nullptr; /**< distribution of sigmaU VS phi for PDXDigits not contained in a ROI*/
    TH2F* m_h2sigmaVphi_out4 = nullptr; /**< distribution of sigmaV VS phi for PDXDigits not contained in a ROI*/
    TH1F* m_h1SigmaU_out4 = nullptr; /**< distribution of sigmaU for PXDDigits not contained in a ROI*/
    TH1F* m_h1SigmaV_out4 = nullptr; /**< distribution of sigmaV for PXDDigits not contained in a ROI*/
    TH1F* m_h1GlobalTime_out4 = nullptr; /**< distribution of global time for PDXDigits not contained in a ROI*/
    TH2F* m_h2Mapglob_out4 = nullptr; /**<sensor perp,phi */
    TH2F* m_h2MaplocL1_out4 = nullptr; /**< L1 ladder u,v */
    TH2F* m_h2MaplocL2_out4 = nullptr; /**< L2 ladder u,v */

    //fill digits outside5 ROIs
    TH1F* m_h1GlobalTime_out5 = nullptr; /**< distribution of global time for PDXDigits not contained in a ROI*/
    TH2F* m_h2Mapglob_out5 = nullptr; /**<sensor perp,phi */
    TH2F* m_h2MaplocL1_out5 = nullptr; /**< L1 ladder u,v */
    TH2F* m_h2MaplocL2_out5 = nullptr; /**< L2 ladder u,v */

    //ROI stuff
    TH2F* m_h2ROIbottomLeft = nullptr; /**< u,v coordinates of the bottom left pixel */
    TH2F* m_h2ROItopRight = nullptr; /**< u,v coordinates of the top right pixel */
    TH2F* m_h2ROIuMinMax = nullptr; /**< u-coordinate Min vs Max */
    TH2F* m_h2ROIvMinMax = nullptr; /**< v-coordinate Min vs Max */
    TH1F* m_h1totROIs = nullptr; /**< distribution of number of all ROIs*/
    TH1F* m_h1okROIs = nullptr; /**< distribution of number of ROIs containin a PXDDigit*/
    TH1F* m_h1okROIfrac = nullptr; /**< distribution of number of ROIsreduction factor*/
    TH1F* m_h1redFactor = nullptr; /**< distribution of number of ROIsreduction factor*/
    TH1F* m_h1redFactor_L1 = nullptr; /**< distribution of number of ROIsreduction factor*/
    TH1F* m_h1redFactor_L2 = nullptr; /**< distribution of number of ROIsreduction factor*/
    TH1F* m_h1totArea = nullptr; /**< distribution of Area of all ROIs*/
    TH1F* m_h1okArea = nullptr; /**< distribution of Area of ROIs containing a PXDDigit*/

    TH1F* m_h1effPerTrack = nullptr; /**< fraction of digits in ROI per track*/



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

    unsigned int Ntrack; /**< number of tracks with pxd digits */
    unsigned int NtrackHit; /**< number of tracks with hits */
    unsigned int n_notINtrack2; /**< number of tracks with no digits in ROI (correct vxdID) */
    unsigned int n_notINtrack3; /**< number of tracks with no digits in ROI (wrong vxdID) */
    unsigned int n_notINtrack4; /**< number of tracks with no ROI (intercept with correct vxdID) */
    unsigned int n_notINtrack5; /**< number of tracks with no ROI (intercept with wrong vxdID) */


    unsigned int n_rois; /**< number of ROIs*/
    unsigned int n_intercepts; /**< number of PXDIntercepts*/
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
    unsigned int TrackOneDigiIn[6]; /**< number of tracks with one digit inside ROI in bins of pt*/
    unsigned int nnotINtrack2[6]; /**< number of tracks in bins of pt: no hit, correct vxdID*/
    unsigned int nnotINtrack3[6]; /**< number of tracks in bins of pt: no hit, wrong vxdID*/
    unsigned int nnotINtrack4[6]; /**< number of tracks in bins of pt: no ROI, intercepts with correct vxdID*/
    unsigned int nnotINtrack5[6]; /**< number of tracks in bins of pt: no ROI, intercepts with wrong vxdID*/

  };

}
