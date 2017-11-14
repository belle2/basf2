/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio, Giulia Casarosa                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef SVDClusterEvaluation_H
#define SVDClusterEvaluation_H


#include <framework/core/Module.h>
#include <TTree.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <TList.h>
#include <svd/geometry/SensorInfo.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TLegend.h>


namespace Belle2 {

  /*
  */

  class SVDClusterEvaluationModule : public Module {
  public:

    SVDClusterEvaluationModule();

    virtual ~SVDClusterEvaluationModule();

    /** Initialize the SVDClusterEvaluation.
     * This method is called only once before the actual event processing starts.   */
    virtual void initialize() override;

    /** Called when entering a new run.     */
    virtual void beginRun() override;

    /** This method is the core of the SVDClusterEvaluation.
     * This method is called for each event. All processing of the event has to take place in this method.   */
    virtual void event() override;

    /** This method is called if the current run ends. */
    virtual void endRun() override;

    /** This method is called at the end of the event processing.   */
    virtual void terminate() override;


    static const int m_Nsets = 6; // L3-barrel-U,  L3-barrel-V, L456-barrel-U, L456-barrel-V, L456-slanted-U,  L456-slanted-V
    static const int m_NsetsRed = 3;

    //  private:
    TFile* m_outputFile;
    std::string m_outputFileName;

    int indexForHistosAndGraphs;

    TString NameOfHisto, TitleOfHisto;

    TH1F* m_histo_StripTimeResolution[m_Nsets];
    TH1F* m_histo_ClusterTimeResolution[m_Nsets];
    TH1F* m_histo_ClusterTimePull[m_Nsets];
    TH1F* m_histo_ClusterUPositionResolution[m_NsetsRed];
    TH1F* m_histo_ClusterVPositionResolution[m_NsetsRed];
    TH1F* m_histo_ClusterUPositionPull[m_NsetsRed];
    TH1F* m_histo_ClusterVPositionPull[m_NsetsRed];
    TH2F* m_histo2D_TresVsPosres[m_Nsets];
    TH1F* m_histo_PurityInsideTMCluster[m_Nsets];
    TH2F* m_histo2D_PurityInsideTMCluster[m_Nsets];
    TH1F* m_histo_PurityInsideNOTMCluster[m_Nsets];
    TH1F* m_histo_Puddlyness[m_Nsets];
    TH1F* m_histo_PuddlynessTM[m_Nsets];

    float m_mean_StripTimeResolution[m_Nsets];
    float m_RMS_StripTimeResolution[m_Nsets];

    float m_mean_ClusterTimeResolution[m_Nsets];
    float m_RMS_ClusterTimeResolution[m_Nsets];

    float m_mean_ClusterUPositionResolution[m_Nsets];
    float m_RMS_ClusterUPositionResolution[m_Nsets];

    float m_mean_ClusterVPositionResolution[m_Nsets];
    float m_RMS_ClusterVPositionResolution[m_Nsets];

    float m_mean_PurityInsideTMCluster[m_Nsets];
    float m_RMS_PurityInsideTMCluster[m_Nsets];

    float m_mean_Puddlyness[m_Nsets];
    float m_RMS_Puddlyness[m_Nsets];

    float m_mean_PuddlynessTM[m_Nsets];
    float m_RMS_PuddlynessTM[m_Nsets];

    float m_OrderingVec[m_Nsets] = {1., 2., 3., 4., 5., 6.};
    float m_NullVec[m_Nsets] = {0.};

    TList* m_histoList_StripTimeResolution;
    TList* m_histoList_ClusterTimeResolution;
    TList* m_histoList_ClusterTimePull;
    TList* m_histoList_ClusterPositionResolution;
    TList* m_histoList_ClusterPositionPull;
    TList* m_histo2DList_TresVsPosres;
    TList* m_histoList_PurityInsideTMCluster;
    TList* m_histo2DList_PurityInsideTMCluster;
    TList* m_histoList_PurityInsideNOTMCluster;
    TList* m_histoList_Puddlyness;
    TList* m_histoList_PuddlynessTM;
    TList* m_graphList;

    int m_NumberOfShaperDigit[m_Nsets] = {0};
    int m_NumberOfRecoDigit[m_Nsets] = {0};

    int m_NumberOfTH[m_Nsets] = {0};
    int m_NumberOfClustersRelatedToTH[m_Nsets] = {0};

    int m_NumberOfClusters[m_Nsets] = {0};
    int m_NumberOfTMClusters[m_Nsets] = {0};

    int m_NumberOfTMRecoInTMCluster = 0;
    int m_NumberOfTMRecoInNOTMCluster = 0;

    //Control Objects
    TH1F* m_histoControl_MCcharge;
    TH1F* m_histoControl_MCisPrimary;
    TH1F* m_histoControl_THToMCsize;
    TList* m_histoList_Control;

  protected:

    int indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber);

    TString IntExtFromIndex(int idx);

    TString FWFromIndex(int idx);

    TString UVFromIndex(int idx);

    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList);

    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX,
                            const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY,
                            const char* titleY, TList* histoList);

    void createEfficiencyGraph(const char* name, const char* title, int vNum[m_Nsets], int vDen[m_Nsets], TString xTitle,
                               TString yTitle, TList* list);

    void createArbitraryGraphErrorChooser(const char* name, const char* title, float x[m_Nsets], float xErr[m_Nsets], float y[m_Nsets],
                                          float yErr[m_Nsets], TString xTitle, TString yTitle, TList* list, int len);

    void createArbitraryGraphError_Std(const char* name, const char* title, float x[m_Nsets], float xErr[m_Nsets], float y[m_Nsets],
                                       float yErr[m_Nsets], TString xTitle, TString yTitle, TList* list);

    void createArbitraryGraphError_Red(const char* name, const char* title, float x[m_NsetsRed], float xErr[m_NsetsRed],
                                       float y[m_NsetsRed], float yErr[m_NsetsRed], TString xTitle, TString yTitle, TList* list);

    bool goodTrueHit(SVDTrueHit thino);

    /*float getMeanFromHistoWithoutABin(TH1F* histo, int BadBin);*/

  };
}


#endif


























