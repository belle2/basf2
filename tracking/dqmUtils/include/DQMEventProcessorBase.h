/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys, Jachym Bartik                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/DQMHistoModuleBase.h>

#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/SensorInfoBase.h>

using namespace std;

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
     * @param trackStoreArrayName - StoreArray name where the merged Tracks are written. */
    DQMEventProcessorBase(DQMHistoModuleBase* histoModule, string recoTracksStoreArrayName, string tracksStoreArrayName)
    {
      m_histoModule = histoModule;
      m_recoTracksStoreArrayName = recoTracksStoreArrayName;
      m_tracksStoreArrayName = tracksStoreArrayName;
    }

    /** Call this to start processing the event data and filling histograms.
    * Calls ProcessTrack function for each track in store array. */
    virtual void Run();

  protected:
    /** Find RecoTrack for given track. Calls ProcessSuccesfulFit if the RecoTrack has a successful fit. */
    virtual void ProcessTrack(const Track& track);
    /** Make debug message with information about RecoTrack. Used in ProcessTrack function. */
    virtual TString ConstructMessage();
    /** Continue track processing by calling ProcessRecoHit function on each RecoHitInformation in given RecoHit. */
    virtual bool ProcessSuccessfulFit();
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
    * Returns true if sensor with given ladderNumber and layerNumber isn't in the Yang half-shell, therefore it should be in the Ying half-shell if it's from PXD detector.
    * Returns false if the sensor is in the Yang.
    *
    * Possible combinations of parameters for Yang:
    *
    * | layerNumber | ladderNumber        |
    * | 1           | 5, 6, 7, 8          |
    * | 2           | 7, 8, 9, 10, 11, 12 |
    */
    static bool IsNotYang(int ladder, int layer);

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
    static bool IsNotMat(int ladder, int layer);

    DQMHistoModuleBase* m_histoModule;
    string m_tracksStoreArrayName;
    string m_recoTracksStoreArrayName;

    int m_nPXD;
    int m_nSVD;
    int m_nCDC;

    int m_iTrack;
    int m_iTrackVXD;
    int m_iTrackCDC;
    int m_iTrackVXDCDC;

    RecoTrack* m_recoTrack;
    const TrackFitResult* m_trackFitResult;

    bool m_isNotFirstHit = false;

    TVectorT<double>* m_resUnBias = nullptr;
    VxdID m_sensorID;
    VxdID m_sensorIDPrew;

    float m_residUPlaneRHUnBias;
    float m_residVPlaneRHUnBias;
    float m_fPosSPU;
    float m_fPosSPUPrev;
    float m_fPosSPV;
    float m_fPosSPVPrev;
    float m_posU;
    float m_posV;
    int m_layerNumber;
    int m_layerNumberPrev;
    int m_layerIndex;
    int m_correlationIndex;
    int m_sensorIndex;
    const VXD::SensorInfoBase* m_sensorInfo;
  };
}