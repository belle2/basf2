/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/DQMHistoModuleBase.h>

#include <tracking/dataobjects/RecoHitInformation.h>

namespace Belle2 {
  /**
   * The purpose of this class is to process one event() in DQMHistoModuleBase, which is a base for TrackDQMModule and AlignDQMModule.
   * This class is a base for TrackDQMEventProcessor and AlignDQMEventProcessor.
   *
   * After instance of this class is created via constructor its only public function Run() should be called to process the event.
   *
   * This class doesn't actually fill the histograms but it calls Fill- functions on given DQMHistoModuleBase instead.
   *
   * All functions of this class are supposed to be virtual so they can be overridden in derived classes. */
  class DQMEventProcessorBase {

  public:
    /** Constructor.
     * @param histoModule - DQMHistoModuleBase or derived module on which the Fill- functions are called.
     * @param recoTracksStoreArrayName - StoreArray name where the merged RecoTracks are written.
     * @param tracksStoreArrayName - StoreArray name where the merged Tracks are written. */
    DQMEventProcessorBase(DQMHistoModuleBase* histoModule, const std::string& recoTracksStoreArrayName,
                          const std::string& tracksStoreArrayName, bool runningOnHLT = false) :
      m_tracksStoreArrayName(tracksStoreArrayName),
      m_recoTracksStoreArrayName(recoTracksStoreArrayName),
      m_runningOnHLT(runningOnHLT)
    {
      m_histoModule = histoModule;
    }

    /** Call this to start processing the event data and filling histograms.
    * Calls ProcessTrack function for each track in store array. */
    virtual void Run();

  protected:
    /** Find RecoTrack for given track. Calls ProcessSuccesfulFit if the RecoTrack has a successful fit. */
    virtual void ProcessTrack(const Track& track);
    /** Make debug message with information about RecoTrack. Used in ProcessTrack function. */
    virtual TString ConstructMessage(const TrackFitResult* trackFitResult, int nPXDClusters, int nSVDClusters, int nCDCHits);
    /** Fill histograms with values derived from TrackFitResult. */
    virtual void FillTrackFitResult(const TrackFitResult* trackFitResult);
    /** Continue track processing by calling ProcessRecoHit function on each RecoHitInformation in given RecoHit. */
    virtual void ProcessSuccessfulFit();
    /** Compute unbiased residual and the calls ProcesPXDRecoHit or ProcessSVDRecoHit. */
    virtual void ProcessRecoHit(RecoHitInformation* recoHitInfo);
    /** Compute position in a PXD way. Then compute some other variables and fill some histograms. */
    virtual void ProcessPXDRecoHit(RecoHitInformation* recoHitInfo);
    /** Compute position in a SVD way which means we need two consecutive hits to be from the same sensor to get both u and v coordinates.
    * Then, if this condition is met and we have complete information about position, we can continue in a similar way as the ProcessPXDRecoHit function does. */
    virtual void ProcessSVDRecoHit(RecoHitInformation* recoHitInfo);
    /** Compute variables which are common for PXD and SVD hit. */
    virtual void ComputeCommonVariables();
    /** Fill histograms which are common for PXD and SVD hit. */
    virtual void FillCommonHistograms();
    /** Set the value of -Prev values which are common for PXD and SVD hit. */
    virtual void SetCommonPrevVariables();

    /**
    * Returns true if sensor with given ladderNumber and layerNumber isn't in the Yang half-shell, therefore it should be in the Yin half-shell if it's from PXD detector.
    * Returns false if the sensor is in the Yang.
    *
    * Possible combinations of parameters for Yang:
    *
    * | layerNumber | ladderNumber        |
    * | 1           | 5, 6, 7, 8          |
    * | 2           | 7, 8, 9, 10, 11, 12 |
    */
    static bool IsNotYang(int ladderNumber, int layerNumber);

    /**
    * Returns true if sensor with given ladderNumber and layerNumber isn't in the Mat half-shell, therefore it should be in the Pat half-shell if it's from SVD detector.
    * Returns false if the sensor is int the Mat.
    *
    * Possible combinations of parameters for Mat:
    *
    * | layerNumber | ladderNumber               |
    * | 3           | 3, 4, 5                    |
    * | 4           | 4, 5, 6, 7, 8              |
    * | 5           | 5, 6, 7, 8, 9, 10          |
    * | 6           | 6, 7, 8, 9, 10, 11, 12, 13 |
    */
    static bool IsNotMat(int ladderNumber, int layerNumber);

    /** DQM histogram module on which the Fill- functions are called to fill histograms. */
    DQMHistoModuleBase* m_histoModule = nullptr;
    /** StoreArray name where Tracks are written. */
    std::string m_tracksStoreArrayName = "";
    /** StoreArray name where RecoTracks are written. */
    std::string m_recoTracksStoreArrayName = "";

    bool m_runningOnHLT;

    /** index of track (with valid TrackFitResult and related RecoTrack) */
    int m_iTrack = 0;
    /** index of track where are VXD hits and aren't CDC hits (with valid TrackFitResult and related RecoTrack) */
    int m_iTrackVXD = 0;
    /** index of track where are CDC hits and aren't VXD hits (with valid TrackFitResult and related RecoTrack) */
    int m_iTrackCDC = 0;
    /** index of track where are both VXD hits and CDC hits (with valid TrackFitResult and related RecoTrack) */
    int m_iTrackVXDCDC = 0;

    /** RecoTrack related to currently processed Track */
    RecoTrack* m_recoTrack = nullptr;
    /** Determines if the hit is not the first hit in the current track */
    bool m_isNotFirstHit = false;

    /** unbiased residual for the hit obtained from the sensor so its length is different for PXD and SVD sensors */
    TVectorT<double>* m_rawSensorResidual = nullptr;
    /** ID of the current sensor */
    VxdID m_sensorID = VxdID(0);
    /** ID of the prewious sensor*/
    VxdID m_sensorIDPrev = VxdID(0);

    /** local coordinates of the hit position (u, v, w) */
    TVector3 m_position = TVector3();
    /** unbiased residual for the hit in micrometers in local coordinates (u, v, w) */
    TVector3 m_residual_um = TVector3();
    /** unbiased residual for the hit in micrometers in global coordinates (x, y, z) */
    TVector3 m_globalResidual_um = TVector3();
    /** global phi in degrees of the hit */
    float m_phi_deg = .0;
    /** global phi in degrees of the previous hit*/
    float m_phiPrev_deg = .0;
    /** global theta in degrees of the hit */
    float m_theta_deg = .0;
    /** global theta in degrees of the previous hit*/
    float m_thetaPrev_deg = .0;
    /** number of the layer of the hit */
    int m_layerNumber = 0;
    /** number of the layer of the previous hit */
    int m_layerNumberPrev = 0;
    /** index of the layer of the hit */
    int m_layerIndex = 0;
    /** index of the layer of the previous hit */
    int m_correlationIndex = 0;
    /** index of the sensor of the hit */
    int m_sensorIndex = 0;
  };
}
