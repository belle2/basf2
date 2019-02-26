/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for track quality check                                       *
 * Prepared for Phase 2 and Belle II geometry                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>

#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /** DQM of Alignment for off line
    * residuals per sensor, layer,
    * keep also On-Line DQM from tracking:
    * their momentum,
    * Number of hits in tracks,
    * Number of tracks.
    *
    */
  class AlignDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    AlignDQMModule();
    /* Destructor */
    ~AlignDQMModule();

    /** Module function initialize */
    void initialize() override final;
    /** Module function beginRun */
    void beginRun() override final;
    /** Module function event */
    void event() override final;
    /** Module function endRun */
    void endRun() override final;

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    void defineHisto() override final;

  private:
    // Special Alignment related: Sensor level
    /** ResidaulMean vs U vs V counter for sensor*/
    TH2F** m_ResMeanPosUVSensCounts = nullptr;
    /** ResidaulMeanU vs U vs V for sensor*/
    TH2F** m_ResMeanUPosUVSens = nullptr;
    /** ResidaulMeanU vs U vs V for sensor*/
    TH2F** m_ResMeanVPosUVSens = nullptr;
    /** ResidaulU vs U for sensor*/
    TH2F** m_ResUPosUSens = nullptr;
    /** ResidaulU vs V for sensor*/
    TH2F** m_ResUPosVSens = nullptr;
    /** ResidaulV vs U for sensor*/
    TH2F** m_ResVPosUSens = nullptr;
    /** ResidaulV vs V for sensor*/
    TH2F** m_ResVPosVSens = nullptr;
    /** ResidaulMeanU vs U for sensor*/
    TH1F** m_ResMeanUPosUSens = nullptr;
    /** ResidaulMeanU vs V for sensor*/
    TH1F** m_ResMeanUPosVSens = nullptr;
    /** ResidaulMeanV vs U for sensor*/
    TH1F** m_ResMeanVPosUSens = nullptr;
    /** ResidaulMeanV vs V for sensor*/
    TH1F** m_ResMeanVPosVSens = nullptr;

    // Special Alignment related: Layer level
    /** ResidaulMean vs Phi vs Theta counter for Layer*/
    TH2F** m_ResMeanPhiThetaLayerCounts = nullptr;
    /** ResidaulMeanU vs Phi vs Theta for Layer*/
    TH2F** m_ResMeanUPhiThetaLayer = nullptr;
    /** ResidaulMeanU vs Phi vs Theta for Layer*/
    TH2F** m_ResMeanVPhiThetaLayer = nullptr;
    /** ResidaulU vs Phi for Layer*/
    TH2F** m_ResUPhiLayer = nullptr;
    /** ResidaulU vs Theta for Layer*/
    TH2F** m_ResUThetaLayer = nullptr;
    /** ResidaulV vs Phi for Layer*/
    TH2F** m_ResVPhiLayer = nullptr;
    /** ResidaulV vs Theta for Layer*/
    TH2F** m_ResVThetaLayer = nullptr;
    /** ResidaulMeanU vs Phi for Layer*/
    TH1F** m_ResMeanUPhiLayer = nullptr;
    /** ResidaulMeanU vs Theta for Layer*/
    TH1F** m_ResMeanUThetaLayer = nullptr;
    /** ResidaulMeanV vs Phi for Layer*/
    TH1F** m_ResMeanVPhiLayer = nullptr;
    /** ResidaulMeanV vs Theta for Layer*/
    TH1F** m_ResMeanVThetaLayer = nullptr;

    /** p Value */
    TH1F* m_PValue = nullptr;
    /** Chi2 */
    TH1F* m_Chi2 = nullptr;
    /** NDF */
    TH1F* m_NDF = nullptr;
    /** Chi2 / NDF */
    TH1F* m_Chi2NDF = nullptr;
    /** Unbiased residuals for PXD u vs v */
    TH2F* m_UBResidualsPXD = nullptr;
    /** Unbiased residuals for SVD u vs v */
    TH2F* m_UBResidualsSVD = nullptr;
    /** Unbiased residuals for PXD and SVD u vs v per sensor*/
    TH2F** m_UBResidualsSensor = nullptr;
    /** Unbiased residuals for PXD u */
    TH1F* m_UBResidualsPXDU = nullptr;
    /** Unbiased residuals for SVD u */
    TH1F* m_UBResidualsSVDU = nullptr;
    /** Unbiased residuals for PXD and SVD u per sensor*/
    TH1F** m_UBResidualsSensorU = nullptr;
    /** Unbiased residuals for PXD v */
    TH1F* m_UBResidualsPXDV = nullptr;
    /** Unbiased residuals for SVD v */
    TH1F* m_UBResidualsSVDV = nullptr;
    /** Unbiased residuals for PXD and SVD v per sensor*/
    TH1F** m_UBResidualsSensorV = nullptr;
    /** Track related clusters - hitmap in IP angle range */
    TH2F** m_TRClusterHitmap = nullptr;
    /** Track related clusters - neighbor corelations in Phi */
    TH2F** m_TRClusterCorrelationsPhi = nullptr;
    /** Track related clusters - neighbor corelations in Theta */
    TH2F** m_TRClusterCorrelationsTheta = nullptr;


    /** Track momentum Pt.Phi */
    TH1F* m_MomPhi = nullptr;
    /** Track momentum Pt.Theta */
    TH1F* m_MomTheta = nullptr;
    /** Track momentum Pt.CosTheta */
    TH1F* m_MomCosTheta = nullptr;
    /** Track momentum Pt.X */
    TH1F* m_MomX = nullptr;
    /** Track momentum Pt.Y */
    TH1F* m_MomY = nullptr;
    /** Track momentum Pt.Z */
    TH1F* m_MomZ = nullptr;
    /** Track momentum Pt */
    TH1F* m_MomPt = nullptr;
    /** Track momentum Magnitude */
    TH1F* m_Mom = nullptr;
    /** Number of hits on PXD */
    TH1F* m_HitsPXD = nullptr;
    /** Number of hits on VXD */
    TH1F* m_HitsSVD = nullptr;
    /** Number of hits on CDC */
    TH1F* m_HitsCDC = nullptr;
    /** Number of all hits in tracks */
    TH1F* m_Hits = nullptr;
    /** Number of tracks only with VXD */
    TH1F* m_TracksVXD = nullptr;
    /** Number of tracks only with CDC */
    TH1F* m_TracksCDC = nullptr;
    /** Number of full tracks with VXD+CDC */
    TH1F* m_TracksVXDCDC = nullptr;
    /** Number of all finding tracks */
    TH1F* m_Tracks = nullptr;

    /** helix parameters and their corellations: */

    /** d0 - the signed distance to the IP in the r-phi plane */
    TH1F* m_D0 = nullptr;
    /** z0 - the z0 coordinate of the perigee (beam spot position) */
    TH1F* m_Z0 = nullptr;
    /** Omega - the curvature of the track. It's sign is defined by the charge of the particle */
    TH1F* m_Omega = nullptr;
    /** TanLambda - the slope of the track in the r-z plane */
    TH1F* m_TanLambda = nullptr;

    /** d0 - signed distance to the IP in r-phi vs. z0 of the perigee (to see primary vertex shifts along R or z) */
    TH2F* m_D0Z0 = nullptr;
    /** d0 - signed distance to the IP in r-phi vs. Track momentum Pt */
    TH2F* m_D0MomPt = nullptr;
    /** d0 - signed distance to the IP in r-phi vs. Omega - the curvature of the track */
    TH2F* m_D0Omega = nullptr;
    /** d0 - signed distance to the IP in r-phi vs. TanLambda - the slope of the track in the r-z plane */
    TH2F* m_D0TanLambda = nullptr;
    /** z0 - the z0 coordinate of the perigee vs. Track momentum Pt */
    TH2F* m_Z0MomPt = nullptr;
    /** z0 - the z0 coordinate of the perigee vs. Omega - the curvature of the track */
    TH2F* m_Z0Omega = nullptr;
    /** z0 - the z0 coordinate of the perigee vs. TanLambda - the slope of the track in the r-z plane */
    TH2F* m_Z0TanLambda = nullptr;
    /** Track momentum Pt vs. Omega - the curvature of the track */
    TH2F* m_MomPtOmega = nullptr;
    /** Track momentum Pt vs. TanLambda - the slope of the track in the r-z plane */
    TH2F* m_MomPtTanLambda = nullptr;
    /** Omega - the curvature of the track vs. TanLambda - the slope of the track in the r-z plane */
    TH2F* m_OmegaTanLambda = nullptr;


    /// StoreArray name where the merged Tracks are written.
    std::string m_param_TracksStoreArrayName = "";

    /// StoreArray name where the merged RecoTracks are written.
    std::string m_param_RecoTracksStoreArrayName = "";
  };  //end class declaration

}  // end namespace Belle2

