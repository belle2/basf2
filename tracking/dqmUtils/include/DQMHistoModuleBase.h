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

#include <framework/core/HistoModule.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <TH1F.h>
#include <TH2F.h>

using namespace std;

namespace Belle2 {
  /**
   * This class serves as a base for the TrackDQMModule and AlignDQMModule (and possibly other DQM histogram modules).
   *
   * Most of the functions of this class are supposed to be virtual so they can be overridden in derived classes. */
  class DQMHistoModuleBase : public HistoModule {

  public:
    /** Constructor */
    DQMHistoModuleBase();
    /** Destructor */
    ~DQMHistoModuleBase();

    /** Module functions */
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;

    /** Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed to be placed in this function.
     * Also at the end function all m_histogramParameterChanges should be processed via the ProcessHistogramParameterChange function. */
    virtual void defineHisto() override;

    /** Function to create TH1F and add it to the vector of histograms (m_histograms).
     * All histograms in the module should be created via this function (or following Create- functions). */
    TH1F* Create(const char* name, const char* title, int nbinsx, double xlow, double xup, const char* xTitle, const char* yTitle);
    /** Same as above but for TH2F. */
    TH2F* Create(const char* name, const char* title, int nbinsx, double xlow, double xup,  int nbinsy, double ylow, double yup,
                 const char* xTitle, const char* yTitle, const char* zTitle);

    /** Function to create array of TH1F histograms, one for each layer.
     * @param nameTemplate - format() of string with exactly one %1% which is then replaced by the layer number and then used as a name for the histogram.
     * @param titleTemplate - same as nameTemplate but for title. */
    TH1F** CreateLayers(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup,
                        const char* xTitle, const char* yTitle);
    /** Same as above but for TH2F. */
    TH2F** CreateLayers(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup, int nbinsy,
                        double ylow, double yup, const char* xTitle, const char* yTitle, const char* zTitle);

    /** Function to create array of TH1F histograms, one for each sensor.
     * @param nameTemplate - format() of string with exactly one %1% which is then replaced by the output of the SensorNameDescription function and then used as a name for the histogram.
     * @param titleTemplate - same as nameTemplate but for title and with the SensorTitleDescription function. */
    TH1F** CreateSensors(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup,
                         const char* xTitle, const char* yTitle);
    /** Same as above but for TH2F. */
    TH2F** CreateSensors(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup, int nbinsy,
                         double ylow, double yup, const char* xTitle, const char* yTitle, const char* zTitle);

    /** All the following Fill- functions are used by DQMEventProcessorBase or derived classes to fill histograms.
     * They are supposed not to contain any computations need for more than one of them. If that happens, the computations should be moved to the DQMEventProcessorBase or derived classes. */
    virtual void FillTracks(int iTrack, int iTrackVXD, int iTrackCDC, int iTrackVXDCDC);
    virtual void FillHits(int nPXD, int nSVD, int nCDC);
    virtual void FillMomentum(const TrackFitResult* tfr);
    virtual void FillTrackFitResult(const TrackFitResult* tfr);
    virtual void FillTrackFitStatus(const genfit::FitStatus* tfs);
    virtual void FillCorrelations(float fPosSPU, float fPosSPUPrev, float fPosSPV, float fPosSPVPrev, int correlationIndex);
    virtual void FillUBResidualsPXD(float residUPlaneRHUnBias, float residVPlaneRHUnBias);
    virtual void FillUBResidualsSVD(float residUPlaneRHUnBias, float residVPlaneRHUnBias);
    virtual void FillPXDHalfShells(float residUPlaneRHUnBias, float residVPlaneRHUnBias, const VXD::SensorInfoBase* sensorInfo,
                                   bool isNotYang);
    virtual void FillSVDHalfShells(float residUPlaneRHUnBias, float residVPlaneRHUnBias, const VXD::SensorInfoBase* sensorInfo,
                                   bool isNotMat);
    virtual void FillUBResidualsSensor(float residUPlaneRHUnBias, float residVPlaneRHUnBias, int sensorIndex);
    virtual void FillTRClusterHitmap(float fPosSPU, float fPosSPV, int layerIndex);

  protected:
    /** Creates string description of the sensor from given sensor ID to be used in a histogram name. Its used in the CreateSensors functions. */
    static string SensorNameDescription(VxdID sensorID);
    /** Creates string description of the sensor from given sensor ID to be used in a histogram title. Its used in the CreateSensors functions. */
    static string SensorTitleDescription(VxdID sensorID);

    /** Creates a graph of means by given axis from given TH2F histogram.
     * @param output - value of every bin of this histogram is set to be the mean of all bins from input with the same x (or y) coordinate.
     * @param input - any TH2F histogram which NBinsX (or NBinsY) has the same value as the NBinsX of the input.
     * @param onX - if true, use "x" and "NBinsX" options in the statements above. If false, use "y" and "NBinsY" instead. */
    static void ComputeMean(TH1F* output, TH2F* input, bool onX = true);

    /** Process one change in histogram parameters. Should be applied to all items from m_histogramParameterChanges at the end of the defineHisto() function.
     * @param name - name of the histogram whose parameter we want to change.
     * @param parameter - name of the parameter we want to change. Possible values are:
     * name, title, nbinsx, xlow, xup, xTitle, yTitle (for both TH1F and TH2F) and nbinsy, ylow, yup, zTitle (only for TH2F).
     * @param value - new value we wish the parameter of the histogram to have. Int and double values are parsed from string so they must be given correctly. */
    void ProcessHistogramParameterChange(string name, string parameter, string value);
    /** On given histogram sets given parameter to given value. Used in the function above. */
    void EditHistogramParameter(TH1* histogram, string parameter, string value);

    /** All the following Define- functions should be used in the defineHisto() function to define histograms. The convention is that every Define- function is responsible for creating its
     * own TDirectory (if it's needed). In any case the function must then return to the original gDirectory.
     * For the creation of histograms the THFFactory or the Create- functions should be used. */
    virtual void DefineGeneral();
    virtual void DefineUBResiduals();
    virtual void DefineHelixParameters();
    virtual void DefineMomentum();
    virtual void DefineHits();
    virtual void DefineTracks();
    virtual void DefineHalfShells();
    virtual void DefineClusters();
    virtual void DefineSensors();

    /** All histograms created via the Create- functions are automatically added to this set.
     * Its used to easy call Reset() on all histograms in beginRun() and also for changing parameters of histograms via the ProcessHistogramParameterChange function. */
    vector<TH1*> m_histograms;
    /** True if the defineHisto() was called. If false, the event() function does nothing. */
    bool histogramsDefined = false;

    /** Used for changing parameters of histograms via the ProcessHistogramParameterChange function.  */
    vector<tuple<string, string, string>> m_histogramParameterChanges;
    /** StoreArray name where Tracks are written. */
    string m_tracksStoreArrayName;
    /** StoreArray name where RecoTracks are written. */
    string m_recoTracksStoreArrayName;

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

    /** half-shells */
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
    /** Track related clusters - neighbor correlations in Phi */
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