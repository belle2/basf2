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
    TFile* m_rootFilePtr = NULL; /**< pointer at root file used for storing histograms */

  private:

    int m_nEvents = 0;

    SVDNoiseCalibrations m_NoiseCal;
    SVDPulseShapeCalibrations m_PulseShapeCal;

    StoreArray<SVDRecoDigit> m_svdRecos;
    StoreArray<SVDCluster> m_svdClusters;
    StoreArray<RecoTrack> m_recoTracks;
    StoreArray<Track> m_Tracks;
    StoreArray<TrackFitResult> m_tfr;

    int m_ntracks = 0;

    //TRACKS
    TH1F* m_nTracks = NULL; /**< number of tracks*/
    TH1F* m_Pvalue = NULL; /**< track p value*/
    TH1F* m_mom = NULL; /**< track momentum*/
    TH1F* m_nSVDhits = NULL; /**< track momentum*/

    //RECO
    SVDHistograms<TH1F>* h_recoCharge = NULL; /**< SVDRecoDigits Charge */
    SVDHistograms<TH1F>* h_recoEnergy = NULL; /**< SVDRecoDigits Energy */
    SVDHistograms<TH1F>* h_recoTime = NULL; /**< SVDRecoDigits Time */
    SVDHistograms<TH1F>* h_recoNoise = NULL; /**< SVDRecoDigits Noise */

    //CLUSTER
    SVDHistograms<TH1F>* h_clusterCharge = NULL; /**< SVDClusters Charge */
    SVDHistograms<TH1F>* h_clusterSize = NULL; /**< SVDClusters Size */
    SVDHistograms<TH1F>* h_clusterSNR = NULL; /**< SVDClusters SNR */
    SVDHistograms<TH1F>* h_clusterEnergy = NULL; /**< SVDClusters Energy */
    SVDHistograms<TH1F>* h_clusterTime = NULL; /**< SVDClusters Time */

    //CLUSTER RELATED TO TRACKS
    SVDHistograms<TH1F>* h_clusterTrkCharge = NULL; /**< SVDClusters Charge */
    SVDHistograms<TH1F>* h_clusterTrkSize = NULL; /**< SVDClusters Size */
    SVDHistograms<TH1F>* h_clusterTrkSNR = NULL; /**< SVDClusters SNR */
    SVDHistograms<TH1F>* h_clusterTrkEnergy = NULL; /**< SVDClusters Energy */
    SVDHistograms<TH1F>* h_clusterTrkTime = NULL; /**< SVDClusters Time */

  };
}

#endif /* SVDB4CommissioningPlotsModule_H_ */

