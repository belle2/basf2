#pragma once

#include <framework/core/HistoModule.h>
#include <tracking/dqmUtils/BaseDQMHistogramModule.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/SensorInfoBase.h>

using namespace std;

namespace Belle2 {
  class BaseDQMEventProcessor {
  public:
    BaseDQMEventProcessor(BaseDQMHistogramModule* p_histoModule, std::string p_m_RecoTracksStoreArrayName,
                          std::string p_m_TracksStoreArrayName)
    {
      histoModule = p_histoModule;
      m_RecoTracksStoreArrayName = p_m_RecoTracksStoreArrayName;
      m_TracksStoreArrayName = p_m_TracksStoreArrayName;
    }

    virtual void Run();

  protected:
    BaseDQMHistogramModule* histoModule;
    string m_TracksStoreArrayName;
    string m_RecoTracksStoreArrayName;

    virtual void ProcessOneTrack(const Track& track);

    virtual TString ConstructMessage() = 0;

    int nPXD;
    int nSVD;
    int nCDC;

    int iTrack;
    int iTrackVXD;
    int iTrackCDC;
    int iTrackVXDCDC;

    RecoTrack* recoTrack;
    const TrackFitResult* trackFitResult;

    virtual bool ProcessSuccessfulFit();

    bool isNotFirstHit = false;

    VxdID sensorIDPrew;

    float ResidUPlaneRHUnBias;
    float ResidVPlaneRHUnBias;
    float fPosSPUPrev;
    float fPosSPVPrev;
    float fPosSPU;
    float fPosSPV;
    float posU;
    float posV;
    int layerNumberPrev;
    int layerNumber;
    int layerIndex;
    int correlationIndex;
    int sensorIndex;

    virtual void ProcessOneRecoHit(RecoHitInformation* recoHitInfo);

    TVectorT<double>* resUnBias = nullptr;
    VxdID sensorID;

    virtual void ProcessPXDRecoHit(RecoHitInformation* recoHitInfo);
    virtual void ProcessSVDRecoHit(RecoHitInformation* recoHitInfo);

    const VXD::SensorInfoBase* sensorInfo;

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
    bool IsNotYang(int ladder, int layer);

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
    bool IsNotMat(int ladder, int layer);
  };
}