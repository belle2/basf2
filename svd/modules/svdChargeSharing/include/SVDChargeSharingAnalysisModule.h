/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
#include <TList.h>
#include <TCanvas.h>

namespace Belle2 {
  /**
  * Module for monitoring DSSD cluster charge deposition in regard of capacitive charge sharing
  * between adjacent strips in a cluster for data and the simulation.
  */
  class SVDChargeSharingAnalysisModule : public Module {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module. */
    SVDChargeSharingAnalysisModule();

    /** Default destructor. */
    virtual ~SVDChargeSharingAnalysisModule() override;

    /** Initialize the SVDChargeSharingAnalysi module. */
    virtual void initialize() override;

    /** Core method, called for each processed event. */
    virtual void event() override;

    /** Summary method, called after processing of all events. */
    virtual void terminate() override;


  private:

    // Input, output.
    TFile* m_outputRootFile = nullptr; /**< root ouput file pointer. */
    std::string m_outputDirName; /**< output directory. */
    std::string m_outputRootFileName; /**< root output file name. */

    // Module parameters.
    bool m_useTrackInfo = true; /**< True if using clusters related to tracks. */

    // Store arrays.
    StoreArray<SVDCluster> m_svdClusters; /**< SVD clusters store array. */
    StoreArray<Track> m_Tracks; /**< SVD Tracks strore array. */
    StoreArray<RecoTrack> m_recoTracks; /**< SVD RecoTracks store array. */
    StoreArray<TrackFitResult> m_tfr; /**< Track Fit Result store array. */

    // Constants related to SVD geometry.
    static const int m_nLayers = 4; /**< Number of SVD layers. */
    static const int m_nSides = 2; /**< DSSD sides */
    static const int m_nClSizes = 3; /**< Distinction between clSize=1, clSize=2 & clSize>=3. */
    static const int m_nSensorTypes = 4; /**< L3 small rect,. L456 Origami, L456 BWD, L456 FWD. */

    std::string m_nameSensorTypes[m_nSensorTypes] = {"L3", "Origami", "BWD", "FWD"}; /**< Sensor type names. */
    int m_nSensorsOnLayer[m_nLayers] = {2, 3, 4, 5}; /**< Total number of DSSDs on L3,4,5,6. */

    // Histogram related parameters.
    const Double_t m_ADUEquivalent = 375.; /**< Nominal ADUEquivalent, this is just a parameter for histogram bining.*/
    const Int_t m_nBins = 270; /**< Number of bins. */
    const Double_t m_minCharge = 0.; /**< Minimum charge in histogram. */
    const Double_t m_maxCharge = m_nBins * m_ADUEquivalent; /**< 1ADU bin width. */

    // tracks.
    TH1F* h_nTracks = nullptr; /**< Number of tracks. */
    TH1F* h_TracksPvalue = nullptr; /**< Tracks P value. */
    TH1F* h_TracksMomentum = nullptr; /**< Tracks momentum. */
    TH1F* h_TracksnSVDhits = nullptr; /**< Number of SVDhits for a track. */

    // Cluster charge.
    TH1F* h_clCharge[m_nSensorTypes][m_nSides][m_nClSizes] = {nullptr}; /**< SVDCluster charge. */
    TH2F* h_clChargeVsMomentum[m_nSensorTypes][m_nSides][m_nClSizes] = {nullptr}; /**< SVDCluster charge vs. track momentum. */
    TH2F* h_clChargeVsIncidentAngle[m_nSensorTypes][m_nSides][m_nClSizes] = {nullptr}; /**< SVDCluster charge vs. incident angle. */
    TH2F* h_clChargeVsSNR[m_nSensorTypes][m_nSides][m_nClSizes] = {nullptr}; /**< SVDCluster charge vs. SVDCluster SNR. */

    // Cluster size.
    TH1F* h_clSize[m_nSensorTypes][m_nSides] = {nullptr}; /**< SVDCluster size */
    TH2F* h_clSizeVsMomentum[m_nSensorTypes][m_nSides] = {nullptr}; /**< SVDCluster size vs. track momentum. */
    TH2F* h_clSizeVsIncidentAngle[m_nSensorTypes][m_nSides] = {nullptr}; /**< SVDCluster size vs. incident angle. */
    TH2F* h_clSizeVsSNR[m_nSensorTypes][m_nSides] = {nullptr}; /**< SVDCluster size vs. SVDCluster SNR. */

    // cluster SNR.
    TH1F* h_clSNR[m_nSensorTypes][m_nSides][m_nClSizes] = {nullptr}; /**< SVDCluster SNR. */
    TH2F* h_clSNRVsMomentum[m_nSensorTypes][m_nSides][m_nClSizes] = {nullptr}; /**< SVDCluster SNR vs. track momentum. */
    TH2F* h_clSNRVsIncidentAngle[m_nSensorTypes][m_nSides][m_nClSizes] = {nullptr}; /**< SVDCluster SNR vs. incident angle. */

    // histogram lists.
    /** List of all histograms concerning track information: nTracks, Track P Value, momentum and SVD hits */
    TList* m_histoList_Tracks = nullptr;

    TList* m_histoList_clCharge[m_nSensorTypes] = {nullptr}; /**< SVDCluster charge histogram list. */
    TList* m_histoList_clChargeVsMomentum[m_nSensorTypes] = {nullptr}; /**< SVDCluster charge vs. momentum histogram list. */
    TList* m_histoList_clChargeVsIncidentAngle[m_nSensorTypes] = {nullptr}; /**< SVDCluster charge vs. incident angle hist list. */
    TList* m_histoList_clChargeVsSNR[m_nSensorTypes] = {nullptr}; /**< SVDCluster charge vs. SVDCluster SNR histogram list. */

    TList* m_histoList_clSize[m_nSensorTypes] = {nullptr}; /**< SVDCluster size histogram list. */
    TList* m_histoList_clSizeVsMomentum[m_nSensorTypes] = {nullptr}; /**< SVDCluster size vs. track momentum histogram list. */
    TList* m_histoList_clSizeVsIncidentAngle[m_nSensorTypes] = {nullptr}; /**< SVDCluster size vs. incident angle histogram list. */
    TList* m_histoList_clSizeVsSNR[m_nSensorTypes] = {nullptr}; /**< SVDCluster size vs. SVDCluster SNR histogram list. */

    TList* m_histoList_clSNR[m_nSensorTypes] = {nullptr}; /**< SVDCluster SNR histogram list. */
    TList* m_histoList_clSNRVsMomentum[m_nSensorTypes] = {nullptr}; /**< SVDCluster SNR vs. track momentum histogram list. */
    TList* m_histoList_clSNRVsIncidentAngle[m_nSensorTypes] = {nullptr}; /**< SVDCluster SNR vs. incident angle histogram list. */

    // utility methods
    /** 1D histogram creator. */
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, const char* ytitle, TList* histoList = nullptr);

    /** 2D histogram creator. */
    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX,
                            const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY,
                            const char* titleY, TList* histoList);
    /** Used to compare charge histograms for different cluster sizes. */
    TCanvas* comparisonPlot(TH1F* h1, TH1F* h2, TH1F* h3);
  };
}
