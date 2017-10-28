/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Giulia Casarosa                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
 * This Module is in an early stage of developement. The comments are mainly for temporal purposes
 * and will be changed and corrected in later stages of developement. So please ignore them.
 */

#ifndef SVD_PERFORMANCE_H_
#define SVD_PERFORMANCE_H_

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>

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

    std::string m_ClusterName; /**< */
    std::string m_TrackFitResultName; /**< */
    std::string m_TrackName; /**< */

    bool isDUT(VxdID::baseType theVxdID);
    bool isDUT_L3(VxdID::baseType theVxdID);
    bool isDUT_L4(VxdID::baseType theVxdID);
    bool isDUT_L5(VxdID::baseType theVxdID);
    bool isDUT_L6(VxdID::baseType theVxdID);

    TList* m_histoList;

    TList* m_histoList_track;
    TList* m_histoList_clustertrk;
    TList* m_histoList_cluster;
    TList* m_histoList_shaper;

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


    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

  private:
    SVDNoiseCalibrations m_NoiseCal;
    SVDPulseShapeCalibrations m_PulseShapeCal;

    int m_ntracks;

    //TRACKS
    TH1F* m_nTracks; /**< number of tracks*/
    TH1F* m_Pvalue; /**< track p value*/
    TH1F* m_mom; /**< track momentum*/
    TH1F* m_nSVDhits; /**< track momentum*/

    //STRIPS //layer sensor side
    TH1F* h_stripNoise[4][2][2];  //strip noise

    //CLUSTERS  //layer sensor side
    TH1F* h_nCltrk[4][2][2]; //number per event
    TH1F* h_cltrkSize[4][2][2]; //size
    //1-STRIP CLUSTERS //layer sensor side
    TH1F* h_1cltrkCharge[4][2][2]; //charge
    TH2F* h_cltrkChargeVSSize[4][2][2]; //charge VS size
    TH1F* h_1cltrkSN[4][2][2]; //signal over noise

    int getSensor(int layer, int sensor)
    {
      int result = 0;
      if (layer == 0)
        result = sensor - 1;
      else if (layer == 1 || layer == 2)
        result = sensor - 2;
      else if (layer == 3)
        result = sensor - 3;
      return result;
    }

  };
}

#endif /* SVDPerformanceModule_H_ */

