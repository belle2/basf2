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
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/dataobjects/SVDEventInfo.h>

#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TList.h>

namespace Belle2 {

  /** The (TB) SVD Performance Module
   *
   */

  class SVDPerformanceModule : public Module {

  public:

    SVDPerformanceModule();

    virtual ~SVDPerformanceModule();
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    std::string m_ShaperDigitName = "SVDShaperDigits"; /**< ShaperDigits Store Array name*/
    std::string m_RecoDigitName = "SVDRecoDigits"; /**< SVDRecoDigits Store Array name*/
    std::string m_ClusterName = "SVDClusters"; /**< SVDCluster StoreArray name */
    std::string m_TrackFitResultName = "TrackFitResults"; /**< TrackFitResult StoreArray name*/
    std::string m_TrackName = "Tracks"; /**< Track StoreArray name*/
    bool m_is2017TBanalysis = false; /**< true if we analyze 2017 TB data*/
    bool m_isSimulation = false; /**< true if we analyze Simulated data*/

    float m_debugLowTime = - 100; /**< cluster Time below this number will produce a printout */


    /* user-defined parameters */
    std::string m_rootFileName = "";   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

  private:

    int m_nEvents = 0; /**< number of events*/

    SVDNoiseCalibrations m_NoiseCal; /**<SVDNoise calibration db object*/
    SVDPulseShapeCalibrations m_PulseShapeCal; /**<SVDPulseShape calibration db object*/

    StoreArray<SVDShaperDigit> m_svdShapers; /**<SVDShaperDigit store array*/
    StoreArray<SVDRecoDigit> m_svdRecos; /**<SVDRecoDigits store array*/
    StoreArray<SVDCluster> m_svdClusters; /**<SVDCluster store array*/
    StoreArray<RecoTrack> m_recoTracks; /**<RecoTracks store array*/
    StoreArray<Track> m_Tracks; /**<Tracks store array*/
    StoreArray<TrackFitResult> m_tfr; /**<TrackFitResult store array*/

    /** Storage for SVDEventInfo object */
    StoreObjPtr<SVDEventInfo> m_storeSVDEvtInfo;

    /** Name of the SVDEventInfo object */
    std::string m_svdEventInfoName;

    int m_ntracks = 0; /**<numner of tracks*/

    static const int m_nLayers = 4; /**< max number of layers*/
    static const int m_nSensors = 5; /**<max number of sensors*/
    static const int m_nSides = 2; /**<max number of sides*/

    unsigned int sensorsOnLayer[4] = {0}; /**< sensors on layer i*/

    TList* m_histoList_track = nullptr;  /**< histo list tracks*/
    TList* m_histoList_corr = nullptr; /**< histo list correlations*/
    TList* m_histoList_clTRK[m_nLayers] = {nullptr}; /**< histo list clusters related to tracks*/
    TList* m_histoList_cluster[m_nLayers] = {nullptr}; /**< histo list  clusters*/
    TList* m_histoList_shaper[m_nLayers] = {nullptr}; /**< histo list shaper digits */
    TList* m_histoList_reco[m_nLayers] = {nullptr}; /**< histo list reco digits*/

    //TRACKS
    TH1F* m_nTracks = nullptr; /**< number of tracks*/
    TH1F* m_Pvalue = nullptr; /**< track p value*/
    TH1F* m_mom = nullptr; /**< track momentum*/
    TH1F* m_nSVDhits = nullptr; /**< track momentum*/

    //SHAPER
    TH1F* h_nShaper[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<number per event*/

    //RECO
    TH1F* h_nReco[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<number per event*/
    TH1F* h_recoCharge[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge*/
    TH1F* h_recoEnergy[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<energy*/
    TH1F* h_stripNoise[m_nLayers][m_nSensors][m_nSides] = {nullptr};  /**<strip noise*/
    TH1F* h_recoTime[m_nLayers][m_nSensors][m_nSides] = {nullptr};  /**<time*/

    //CLUSTERS NOT RELATED TO TRACKS
    TH1F* h_nCl[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<number per event*/
    TH1F* h_clSize[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<size*/
    TH1F* h_clCharge[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge*/
    TH1F* h_clEnergy[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<energy*/
    TH1F* h_clSeedMaxbin[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<maxbin seed*/
    TH2F* h_clEnergyVSMaxbin[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<energy VS maxbin seed*/
    TH2F* h_clEnergyVSCoorU[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<energy VS position U*/
    TH2F* h_clEnergyVSCoorV[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<energy VS position V*/
    TH2F* h_clNuVSNv[m_nLayers][m_nSensors] = {nullptr}; /**<N U culsters VS N V clusters*/
    TH2F* h_clCoorUVSCoorV[m_nLayers][m_nSensors] = {nullptr}; /**<energy VS position*/
    TH1F* h_clCoor1VSCoor2[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<coor1 VS coor2*/
    TH2F* h_clEnergy12VSdelta[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<coor1 VS coor2*/
    TH1F* h_clCellID1VSCellID2[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<coor1 VS coor2*/
    TH2F* h_clEnergyUVSEnergyV[m_nLayers][m_nSensors] = {nullptr}; /**<energy VS position*/
    TH1F* h_clSN[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<signal over noise*/
    TH1F* h_clTime[m_nLayers][m_nSensors][m_nSides] = {nullptr};  /**<time*/
    TH2F* h_clChargeVSSize[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge VS size*/
    TH2F* h_clEnergyVSSize_mb12[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<energy VS size, maxbin == 1,2*/
    TH2F* h_clEnergyVSSize_mb345[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<energy VS size, maxbin == 3,4,5*/
    TH2F* h_clEnergyVSSize_mb6[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<energy VS size, max bin == 6*/
    TH2F* h_clSNVSSize[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge VS size*/
    TH2F* h_clTimeVSSize[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge VS size*/
    TH2F* h_clTimeVSTrueTime[m_nLayers][m_nSensors][m_nSides] = {nullptr};  /**<time VS true time*/

    //CLUSTERS RELATED TO TRACKS
    TH1F* h_nCltrk[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<number per event*/
    TH1F* h_cltrkSize[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<size*/
    TH1F* h_cltrkCharge[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge*/
    TH1F* h_cltrkEnergy[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<energy*/
    TH1F* h_cltrkSN[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<signal over noise*/
    TH1F* h_cltrkTime[m_nLayers][m_nSensors][m_nSides] = {nullptr};  /**<time*/
    TH1F* h_cltrkTime_TB1[m_nLayers][m_nSensors][m_nSides] = {nullptr};  /**<time*/
    TH1F* h_cltrkTime_TB2[m_nLayers][m_nSensors][m_nSides] = {nullptr};  /**<time*/
    TH1F* h_cltrkTime_TB3[m_nLayers][m_nSensors][m_nSides] = {nullptr};  /**<time*/
    TH1F* h_cltrkTime_TB4[m_nLayers][m_nSensors][m_nSides] = {nullptr};  /**<time*/
    TH2F* h_cltrkChargeVSSize[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge VS size*/
    TH2F* h_cltrkSNVSSize[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge VS size*/
    TH2F* h_cltrkTimeVSSize[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge VS size*/
    TH2F* h_cltrkTimeVSTrueTime[m_nLayers][m_nSensors][m_nSides] = {nullptr};  /**<time VS true time*/

    //1-STRIP CLUSTERS
    TH1F* h_1cltrkCharge[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge*/
    TH1F* h_1cltrkSN[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<signal over noise*/

    //2-STRIP CLUSTERS
    TH1F* h_2cltrkCharge[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<charge*/
    TH1F* h_2cltrkSN[m_nLayers][m_nSensors][m_nSides] = {nullptr}; /**<signal over noise*/

    //CORRELATIONS
    TH1F* h_cltrk_UU = nullptr; /**< U time vs U time*/
    TH1F* h_cltrk_VV = nullptr; /**< V time vs V time*/
    TH1F* h_cltrk_UV = nullptr; /**< U time vs V time*/

    TH2F* h_cltrkTime_L4uL5u = nullptr; /**< L4U time VS L5U time*/
    TH2F* h_cltrkTime_L4vL5v = nullptr; /**< L4V time VS L5V time*/
    TH2F* h_cltrkTime_L5uL5v = nullptr; /**< L5U time VS L5V time*/

    /** get sensor number*/
    int getSensor(int layer, int sensor, bool isTB)
    {
      int result = 0;
      if (isTB) {
        if (layer == 0)
          result = sensor - 1;
        else if (layer == 1 || layer == 2)
          result = sensor - 2;
        else if (layer == 3)
          result = sensor - 3;
      } else result = sensor - 1;

      return result;
    }

    /** Function returning a TH1F */
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = nullptr);
    /** Function returning TH2F */
    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            TList* histoList = nullptr);

  };
}


