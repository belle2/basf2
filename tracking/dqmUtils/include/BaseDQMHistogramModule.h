#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TH1F.h>
#include <TH2F.h>

#include <mdst/dataobjects/EventLevelTrackingInfo.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>

using namespace std;

namespace Belle2 {

  class BaseDQMHistogramModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    BaseDQMHistogramModule();
    /* Destructor */
    ~BaseDQMHistogramModule();

    /** Module functions */
    void initialize() override;
    void beginRun() override;
    void event() override;

    /**
    * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
    * to be placed in this function.
    */
    void defineHisto() override;

    TH1F* Create(const char* name, const char* title, int nbinsx, double xlow, double xup, const char* xTitle, const char* yTitle);
    TH2F* Create(const char* name, const char* title, int nbinsx, double xlow, double xup,  int nbinsy, double ylow, double yup,
                 const char* xTitle, const char* yTitle, const char* zTitle);

    TH1F** CreateLayers(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup,
                        const char* xTitle, const char* yTitle);
    TH2F** CreateLayers(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup, int nbinsy,
                        double ylow, double yup, const char* xTitle, const char* yTitle, const char* zTitle);

    TH1F** CreateSensors(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup,
                         const char* xTitle, const char* yTitle);
    TH2F** CreateSensors(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup, int nbinsy,
                         double ylow, double yup, const char* xTitle, const char* yTitle, const char* zTitle);

    virtual void FillTracks(int iTrack, int iTrackVXD, int iTrackCDC, int iTrackVXDCDC);
    virtual void FillHits(int nPXD, int nSVD, int nCDC);
    virtual void FillMomentum(const TrackFitResult* tfr);
    virtual void FillTrackFitResult(const TrackFitResult* tfr);
    virtual void FillTrackFitStatus(const genfit::FitStatus* tfs);
    virtual void FillCorrelations(float fPosSPU, float fPosSPUPrev, float fPosSPV, float fPosSPVPrev, int correlationIndex);
    virtual void FillUBResidualsPXD(float ResidUPlaneRHUnBias, float ResidVPlaneRHUnBias);
    virtual void FillUBResidualsSVD(float ResidUPlaneRHUnBias, float ResidVPlaneRHUnBias);
    virtual void FillPXDHalfShells(float ResidUPlaneRHUnBias, float ResidVPlaneRHUnBias, const VXD::SensorInfoBase* sensorInfo,
                                   bool isNotYang);
    virtual void FillSVDHalfShells(float ResidUPlaneRHUnBias, float ResidVPlaneRHUnBias, const VXD::SensorInfoBase* sensorInfo,
                                   bool isNotMat);
    virtual void FillUBResidualsSensor(float ResidUPlaneRHUnBias, float ResidVPlaneRHUnBias, int sensorIndex);
    virtual void FillTRClusterHitmap(float fPosSPU, float fPosSPV, int layerIndex);

  protected:
    vector<TH1*> histograms;

    string SensorNameDescription(VxdID sensorID);
    string SensorTitleDescription(VxdID sensorID);

    virtual void DefineGeneral();
    virtual void DefineUBResiduals();
    virtual void DefineHelixParameters();
    virtual void DefineMomentum();
    virtual void DefineHits();
    virtual void DefineTracks();
    virtual void DefineHalfShells();
    virtual void DefineClusters();
    virtual void DefineSensors();

    void ComputeMean(TH1F* output, TH2F* input, bool onX = true);

    /** StoreArray name where Tracks are written. */
    std::string m_TracksStoreArrayName;
    /** StoreArray name where RecoTracks are written. */
    std::string m_RecoTracksStoreArrayName;

    /// Acccess to the EventLevelTrackingInfo object in the datastore.
    StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;

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

    // half-shells
    /** Unbiased residuals in X for PXD for Ying */
    TH1F* m_UBResidualsPXDX_Ying = nullptr;
    /** Unbiased residuals in X for PXD for Yang */
    TH1F* m_UBResidualsPXDX_Yang = nullptr;
    /** Unbiased residuals in X for PXD for Pat */
    TH1F* m_UBResidualsSVDX_Pat = nullptr;
    /** Unbiased residuals in X for PXD for Mat */
    TH1F* m_UBResidualsSVDX_Mat = nullptr;

    /** Unbiased residuals in Y for PXD for Ying */
    TH1F* m_UBResidualsPXDY_Ying = nullptr;
    /** Unbiased residuals in Y for PXD for Yang */
    TH1F* m_UBResidualsPXDY_Yang = nullptr;
    /** Unbiased residuals in Y for PXD for Pat */
    TH1F* m_UBResidualsSVDY_Pat = nullptr;
    /** Unbiased residuals in Y for PXD for Mat */
    TH1F* m_UBResidualsSVDY_Mat = nullptr;

    /** Unbiased residuals in Z for PXD for Ying */
    TH1F* m_UBResidualsPXDZ_Ying = nullptr;
    /** Unbiased residuals in Z for PXD for Yang */
    TH1F* m_UBResidualsPXDZ_Yang = nullptr;
    /** Unbiased residuals in Z for PXD for Pat */
    TH1F* m_UBResidualsSVDZ_Pat = nullptr;
    /** Unbiased residuals in Z for PXD for Mat */
    TH1F* m_UBResidualsSVDZ_Mat = nullptr;

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
    /** d0 - the signed distance to the IP in the r-phi plane */
    TH1F* m_D0 = nullptr;
    /** d0 vs Phi - the signed distance to the IP in the r-phi plane */
    TH2F* m_PhiD0 = nullptr;
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
  };
}