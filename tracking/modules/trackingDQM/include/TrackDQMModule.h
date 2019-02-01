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

  /** DQM of tracks
    * their momentum,
    * Number of hits in tracks,
    * Number of tracks.
    *
    */
  class TrackDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    TrackDQMModule();
    /* Destructor */
    ~TrackDQMModule();

    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    void defineHisto() override final;

  private:

    /** StoreArray name where Tracks are written. */
    std::string m_TracksStoreArrayName;
    /** StoreArray name where RecoTracks are written. */
    std::string m_RecoTracksStoreArrayName;

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
    /** d0 - the signed distance to the IP in the r-phi plane */
    TH1F* m_D0 = nullptr;
    /** d0 vs Phi - the signed distance to the IP in the r-phi plane */
    TH2F* m_D0Phi = nullptr;
    /** z0 - the z0 coordinate of the perigee (beam spot position) */
    TH1F* m_Z0 = nullptr;
    /** z0 vs d0 - signed distance to the IP in r-phi vs. z0 of the perigee (to see primary vertex shifts along R or z) */
    TH2F* m_D0Z0 = nullptr;
    /** Phi - the angle of the transverse momentum in the r-phi plane, with CDF naming convention */
    TH1F* m_Phi = nullptr;
    /** TanLambda - the slope of the track in the r-z plane */
    TH1F* m_TanLambda = nullptr;
    /** Omega - the curvature of the track. It's sign is defined by the charge of the particle */
    TH1F* m_Omega = nullptr;


    /** Number of hits on PXD */
    TH1F* m_HitsPXD = nullptr;
    /** Number of hits on SVD */
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

  };  //end class declaration

}  // end namespace Belle2

