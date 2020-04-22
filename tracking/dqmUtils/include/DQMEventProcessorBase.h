#pragma once

#include <tracking/dqmUtils/DQMHistoModuleBase.h>

#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/SensorInfoBase.h>

using namespace std;

namespace Belle2 {

  class DQMEventProcessorBase {

  public:
    DQMEventProcessorBase(DQMHistoModuleBase* histoModule, string recoTracksStoreArrayName, string tracksStoreArrayName)
    {
      m_histoModule = histoModule;
      m_recoTracksStoreArrayName = recoTracksStoreArrayName;
      m_tracksStoreArrayName = tracksStoreArrayName;
    }

    virtual void Run();

  protected:
    virtual void ProcessOneTrack(const Track& track);
    virtual TString ConstructMessage();
    virtual bool ProcessSuccessfulFit();
    virtual void ProcessOneRecoHit(RecoHitInformation* recoHitInfo);
    virtual void ProcessPXDRecoHit(RecoHitInformation* recoHitInfo);
    virtual void ProcessSVDRecoHit(RecoHitInformation* recoHitInfo);
    virtual void ComputeCommonVariables();
    virtual void FillCommonHistograms();
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