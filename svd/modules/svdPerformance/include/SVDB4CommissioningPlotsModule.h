/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Giulia Casarosa                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_B4COMMISSIONINGPLOTS_H_
#define SVD_B4COMMISSIONINGPLOTS_H_

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/datastore/StoreArray.h>

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>

#include <svd/dataobjects/SVDHistograms.h>

#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>
#include <TCollection.h>
#include <TList.h>
#include <TH3F.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {

  /** The SVD B4CommissioningPlots Module
   *
   */

  class SVDB4CommissioningPlotsModule : public Module {

  public:

    SVDB4CommissioningPlotsModule();

    virtual ~SVDB4CommissioningPlotsModule();
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    std::string m_ShaperDigitName; /**< */
    std::string m_RecoDigitName; /**< */
    std::string m_ClusterName; /**< */
    std::string m_TrackFitResultName; /**< */
    std::string m_TrackName; /**< */

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

  private:

    int m_nEvents;

    SVDNoiseCalibrations m_NoiseCal;
    SVDPulseShapeCalibrations m_PulseShapeCal;

    StoreArray<SVDRecoDigit> m_svdRecos;
    StoreArray<SVDCluster> m_svdClusters;
    StoreArray<RecoTrack> m_recoTracks;
    StoreArray<Track> m_Tracks;
    StoreArray<TrackFitResult> m_tfr;

    int m_ntracks;

    //TRACKS
    TH1F* m_nTracks; /**< number of tracks*/
    TH1F* m_Pvalue; /**< track p value*/
    TH1F* m_mom; /**< track momentum*/
    TH1F* m_nSVDhits; /**< track momentum*/

    //RECO
    SVDHistograms<TH1F>* h_recoCharge; /**< SVDRecoDigits Charge */
    SVDHistograms<TH1F>* h_recoEnergy; /**< SVDRecoDigits Energy */
    SVDHistograms<TH1F>* h_recoTime; /**< SVDRecoDigits Time */
    SVDHistograms<TH1F>* h_recoNoise; /**< SVDRecoDigits Noise */

    //CLUSTER
    SVDHistograms<TH1F>* h_clusterCharge; /**< SVDClusters Charge */
    SVDHistograms<TH1F>* h_clusterSize; /**< SVDClusters Size */
    SVDHistograms<TH1F>* h_clusterSNR; /**< SVDClusters SNR */
    SVDHistograms<TH1F>* h_clusterEnergy; /**< SVDClusters Energy */
    SVDHistograms<TH1F>* h_clusterTime; /**< SVDClusters Time */

    //CLUSTER RELATED TO TRACKS
    SVDHistograms<TH1F>* h_clusterTrkCharge; /**< SVDClusters Charge */
    SVDHistograms<TH1F>* h_clusterTrkSize; /**< SVDClusters Size */
    SVDHistograms<TH1F>* h_clusterTrkSNR; /**< SVDClusters SNR */
    SVDHistograms<TH1F>* h_clusterTrkEnergy; /**< SVDClusters Energy */
    SVDHistograms<TH1F>* h_clusterTrkTime; /**< SVDClusters Time */

  };
}

#endif /* SVDB4CommissioningPlotsModule_H_ */

