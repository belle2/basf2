/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_DATA_REDUCTION_ANALYSIS_MODULE_H_
#define SVD_DATA_REDUCTION_ANALYSIS_MODULE_H_

#include <framework/core/Module.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/SVDIntercept.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraphErrors.h>

namespace Belle2 {

  /** The SVD Data Reduction AnalysisData Module
   *
   * this module performs the analysis of the SVD data reduction module performances
   *
   */

  class SVDROIFinderAnalysisDataModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    SVDROIFinderAnalysisDataModule();

    /**
     * Destructor of the module.
     */
    virtual ~SVDROIFinderAnalysisDataModule();

    /**
     *Initializes the Module.
     */
    void initialize() override;

    void event() override;

    /**
     * Termination action.
     */
    void terminate() override;

  private:

    StoreArray<SVDShaperDigit> m_shapers;
    StoreArray<ROIid> m_ROIs;
    StoreArray<RecoTrack> m_trackList;
    StoreArray<SVDIntercept> m_SVDIntercepts;

    bool m_isSimulation;


    std::string m_shapersName; /**< SVDShaperDigits name */
    std::string m_recoTrackListName; /**< Track list name */
    std::string m_SVDInterceptListName; /**< Intercept list name */
    std::string m_ROIListName; /**< ROI list name */

    TFile* m_rootFilePtr; /**< pointer at root file used for storing infos for debugging and validating purposes */
    std::string m_rootFileName; /**< root file name */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */

    int m_rootEvent;   /**<  event number*/

    //graphs & histos - results
    Double_t pt[6]  = {0.05, 0.15, 0.25, 0.4, 0.75, 1.5}; /**< bin edges (in pt = transverse momentum)*/
    Double_t ptErr[6] = { 0.05, 0.05, 0.05, 0.1, 0.25, 0.5}; /**< bin widths (transverse momentum) */


    TH1F* m_h1digiIn; /**< digits contained in ROI histogram*/
    TH1F* m_h1digiOut; /**< digits contained in ROI histogram*/

    TH1F* m_h1Track; /**< denominator track  */
    TH1F* m_h1Track_pt; /**< denominator track pT*/
    TH1F* m_h1Track_phi; /**< denominator track phi*/
    TH1F* m_h1Track_lambda; /**< denominator track lambda*/
    TH1F* m_h1Track_cosTheta; /**< denominator track cosTheta*/
    TH1F* m_h1Track_pVal; /**< denominator track pVal*/
    TH1F* m_h1Track_nSVDhits; /**< denominator track pVal*/
    TH1F* m_h1Track_nCDChits; /**< denominator track pVal*/

    //Tracks with attached ROI
    TH1F* m_h1ROItrack; /**< track with attached ROI  */
    TH1F* m_h1ROItrack_pt; /**< track with attached ROI - pT*/
    TH1F* m_h1ROItrack_phi; /**< track with attached ROI- phi*/
    TH1F* m_h1ROItrack_lambda; /**< track with attached ROI - lambda*/
    TH1F* m_h1ROItrack_cosTheta; /**< track with attached ROI - costheta*/
    TH1F* m_h1ROItrack_pVal; /**< track with attached ROI - pVal*/
    TH1F* m_h1ROItrack_nSVDhits; /**< track with attached ROI - n SVD hits*/
    TH1F* m_h1ROItrack_nCDChits; /**< track with attached ROI - n SVD hits*/

    //Tracks with attached Good ROI
    TH1F* m_h1GoodROItrack; /**< track with attached ROI  */
    TH1F* m_h1GoodROItrack_pt; /**< track with attached ROI - pT*/
    TH1F* m_h1GoodROItrack_phi; /**< track with attached ROI- phi*/
    TH1F* m_h1GoodROItrack_lambda; /**< track with attached ROI - lambda*/
    TH1F* m_h1GoodROItrack_cosTheta; /**< track with attached ROI - costheta*/
    TH1F* m_h1GoodROItrack_pVal; /**< track with attached ROI - pVal*/
    TH1F* m_h1GoodROItrack_nSVDhits; /**< track with attached ROI - n SVD hits*/
    TH1F* m_h1GoodROItrack_nCDChits; /**< track with attached ROI - n SVD hits*/

    //Tracks with attached ROI containing at least one SVDShaperDigit
    TH1F* m_h1FullROItrack; /**< track with attached ROI  */
    TH1F* m_h1FullROItrack_pt; /**< track with attached ROI - pT*/
    TH1F* m_h1FullROItrack_phi; /**< track with attached ROI- phi*/
    TH1F* m_h1FullROItrack_lambda; /**< track with attached ROI - lambda*/
    TH1F* m_h1FullROItrack_cosTheta; /**< track with attached ROI - costheta*/
    TH1F* m_h1FullROItrack_pVal; /**< track with attached ROI - pVal*/
    TH1F* m_h1FullROItrack_nSVDhits; /**< track with attached ROI - n SVD hits*/
    TH1F* m_h1FullROItrack_nCDChits; /**< track with attached ROI - n SVD hits*/


    //fill digits inside ROIs
    TH1F* m_h1PullU; /**< distribution of U pulls for PDXDigits contained in a ROI*/
    TH1F* m_h1PullV; /**< distribution of V pulls for PDXDigits contained in a ROI*/
    TH2F* m_h2sigmaUphi; /**< distribution of sigmaU VS phi for PDXDigits contained in a ROI*/
    TH2F* m_h2sigmaVphi; /**< distribution of sigmaV VS phi for PDXDigits contained in a ROI*/
    TH1F* m_h1ResidU; /**< distribution of U resid for SVDShaperDigits contained in a ROI*/
    TH1F* m_h1ResidV; /**< distribution of V resid for SVDShaperDigits contained in a ROI*/
    TH1F* m_h1SigmaU; /**< distribution of sigmaU for SVDShaperDigits contained in a ROI*/
    TH1F* m_h1SigmaV; /**< distribution of sigmaV for SVDShaperDigits contained in a ROI*/
    TH1F* m_h1GlobalTime; /**< distribution of global time for PDXDigits contained in a ROI*/

    //fill digits outside2 ROIs
    TH2F* m_h2sigmaUphi_out; /**< distribution of sigmaU VS phi for PDXDigits not contained in a ROI*/
    TH2F* m_h2sigmaVphi_out; /**< distribution of sigmaV VS phi for PDXDigits not contained in a ROI*/
    TH1F* m_h1ResidU_out; /**< distribution of U resid for SVDShaperDigits not contained in a ROI*/
    TH1F* m_h1ResidV_out; /**< distribution of V resid for SVDShaperDigits not contained in a ROI*/
    TH1F* m_h1SigmaU_out; /**< distribution of sigmaU for SVDShaperDigits not contained in a ROI*/
    TH1F* m_h1SigmaV_out; /**< distribution of sigmaV for SVDShaperDigits not contained in a ROI*/
    TH1F* m_h1GlobalTime_out; /**< distribution of global time for PDXDigits not contained in a ROI*/

    //ROI stuff
    TH2F* m_h2ROIuMinMax;
    TH2F* m_h2ROIvMinMax;
    TH1F* m_h1totROIs; /**< distribution of number of all ROIs*/
    TH1F* m_h1goodROIs; /**< distribution of number of ROIs containin a SVDShaperDigit, DATA*/
    TH1F* m_h1okROIs; /**< distribution of number of ROIs containin a SVDShaperDigit*/
    TH1F* m_h1effROIs; /**< distribution of number of ROIs containin a SVDShaperDigit, DATA*/
    TH1F* m_h1totUstrips; /**< distribution of #u strips of all ROIs*/
    TH1F* m_h1totVstrips; /**< distribution of #u strips of all ROIs*/

    //variables
    double m_globalTime; /**< global hit time */
    double m_coorU; /**< intercept U coordinate*/
    double m_coorV; /**< intercept V coordinate*/
    double m_sigmaU; /**< intercept U stat error*/
    double m_sigmaV; /**< intercept V stat error*/
    int m_vxdID; /**< VXD ID*/


    unsigned int n_rois;
    unsigned int m_nGoodROIs;
    unsigned int m_nOkROIs;
    unsigned int n_intercepts;
    unsigned int n_tracks; /**< number of tracks */

  };

}

#endif
