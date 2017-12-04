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


    static const int m_Nsets = 6; // L3-barrel-U, L3-barrel-V, L456-barrel-U, L456-barrel-V, L456-slanted-U,  L456-slanted-V
    static const int m_NsetsRed = 3;

    //  private:
    TFile* m_outputFile;
    std::string m_outputFileName;

    /** Index used for the lists and for the vectors of histograms: it indicates the set of sensors we are looking at */
    int indexForHistosAndGraphs;

    /** Strings to pass names and titles to the histos in the vectors of hitos */
    TString NameOfHisto, TitleOfHisto;

    /** Vector of histograms depicting Strip Time Residuals */
    TH1F* m_histo_StripTimeResolution[m_Nsets];
    /** Vector of histograms depicting Cluster Time Residuals */
    TH1F* m_histo_ClusterTimeResolution[m_Nsets];
    /** Vector of histograms depicting Cluster Time Residuals, divided by TriggerBin */
    TH1F* m_histo_ClusterTimeResolution_bin1[m_Nsets];
    TH1F* m_histo_ClusterTimeResolution_bin2[m_Nsets];
    TH1F* m_histo_ClusterTimeResolution_bin3[m_Nsets];
    TH1F* m_histo_ClusterTimeResolution_bin4[m_Nsets];
    /** Vector of histograms depicting Cluster Time Pull */
    TH1F* m_histo_ClusterTimePull[m_Nsets];
    /** Vector of histograms depicting Cluster U Position Residual (Reduced length!) */
    TH1F* m_histo_ClusterUPositionResolution[m_NsetsRed];
    /** Vector of histograms depicting Cluster V Position Residual (Reduced length!) */
    TH1F* m_histo_ClusterVPositionResolution[m_NsetsRed];
    /** Vector of histograms depicting Cluster U Position Pull (Reduced length!) */
    TH1F* m_histo_ClusterUPositionPull[m_NsetsRed];
    /** Vector of histograms depicting Cluster U Position Pull (Reduced length!) */
    TH1F* m_histo_ClusterVPositionPull[m_NsetsRed];
    /** Vector of 2D histograms depicting Time Residuals Vs Position (U/V) Residuals for Histos */
    TH2F* m_histo2D_TresVsPosres[m_Nsets];
    /** Vector of histograms depicting Cluster Internal Purity (TM Recos over Reco inside a Cluster) */
    TH1F* m_histo_PurityInsideTMCluster[m_Nsets];
    /** Vector of 2D histograms depicting TM Reco Vs Total Reco inside a TM Cluster */
    TH2F* m_histo2D_PurityInsideTMCluster[m_Nsets];
    /** Vector of histograms depicting TM Cluster Internal Purity (TM Recos over Reco inside a Cluster) */
    TH1F* m_histo_PurityInsideNOTMCluster[m_Nsets];
    /** Vector of histograms depicting Number of TH inside a Cluster */
    TH1F* m_histo_THinCluster[m_Nsets];
    /** Vector of histograms depicting Number of TH inside a TM Cluster */
    TH1F* m_histo_THinClusterTM[m_Nsets];
    /** Vector of histograms depicting Number of Good TH inside a TM Cluster */
    TH1F* m_histo_GoodTHinClusterTM[m_Nsets];
    /** Vector of histograms depicting Number of Good TH inside a Good TM Cluster */
    TH1F* m_histo_GoodTHinClusterTMGood[m_Nsets];

    /** Vectors of floats containing the mean and the RMS from the corresponding histo */
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
    float m_mean_THinCluster[m_Nsets];
    float m_RMS_THinCluster[m_Nsets];
    float m_mean_THinClusterTM[m_Nsets];
    float m_RMS_THinClusterTM[m_Nsets];
    float m_mean_GoodTHinClusterTM[m_Nsets];
    float m_RMS_GoodTHinClusterTM[m_Nsets];
    float m_mean_GoodTHinClusterTMGood[m_Nsets];
    float m_RMS_GoodTHinClusterTMGood[m_Nsets];

    /** Vectors used to Draw the TGraphs (defined in the cc) depicting the averages and the means of the histos above for the various sets of sensor*/
    float m_OrderingVec[m_Nsets] = {1., 2., 3., 4., 5., 6.};
    float m_NullVec[m_Nsets] = {0.};

    /** Lists used to easily Draw the corresponding histos; last one is used to draw the TGraphs */
    TList* m_histoList_StripTimeResolution;
    TList* m_histoList_ClusterTimeResolution;
    TList* m_histoList_ClusterTimePull;
    TList* m_histoList_ClusterPositionResolution;
    TList* m_histoList_ClusterPositionPull;
    TList* m_histo2DList_TresVsPosres;
    TList* m_histoList_PurityInsideTMCluster;
    TList* m_histo2DList_PurityInsideTMCluster;
    TList* m_histoList_PurityInsideNOTMCluster;
    TList* m_histoList_THinCluster;
    TList* m_histoList_THinClusterTM;
    TList* m_histoList_GoodTHinClusterTM;
    TList* m_histoList_GoodTHinClusterTMGood;
    TList* m_graphList;

    /** Vectors used to compute the quantities depicted in Histos and Graphs */
    int m_NumberOfShaperDigit[m_Nsets] = {0};
    int m_NumberOfRecoDigit[m_Nsets] = {0};
    int m_NumberOfTH[m_Nsets] = {0};
    int m_NumberOfClustersRelatedToTH[m_Nsets] = {0};
    int m_NumberOfClusters[m_Nsets] = {0};
    int m_NumberOfTMClusters[m_Nsets] = {0};
    int m_NumberOfTMRecoInTMCluster = 0;
    int m_NumberOfTMRecoInNOTMCluster = 0;

    /** Control Histos and List to check if the function used to define a TH as "good" is working fine */
    TH1F* m_histoControl_MCcharge;
    TH1F* m_histoControl_MCisPrimary;
    TH1F* m_histoControl_THToMCsize;
    TList* m_histoList_Control;

  protected:

    /** Function returning the index used for Histos */
    int indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber);

    /** Function returning "Internal" or "External" depending on the index */
    TString IntExtFromIndex(int idx);

    /** Function returning "Forward" or "Backword" depending on the index */
    TString FWFromIndex(int idx);

    /** Function returning "U" or "V" depending on the index */
    TString UVFromIndex(int idx);

    /** Function returning a TH1F */
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList);

    /** Function returning TH2F */
    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX,
                            const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY,
                            const char* titleY, TList* histoList);

    /** Function returning a TGraph with Y axis limited to 1 given numerator and denumerator vectors and plotting the ratio and corresponding uncertainty */
    void createEfficiencyGraph(const char* name, const char* title, int vNum[m_Nsets], int vDen[m_Nsets], TString xTitle,
                               TString yTitle, TList* list);

    /** Function choosing between the two following functions depending on the length of the provided arrays */
    void createArbitraryGraphErrorChooser(const char* name, const char* title, float x[m_Nsets], float xErr[m_Nsets], float y[m_Nsets],
                                          float yErr[m_Nsets], TString xTitle, TString yTitle, TList* list, int len);

    /** Function returning an arbitrarly defined TGraph with arrays length equal to m_Nsets */
    void createArbitraryGraphError_Std(const char* name, const char* title, float x[m_Nsets], float xErr[m_Nsets], float y[m_Nsets],
                                       float yErr[m_Nsets], TString xTitle, TString yTitle, TList* list);

    /** Function returning an arbitrarly defined TGraph with arrays length equal to m_NsetsRed */
    void createArbitraryGraphError_Red(const char* name, const char* title, float x[m_NsetsRed], float xErr[m_NsetsRed],
                                       float y[m_NsetsRed], float yErr[m_NsetsRed], TString xTitle, TString yTitle, TList* list);

    /** Function defining if a TH is good (based on charge and primaryness) */
    bool goodTrueHit(const SVDTrueHit* thino);

  };
}


#endif


























