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

#include <tracking/dqmUtils/BaseDQMHistogramModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TH1F.h>
#include <TH2F.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>

using namespace std;

namespace Belle2 {

  /** DQM of Alignment for off line
    * residuals per sensor, layer,
    * keep also On-Line DQM from tracking:
    * their momentum,
    * Number of hits in tracks,
    * Number of tracks.
    *
    */
  class AlignDQMModule : public BaseDQMHistogramModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    AlignDQMModule();
    /* Destructor */
    ~AlignDQMModule();

    /** Module function initialize */
    void initialize() override final;
    /** Module function beginRun */
    //void beginRun() override final;
    /** Module function event */
    void event() override final;
    /** Module function endRun */
    void endRun() override final;

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    void defineHisto() override final;

    void FillTrackFitResult(const TrackFitResult* tfr) override;

    void FillSensorIndex(float ResidUPlaneRHUnBias, float ResidVPlaneRHUnBias, float posU, float posV, int sensorIndex);
    void FillLayers(float ResidUPlaneRHUnBias, float ResidVPlaneRHUnBias, float fPosSPU, float fPosSPV, int layerIndex);

  private:
    void DefineSensors() override;
    void DefineLayers();
    void DefineHelixParameters() override;

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

    /** helix parameters and their corellations: */

    /** Phi - the angle of the transverse momentum in the r-phi plane vs. z0 of the perigee (to see primary vertex shifts along R or z) */
    TH2F* m_PhiZ0 = nullptr;
    /** Phi - the angle of the transverse momentum in the r-phi plane vs. Track momentum Pt */
    TH2F* m_PhiMomPt = nullptr;
    /** Phi - the angle of the transverse momentum in the r-phi plane vs. Omega - the curvature of the track */
    TH2F* m_PhiOmega = nullptr;
    /** Phi - the angle of the transverse momentum in the r-phi plane vs. TanLambda - the slope of the track in the r-z plane */
    TH2F* m_PhiTanLambda = nullptr;
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
  };
}