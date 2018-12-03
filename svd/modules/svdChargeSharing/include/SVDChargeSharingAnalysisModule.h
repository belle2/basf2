/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Mateusz Kaleta                                           *
*                                                                        *
* This software is provided "as is" without any warranty.                *
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

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    SVDChargeSharingAnalysisModule();

    /**  */
    virtual ~SVDChargeSharingAnalysisModule() override;

    /**  */
    virtual void initialize() override;

    /**  */
    virtual void event() override;

    /**  */
    virtual void terminate() override;


  private:

    TFile* m_outputRootFile;
    std::string m_outputDirName;
    std::string m_outputRootFileName;

    bool m_useTrackInfo;
    bool m_is2017TBanalysis;

    StoreArray<SVDCluster> m_svdClusters;
    StoreArray<RecoTrack> m_recoTracks;
    StoreArray<Track> m_Tracks;
    StoreArray<TrackFitResult> m_tfr;

    static const int m_nLayers = 4;
    static const int m_nSides = 2;
    static const int m_nClSizes = 3;  //distinction between clSize = 1, clSize = 2 & clSize >= 3
    static const int m_nSensorTypes = 4;  // L3 small rect., L456 Origami, L456 BWD, L456 FWD

    std::string m_nameSensorTypes[m_nSensorTypes] = {"L3", "Origami", "BWD", "FWD"};
    int m_nSensorsOnLayer[m_nLayers] = {2, 3, 4, 5};

    const Double_t m_ADUEquivalent = 375.;
    const Int_t m_nBins = 270;
    const Double_t m_minCharge = 0.;
    const Double_t m_maxCharge = m_nBins * m_ADUEquivalent; // 1 ADU bin width

    // tracks
    TH1F* h_nTracks;
    TH1F* h_TracksPvalue;
    TH1F* h_TracksMomentum;
    TH1F* h_TracksnSVDhits;

    // Cluster charge
    TH1F* h_clCharge[m_nSensorTypes][m_nSides][m_nClSizes];
    TH2F* h_clChargeVsMomentum[m_nSensorTypes][m_nSides][m_nClSizes];
    TH2F* h_clChargeVsIncidentAngle[m_nSensorTypes][m_nSides][m_nClSizes];
    TH2F* h_clChargeVsSNR[m_nSensorTypes][m_nSides][m_nClSizes];

    // Cluster size
    TH1F* h_clSize[m_nSensorTypes][m_nSides];
    TH2F* h_clSizeVsMomentum[m_nSensorTypes][m_nSides];
    TH2F* h_clSizeVsIncidentAngle[m_nSensorTypes][m_nSides];
    TH2F* h_clSizeVsSNR[m_nSensorTypes][m_nSides];

    //cluster SNR
    TH1F* h_clSNR[m_nSensorTypes][m_nSides][m_nClSizes];
    TH2F* h_clSNRVsMomentum[m_nSensorTypes][m_nSides][m_nClSizes];
    TH2F* h_clSNRVsIncidentAngle[m_nSensorTypes][m_nSides][m_nClSizes];

    // histogram lists
    TList* m_histoList_Tracks;

    TList* m_histoList_clCharge[m_nSensorTypes];
    TList* m_histoList_clChargeVsMomentum[m_nSensorTypes];
    TList* m_histoList_clChargeVsIncidentAngle[m_nSensorTypes];
    TList* m_histoList_clChargeVsSNR[m_nSensorTypes];

    TList* m_histoList_clSize[m_nSensorTypes];
    TList* m_histoList_clSizeVsMomentum[m_nSensorTypes];
    TList* m_histoList_clSizeVsIncidentAngle[m_nSensorTypes];
    TList* m_histoList_clSizeVsSNR[m_nSensorTypes];

    TList* m_histoList_clSNR[m_nSensorTypes];
    TList* m_histoList_clSNRVsMomentum[m_nSensorTypes];
    TList* m_histoList_clSNRVsIncidentAngle[m_nSensorTypes];

    // utility methods
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, const char* ytitle, TList* histoList = NULL);

    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX,
                            const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY,
                            const char* titleY, TList* histoList);

    TCanvas* comparisonPlot(TH1F* h1, TH1F* h2, TH1F* h3);
  };
}
