/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Giulia Casarosa                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_PERFORMANCE_H_
#define SVD_PERFORMANCE_H_

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/datastore/StoreArray.h>

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>

#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>
#include <TCollection.h>
#include <TList.h>
#include <TH3F.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {

  /** The (TB) SVD Performance Module
   *
   */

  class SVDPerformanceModule : public Module {

  public:

    SVDPerformanceModule();

    virtual ~SVDPerformanceModule();
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    std::string m_ShaperDigitName; /**< */
    std::string m_RecoDigitName; /**< */
    std::string m_ClusterName; /**< */
    std::string m_TrackFitResultName; /**< */
    std::string m_TrackName; /**< */
    bool m_is2017TBanalysis; /**< true if we analyze 2017 TB data*/
    bool m_isSimulation; /**< true if we analyze Simulated data*/

    float m_debugLowTime; /** cluster Time below this number will produce a printout */


    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

  private:

    int m_nEvents;

    SVDNoiseCalibrations m_NoiseCal;
    SVDPulseShapeCalibrations m_PulseShapeCal;

    StoreArray<SVDShaperDigit> m_svdShapers;
    StoreArray<SVDRecoDigit> m_svdRecos;
    StoreArray<SVDCluster> m_svdClusters;
    StoreArray<RecoTrack> m_recoTracks;
    StoreArray<Track> m_Tracks;
    StoreArray<TrackFitResult> m_tfr;

    int m_ntracks;

    static const int m_nLayers = 4;
    static const int m_nSensors = 5;
    static const int m_nSides = 2;

    unsigned int sensorsOnLayer[4];

    TList* m_histoList_track;
    TList* m_histoList_corr;
    TList* m_histoList_clTRK[m_nLayers];
    TList* m_histoList_cluster[m_nLayers];
    TList* m_histoList_shaper[m_nLayers];
    TList* m_histoList_reco[m_nLayers];

    //TRACKS
    TH1F* m_nTracks; /**< number of tracks*/
    TH1F* m_Pvalue; /**< track p value*/
    TH1F* m_mom; /**< track momentum*/
    TH1F* m_nSVDhits; /**< track momentum*/

    //SHAPER
    TH1F* h_nShaper[m_nLayers][m_nSensors][m_nSides]; //number per event

    //RECO
    TH1F* h_nReco[m_nLayers][m_nSensors][m_nSides]; //number per event
    TH1F* h_recoCharge[m_nLayers][m_nSensors][m_nSides]; //charge
    TH1F* h_recoEnergy[m_nLayers][m_nSensors][m_nSides]; //energy
    TH1F* h_stripNoise[m_nLayers][m_nSensors][m_nSides];  //strip noise
    TH1F* h_recoTime[m_nLayers][m_nSensors][m_nSides];  //time

    //CLUSTERS NOT RELATED TO TRACKS
    TH1F* h_nCl[m_nLayers][m_nSensors][m_nSides]; //number per event
    TH1F* h_clSize[m_nLayers][m_nSensors][m_nSides]; //size
    TH1F* h_clCharge[m_nLayers][m_nSensors][m_nSides]; //charge
    TH1F* h_clEnergy[m_nLayers][m_nSensors][m_nSides]; //energy
    TH1F* h_clSeedMaxbin[m_nLayers][m_nSensors][m_nSides]; //maxbin seed
    TH2F* h_clEnergyVSMaxbin[m_nLayers][m_nSensors][m_nSides]; //energy VS maxbin seed
    TH2F* h_clEnergyVSCoorU[m_nLayers][m_nSensors][m_nSides]; //energy VS position U
    TH2F* h_clEnergyVSCoorV[m_nLayers][m_nSensors][m_nSides]; //energy VS position V
    TH2F* h_clCoorUVSCoorV[m_nLayers][m_nSensors]; //energy VS position
    TH1F* h_clCoor1VSCoor2[m_nLayers][m_nSensors][m_nSides]; //coor1 VS coor2
    TH2F* h_clEnergy12VSdelta[m_nLayers][m_nSensors][m_nSides]; //coor1 VS coor2
    TH1F* h_clCellID1VSCellID2[m_nLayers][m_nSensors][m_nSides]; //coor1 VS coor2
    TH2F* h_clEnergyUVSEnergyV[m_nLayers][m_nSensors]; //energy VS position
    TH1F* h_clSN[m_nLayers][m_nSensors][m_nSides]; //signal over noise
    TH1F* h_clTime[m_nLayers][m_nSensors][m_nSides];  //time
    TH2F* h_clChargeVSSize[m_nLayers][m_nSensors][m_nSides]; //charge VS size
    TH2F* h_clEnergyVSSize_mb12[m_nLayers][m_nSensors][m_nSides]; //energy VS size, maxbin == 1,2
    TH2F* h_clEnergyVSSize_mb345[m_nLayers][m_nSensors][m_nSides]; //energy VS size, maxbin == 3,4,5
    TH2F* h_clEnergyVSSize_mb6[m_nLayers][m_nSensors][m_nSides]; //energy VS size, max bin == 6
    TH2F* h_clSNVSSize[m_nLayers][m_nSensors][m_nSides]; //charge VS size
    TH2F* h_clTimeVSSize[m_nLayers][m_nSensors][m_nSides]; //charge VS size
    TH2F* h_clTimeVSTrueTime[m_nLayers][m_nSensors][m_nSides];  //time VS true time

    //CLUSTERS RELATED TO TRACKS
    TH1F* h_nCltrk[m_nLayers][m_nSensors][m_nSides]; //number per event
    TH1F* h_cltrkSize[m_nLayers][m_nSensors][m_nSides]; //size
    TH1F* h_cltrkCharge[m_nLayers][m_nSensors][m_nSides]; //charge
    TH1F* h_cltrkEnergy[m_nLayers][m_nSensors][m_nSides]; //energy
    TH1F* h_cltrkSN[m_nLayers][m_nSensors][m_nSides]; //signal over noise
    TH1F* h_cltrkTime[m_nLayers][m_nSensors][m_nSides];  //time
    TH2F* h_cltrkChargeVSSize[m_nLayers][m_nSensors][m_nSides]; //charge VS size
    TH2F* h_cltrkSNVSSize[m_nLayers][m_nSensors][m_nSides]; //charge VS size
    TH2F* h_cltrkTimeVSSize[m_nLayers][m_nSensors][m_nSides]; //charge VS size
    TH2F* h_cltrkTimeVSTrueTime[m_nLayers][m_nSensors][m_nSides];  //time VS true time

    //1-STRIP CLUSTERS
    TH1F* h_1cltrkCharge[m_nLayers][m_nSensors][m_nSides]; //charge
    TH1F* h_1cltrkSN[m_nLayers][m_nSensors][m_nSides]; //signal over noise

    //2-STRIP CLUSTERS
    TH1F* h_2cltrkCharge[m_nLayers][m_nSensors][m_nSides]; //charge
    TH1F* h_2cltrkSN[m_nLayers][m_nSensors][m_nSides]; //signal over noise

    //CORRELATIONS
    TH1F* h_cltrk_UU;
    TH1F* h_cltrk_VV;
    TH1F* h_cltrk_UV;

    //    TH1F* h_cl_UU;
    //    TH1F* h_cl_VV;
    //    TH1F* h_cl_UV;

    TH2F* h_cltrkTime_L4uL5u;
    TH2F* h_cltrkTime_L4vL5v;
    TH2F* h_cltrkTime_L5uL5v;

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

    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = NULL);  /**< thf */

    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            TList* histoList = NULL);  /**< thf */

    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            Int_t nbinsZ, Double_t minZ, Double_t maxZ, const char* titleZ,
                            TList* histoList = NULL);  /**< thf */

    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t* binsX, const char* titleX,
                            Int_t nbinsY, Double_t* binsY, const char* titleY,
                            Int_t nbinsZ, Double_t* binsZ, const char* titleZ,
                            TList* histoList = NULL);  /**< thf */

    TH1* duplicateHistogram(const char* newname, const char* newtitle,
                            TH1* h, TList* histoList = NULL);  /**< thf */


    TH1F* createHistogramsRatio(const char* name, const char* title,
                                TH1* hNum, TH1* hDen, bool isEffPlot,
                                int axisRef);  /**< thf */



    void addEfficiencyPlots(TList* graphList = NULL, TH3F* h3_xPerMCParticle = NULL, TH3F* h3_MCParticle = NULL);  /**< efficiency */
    void addInefficiencyPlots(TList* graphList = NULL, TH3F* h3_xPerMCParticle = NULL,
                              TH3F* h3_MCParticle = NULL);  /**< inefficiency */
    void addPurityPlots(TList* graphList = NULL, TH3F* h3_xPerMCParticle = NULL, TH3F* h3_MCParticle = NULL);  /**< purity */

  };
}

#endif /* SVDPerformanceModule_H_ */

