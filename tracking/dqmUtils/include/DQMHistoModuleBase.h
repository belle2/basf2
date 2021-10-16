/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <TH1F.h>
#include <TH2F.h>

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

    /** function called when the module is run on HLT */
    void runningOnHLT() {m_hltDQM = true;};

    /** Function to create TH1F and add it to the vector of histograms (m_histograms).
     * All histograms in the module should be created via this function (or following Create- functions). */
    virtual TH1F* Create(std::string name, std::string title, int nbinsx, double xlow, double xup, std::string xTitle,
                         std::string yTitle);
    /** Same as above but for TH2F. */
    virtual TH2F* Create(std::string name, std::string title, int nbinsx, double xlow, double xup, int nbinsy, double ylow, double yup,
                         std::string xTitle, std::string yTitle, std::string zTitle);

    /** Function to create array of TH1F histograms, one for each layer.
     * @param nameTemplate - format() of string with exactly one %1% which is then replaced by the layer number and then used as a name for the histogram.
     * @param titleTemplate - same as nameTemplate but for title.
     * @param nbinsx - number of bins.
     * @param xlow - range x-axis (low boundary).
     * @param xup - range x-axis (high boundary).
     * @param xTitle - title x-axis.
     * @param yTitle - title y-axis
     * */
    virtual TH1F** CreateLayers(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup,
                                std::string xTitle, std::string yTitle);
    /** Same as above but for TH2F. */
    virtual TH2F** CreateLayers(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup,
                                int nbinsy, double ylow, double yup, std::string xTitle, std::string yTitle, std::string zTitle);

    /** Function to create array of TH1F histograms, one for each sensor.
     * @param nameTemplate - format() of string with exactly one %1% which is then replaced by the output of the SensorNameDescription function and then used as a name for the histogram.
     * @param titleTemplate - same as nameTemplate but for title and with the SensorTitleDescription function.
     * @param nbinsx - number of bins.
     * @param xlow - range x-axis (low boundary).
     * @param xup - range x-axis (high boundary).
     * @param xTitle - title x-axis.
     * @param yTitle - title y-axis
     * */
    virtual TH1F** CreateSensors(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup,
                                 std::string xTitle, std::string yTitle);
    /** Same as above but for TH2F. */
    virtual TH2F** CreateSensors(boost::format nameTemplate, boost::format titleTemplate, int nbinsx, double xlow, double xup,
                                 int nbinsy, double ylow, double yup, std::string xTitle, std::string yTitle, std::string zTitle);

    /** @name Fill- functions
     * All the following Fill- functions are used by DQMEventProcessorBase or derived classes to fill histograms.
     * They are supposed not to contain any computations need for more than one of them. All computations should be moved to the DQMEventProcessorBase or derived classes. */
    /** @{ */
    /** Fill histograms with track indexes. */
    virtual void FillTrackIndexes(int iTrack, int iTrackVXD, int iTrackCDC, int iTrackVXDCDC);
    /** Fill histograms with numbers of hits. */
    virtual void FillHitNumbers(int nPXD, int nSVD, int nCDC);
    /** Fill histograms with track momentum Pt. angles. */
    virtual void FillMomentumAngles(const TrackFitResult* tfr);
    /** Fill histograms with track momentum Pt. coordinates. */
    virtual void FillMomentumCoordinates(const TrackFitResult* tfr);
    /** Fill histograms with helix parameters and their correlations. */
    virtual void FillHelixParametersAndCorrelations(const TrackFitResult* tfr);
    /** Fill histograms which require FitStatus. */
    virtual void FillTrackFitStatus(const genfit::FitStatus* tfs);
    /** Fill histograms with correlations between neighbor layers. */
    virtual void FillTRClusterCorrelations(float phi_deg, float phiPrev_deg, float theta_deg, float thetaPrev_deg,
                                           int correlationIndex);
    /** Fill cluster hitmap in IP angle range. */
    virtual void FillTRClusterHitmap(float phi_deg, float theta_deg, int layerIndex);
    /** Fill histograms with unbiased residuals in PXD sensors. */
    virtual void FillUBResidualsPXD(TVector3 residual_um);
    /** Fill histograms with unbiased residuals in SVD sensors. */
    virtual void FillUBResidualsSVD(TVector3 residual_um);
    /** Fill histograms with unbiased residuals for half-shells for PXD sensors. */
    virtual void FillHalfShellsPXD(TVector3 globalResidual_um, bool isNotYang);
    /** Fill histograms with unbiased residuals for half-shells for SVD sensors. */
    virtual void FillHalfShellsSVD(TVector3 globalResidual_um, bool isNotMat);
    /** Fill histograms with unbiased residuals for individual sensors. */
    virtual void FillUBResidualsSensor(TVector3 residual_um, int sensorIndex);
    /** @} */

  protected:
    /** Creates string description of the sensor from given sensor ID to be used in a histogram name. Its used in the CreateSensors functions. */
    static std::string SensorNameDescription(VxdID sensorID);
    /** Creates string description of the sensor from given sensor ID to be used in a histogram title. Its used in the CreateSensors functions. */
    static std::string SensorTitleDescription(VxdID sensorID);

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
    void ProcessHistogramParameterChange(const std::string& name, const std::string& parameter, const std::string& value);
    /** On given histogram sets given parameter to given value. Used in the function above. */
    void EditHistogramParameter(TH1* histogram, const std::string& parameter, std::string value);

    /** All the following Define- functions should be used in the defineHisto() function to define histograms. The convention is that every Define- function is responsible for creating its
     * own TDirectory (if it's needed). In any case the function must then return to the original gDirectory.
     * For the creation of histograms the THFFactory or the Create- functions should be used. */
    /** @{ */
    /** Define histograms with track indexes. */
    virtual void DefineTracks();
    /** Define histograms with numbers of hits. */
    virtual void DefineHits();
    /** Define histograms with track momentum Pt. angles. */
    virtual void DefineMomentumAngles();
    /** Define histograms with track momentum Pt. coordinates. */
    virtual void DefineMomentumCoordinates();
    /** Define histograms with helix parameters and their correlations. */
    virtual void DefineHelixParametersAndCorrelations();
    /** Define histograms which require FitStatus. */
    virtual void DefineTrackFitStatus();
    /** Define histograms with correlations between neighbor layers and cluster hitmap in IP angle range. */
    virtual void DefineTRClusters();
    /** Define histograms with unbiased residuals in PXD and SVD sensors. */
    virtual void DefineUBResidualsVXD();
    /** Define histograms with unbiased residuals for half-shells for PXD and SVD sensors. */
    virtual void DefineHalfShellsVXD();
    /** Define histograms with unbiased residuals for individual sensors. */
    virtual void DefineSensors();
    /** @} */

    /** All histograms created via the Create- functions are automatically added to this set.
     * Its used to easy call Reset() on all histograms in beginRun() and also for changing parameters of histograms via the ProcessHistogramParameterChange function. */
    std::vector<TH1*> m_histograms;
    /** True if the defineHisto() was called. If false, the event() function does nothing. */
    bool histogramsDefined = false;
    bool m_hltDQM = false;

    /** Used for changing parameters of histograms via the ProcessHistogramParameterChange function.  */
    std::vector<std::tuple<std::string, std::string, std::string>> m_histogramParameterChanges;
    /** StoreArray name where Tracks are written. */
    std::string m_tracksStoreArrayName;
    /** StoreArray name where RecoTracks are written. */
    std::string m_recoTracksStoreArrayName;

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
    /** Unbiased residuals in X for PXD for Yin */
    TH1F* m_UBResidualsPXDX_Yin = nullptr;
    /** Unbiased residuals in X for PXD for Yang */
    TH1F* m_UBResidualsPXDX_Yang = nullptr;
    /** Unbiased residuals in X for PXD for Pat */
    TH1F* m_UBResidualsSVDX_Pat = nullptr;
    /** Unbiased residuals in X for PXD for Mat */
    TH1F* m_UBResidualsSVDX_Mat = nullptr;

    /** Unbiased residuals in Y for PXD for Yin */
    TH1F* m_UBResidualsPXDY_Yin = nullptr;
    /** Unbiased residuals in Y for PXD for Yang */
    TH1F* m_UBResidualsPXDY_Yang = nullptr;
    /** Unbiased residuals in Y for PXD for Pat */
    TH1F* m_UBResidualsSVDY_Pat = nullptr;
    /** Unbiased residuals in Y for PXD for Mat */
    TH1F* m_UBResidualsSVDY_Mat = nullptr;

    /** Unbiased residuals in Z for PXD for Yin */
    TH1F* m_UBResidualsPXDZ_Yin = nullptr;
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
