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

    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;
    void endRun() override final;

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    void defineHisto() override final;

  private:
    // Special Alignment related: Sensor level
    /** ResidaulMean vs U vs V counter for sensor*/
    TH2F** m_ResMeanPosUVSensCounts;
    /** ResidaulMeanU vs U vs V for sensor*/
    TH2F** m_ResMeanUPosUVSens;
    /** ResidaulMeanU vs U vs V for sensor*/
    TH2F** m_ResMeanVPosUVSens;
    /** ResidaulU vs U for sensor*/
    TH2F** m_ResUPosUSens;
    /** ResidaulU vs V for sensor*/
    TH2F** m_ResUPosVSens;
    /** ResidaulV vs U for sensor*/
    TH2F** m_ResVPosUSens;
    /** ResidaulV vs V for sensor*/
    TH2F** m_ResVPosVSens;
    /** ResidaulMeanU vs U for sensor*/
    TH1F** m_ResMeanUPosUSens;
    /** ResidaulMeanU vs V for sensor*/
    TH1F** m_ResMeanUPosVSens;
    /** ResidaulMeanV vs U for sensor*/
    TH1F** m_ResMeanVPosUSens;
    /** ResidaulMeanV vs V for sensor*/
    TH1F** m_ResMeanVPosVSens;

    // Special Alignment related: Layer level
    /** ResidaulMean vs Phi vs Theta counter for Layer*/
    TH2F** m_ResMeanPhiThetaLayerCounts;
    /** ResidaulMeanU vs Phi vs Theta for Layer*/
    TH2F** m_ResMeanUPhiThetaLayer;
    /** ResidaulMeanU vs Phi vs Theta for Layer*/
    TH2F** m_ResMeanVPhiThetaLayer;
    /** ResidaulU vs Phi for Layer*/
    TH2F** m_ResUPhiLayer;
    /** ResidaulU vs Theta for Layer*/
    TH2F** m_ResUThetaLayer;
    /** ResidaulV vs Phi for Layer*/
    TH2F** m_ResVPhiLayer;
    /** ResidaulV vs Theta for Layer*/
    TH2F** m_ResVThetaLayer;
    /** ResidaulMeanU vs Phi for Layer*/
    TH1F** m_ResMeanUPhiLayer;
    /** ResidaulMeanU vs Theta for Layer*/
    TH1F** m_ResMeanUThetaLayer;
    /** ResidaulMeanV vs Phi for Layer*/
    TH1F** m_ResMeanVPhiLayer;
    /** ResidaulMeanV vs Theta for Layer*/
    TH1F** m_ResMeanVThetaLayer;

    /** p Value */
    TH1F* m_PValue;
    /** Chi2 */
    TH1F* m_Chi2;
    /** NDF */
    TH1F* m_NDF;
    /** Chi2 / NDF */
    TH1F* m_Chi2NDF;
    /** Unbiased residuals for PXD u vs v */
    TH2F* m_UBResidualsPXD;
    /** Unbiased residuals for SVD u vs v */
    TH2F* m_UBResidualsSVD;
    /** Unbiased residuals for PXD and SVD u vs v per sensor*/
    TH2F** m_UBResidualsSensor;
    /** Unbiased residuals for PXD u */
    TH1F* m_UBResidualsPXDU;
    /** Unbiased residuals for SVD u */
    TH1F* m_UBResidualsSVDU;
    /** Unbiased residuals for PXD and SVD u per sensor*/
    TH1F** m_UBResidualsSensorU;
    /** Unbiased residuals for PXD v */
    TH1F* m_UBResidualsPXDV;
    /** Unbiased residuals for SVD v */
    TH1F* m_UBResidualsSVDV;
    /** Unbiased residuals for PXD and SVD v per sensor*/
    TH1F** m_UBResidualsSensorV;
    /** Track related clusters - hitmap in IP angle range */
    TH2F** m_TRClusterHitmap;
    /** Track related clusters - neighbor corelations in Phi */
    TH2F** m_TRClusterCorrelationsPhi;
    /** Track related clusters - neighbor corelations in Theta */
    TH2F** m_TRClusterCorrelationsTheta;


    /** Track momentum Pt.Phi */
    TH1F* m_MomPhi;
    /** Track momentum Pt.Theta */
    TH1F* m_MomTheta;
    /** Track momentum Pt.CosTheta */
    TH1F* m_MomCosTheta;
    /** Track momentum Pt.X */
    TH1F* m_MomX;
    /** Track momentum Pt.Y */
    TH1F* m_MomY;
    /** Track momentum Pt.Z */
    TH1F* m_MomZ;
    /** Track momentum Pt */
    TH1F* m_MomPt;
    /** Track momentum Magnitude */
    TH1F* m_Mom;
    /** Number of hits on PXD */
    TH1F* m_HitsPXD;
    /** Number of hits on VXD */
    TH1F* m_HitsSVD;
    /** Number of hits on CDC */
    TH1F* m_HitsCDC;
    /** Number of all hits in tracks */
    TH1F* m_Hits;
    /** Number of tracks only with VXD */
    TH1F* m_TracksVXD;
    /** Number of tracks only with CDC */
    TH1F* m_TracksCDC;
    /** Number of full tracks with VXD+CDC */
    TH1F* m_TracksVXDCDC;
    /** Number of all finding tracks */
    TH1F* m_Tracks;

    /// StoreArray name where the merged Tracks are written.
    std::string m_param_TracksStoreArrayName = "";

    /// StoreArray name where the merged RecoTracks are written.
    std::string m_param_RecoTracksStoreArrayName = "";
  };  //end class declaration

}  // end namespace Belle2

