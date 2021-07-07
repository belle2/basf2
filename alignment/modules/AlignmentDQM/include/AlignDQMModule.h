/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/DQMHistoModuleBase.h>

namespace Belle2 {
  /** DQM of Alignment for off line
    * residuals per sensor, layer,
    * keep also On-Line DQM from tracking:
    * their momentum,
    * Number of hits in tracks,
    * Number of tracks. */
  class AlignDQMModule : public DQMHistoModuleBase {  // <- derived from HistoModule class

  public:
    /** Constructor */
    AlignDQMModule();
    /* Destructor */
    ~AlignDQMModule() { }

    /** Module function event */
    virtual void event() override;
    /** Module function endRun */
    virtual void endRun() override;

    /** Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed to be placed in this function.
     * Also at the end function all m_histogramParameterChanges should be processed via the ProcessHistogramParameterChange function. */
    virtual void defineHisto() override;

    /** Function to create TH1F and add it to the vector of histograms (m_histograms).
     * All histograms in the module should be created via this function (or following Create- functions).
     * This function calls base function but wirh "Alig_" prefix to the name parameter */
    virtual TH1F* Create(std::string name, std::string title, int nbinsx, double xlow, double xup, std::string xTitle,
                         std::string yTitle) override;
    /** Same as above but for TH2F. */
    virtual TH2F* Create(std::string name, std::string title, int nbinsx, double xlow, double xup,  int nbinsy, double ylow, double yup,
                         std::string xTitle, std::string yTitle, std::string zTitle) override;

    /** @name Fill- functions
     * All the following Fill- functions are used by DQMEventProcessorBase or derived classes to fill histograms.
     * They are supposed not to contain any computations need for more than one of them. All computations should be moved to the DQMEventProcessorBase or derived classes. */
    /** @{ */
    /** Fill histograms with helix parameters and their correlations. */
    virtual void FillHelixParametersAndCorrelations(const TrackFitResult* tfr) override;
    /** Fill histograms which depend on position for individual sensors. */
    virtual void FillPositionSensors(TVector3 residual_um, TVector3 position, int sensorIndex);
    /** Fill histograms which depend on layerIndex */
    virtual void FillLayers(TVector3 residual_um, float phi_deg, float theta_deg, int layerIndex);
    /** @} */

  protected:
    /** All the following Define- functions should be used in the defineHisto() function to define histograms. The convention is that every Define- function is responsible for creating its
     * own TDirectory (if it's needed). In any case the function must then return to the original gDirectory.
     * For the creation of histograms the THFFactory or the Create- functions should be used. */
    /** @{ */
    /** Define histograms with helix parameters and their correlations. */
    virtual void DefineHelixParametersAndCorrelations() override;
    /** Define histograms which depend on position for individual sensors. */
    virtual void DefineSensors() override;
    /** Define histograms which depend on layerIndex */
    virtual void DefineLayers();
    /** @} */

    /** Special Alignment related: Sensor level */
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

    /** Special Alignment related: Layer level */
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

    /** helix parameters and their corellations */
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
