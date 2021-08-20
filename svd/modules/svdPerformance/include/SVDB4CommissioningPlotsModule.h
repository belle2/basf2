/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVD_B4COMMISSIONINGPLOTS_H_
#define SVD_B4COMMISSIONINGPLOTS_H_

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>

#include <svd/dataobjects/SVDHistograms.h>

#include <string>
#include <TFile.h>
#include <TH1F.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {

  /** The SVD B4CommissioningPlots Module
   *
   */

  class SVDB4CommissioningPlotsModule : public Module {

  public:

    /** constructor */
    SVDB4CommissioningPlotsModule();

    /** destructor */
    virtual ~SVDB4CommissioningPlotsModule() {};

    virtual void initialize() override; /**< check StoreArrays & create rootfile */
    virtual void beginRun() override; /**< create histograms */
    virtual void event() override; /**< fill histograms */
    virtual void endRun() override; /**< write histogrmas */

    std::string m_ShaperDigitName; /**< SVDSHaperDigit StoreArray name*/
    std::string m_RecoDigitName; /**< SVDRecoDigit StoreArray name*/
    std::string m_ClusterName; /**< SVDCluster StoreArray name*/
    std::string m_TrackFitResultName; /**< TrackFitResult name*/
    std::string m_TrackName; /**< Track StoreArray name*/

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

  private:

    int m_nEvents = 0; /**< number of events*/

    SVDNoiseCalibrations m_NoiseCal; /**< SVDNoise Calibrations db object*/
    SVDPulseShapeCalibrations m_PulseShapeCal; /**<SVDPulseShaper calibrations db object*/

    StoreArray<SVDRecoDigit> m_svdRecos; /**<SVDRecoDigit StoreArray*/
    StoreArray<SVDCluster> m_svdClusters; /**<SVDCluster StoreArray*/
    StoreArray<RecoTrack> m_recoTracks; /**<RecoTrack StoreArray*/
    StoreArray<Track> m_Tracks; /**<Track StoreArray*/
    StoreArray<TrackFitResult> m_tfr; /**<TrackFitResult StoreArray*/

    int m_ntracks = 0; /**< number of tracks*/

    //TRACKS
    TH1F* m_nTracks = nullptr; /**< number of tracks*/
    TH1F* m_Pvalue = nullptr; /**< track p value*/
    TH1F* m_mom = nullptr; /**< track momentum*/
    TH1F* m_nSVDhits = nullptr; /**< track momentum*/

    //RECO
    SVDHistograms<TH1F>* h_recoCharge = nullptr; /**< SVDRecoDigits Charge */
    SVDHistograms<TH1F>* h_recoEnergy = nullptr; /**< SVDRecoDigits Energy */
    SVDHistograms<TH1F>* h_recoTime = nullptr; /**< SVDRecoDigits Time */
    SVDHistograms<TH1F>* h_recoNoise = nullptr; /**< SVDRecoDigits Noise */

    //CLUSTER
    SVDHistograms<TH1F>* h_clusterCharge = nullptr; /**< SVDClusters Charge */
    SVDHistograms<TH1F>* h_clusterSize = nullptr; /**< SVDClusters Size */
    SVDHistograms<TH1F>* h_clusterSNR = nullptr; /**< SVDClusters SNR */
    SVDHistograms<TH1F>* h_clusterEnergy = nullptr; /**< SVDClusters Energy */
    SVDHistograms<TH1F>* h_clusterTime = nullptr; /**< SVDClusters Time */

    //CLUSTER RELATED TO TRACKS
    SVDHistograms<TH1F>* h_clusterTrkCharge = nullptr; /**< SVDClusters Charge */
    SVDHistograms<TH1F>* h_clusterTrkSize = nullptr; /**< SVDClusters Size */
    SVDHistograms<TH1F>* h_clusterTrkSNR = nullptr; /**< SVDClusters SNR */
    SVDHistograms<TH1F>* h_clusterTrkEnergy = nullptr; /**< SVDClusters Energy */
    SVDHistograms<TH1F>* h_clusterTrkTime = nullptr; /**< SVDClusters Time */
    SVDHistograms<TH1F>* h_clusterTrkInterstripPos = nullptr; /**< SVDClusters InterstripPosition */

  };
}

#endif /* SVDB4CommissioningPlotsModule_H_ */

