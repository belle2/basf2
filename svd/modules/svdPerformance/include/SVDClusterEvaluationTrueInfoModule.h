/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#ifndef SVDClusterEvaluationTrueInfo_H
#define SVDClusterEvaluationTrueInfo_H


#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <TList.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>


namespace Belle2 {

  /**
   * Clustering Performance, using true informations
   */

  class SVDClusterEvaluationTrueInfoModule : public Module {

  public:

    /** constructor */
    SVDClusterEvaluationTrueInfoModule();

    virtual ~SVDClusterEvaluationTrueInfoModule();

    /** Initialize the SVDClusterEvaluationTrueInfo */
    virtual void initialize() override;

    /** Called when entering a new run */
    virtual void beginRun() override;

    /** This method is the core of the SVDClusterEvaluationTrueInfo */
    virtual void event() override;

    /** This method is called if the current run ends */
    virtual void endRun() override;

    /** This method is called at the end of the event processing */
    virtual void terminate() override;


    static const int m_Nsets =
      6; /**< number of sets:  L3-barrel-U, L3-barrel-V, L456-barrel-U, L456-barrel-V, L456-slanted-U,  L456-slanted-V*/
    static const int m_NsetsRed = 3; /**< numbner of reduced sets*/

    //  private:
    TFile* m_outputFile = nullptr; /**< output file*/
    std::string m_outputFileName = ""; /**<output file name*/

    /** Name of the SVDEventInfo object */
    std::string m_svdEventInfoName;

    /** Index used for the lists and for the vectors of histograms: it indicates the set of sensors we are looking at */
    int indexForHistosAndGraphs = 0;

    /** Strings to pass names of the histos in the vectors of hitos */
    TString NameOfHisto = "";
    /** Strings to pass titles of the histos in the vectors of hitos */
    TString  TitleOfHisto = "";

    /** Vector of histograms depicting Strip Time Residuals */
    TH1F* m_histo_StripTimeResolution[m_Nsets] = {nullptr};
    /** Vector of histograms depicting Cluster Time Residuals */
    TH1F* m_histo_ClusterTimeResolution[m_Nsets] = {nullptr};
    /** Vector of histograms depicting Cluster Time Residuals, divided by TriggerBin */
    TH1F* m_histo_ClusterTimeResolution_bin1[m_Nsets] = {nullptr}; /**< TB1 */
    TH1F* m_histo_ClusterTimeResolution_bin2[m_Nsets] = {nullptr}; /**< TB2 */
    TH1F* m_histo_ClusterTimeResolution_bin3[m_Nsets] = {nullptr}; /**< TB3 */
    TH1F* m_histo_ClusterTimeResolution_bin4[m_Nsets] = {nullptr}; /**< TB4 */
    /** Vector of histograms depicting Cluster Time Pull */
    TH1F* m_histo_ClusterTimePull[m_Nsets] = {nullptr};
    /** Vector of histograms depicting Cluster U Position Residual (Reduced length!) */
    TH1F* m_histo_ClusterUPositionResolution[m_NsetsRed] = {nullptr};
    /** Vector of histograms depicting Cluster V Position Residual (Reduced length!) */
    TH1F* m_histo_ClusterVPositionResolution[m_NsetsRed] = {nullptr};
    /** Vector of histograms depicting Cluster U Position Pull (Reduced length!) */
    TH1F* m_histo_ClusterUPositionPull[m_NsetsRed] = {nullptr};
    /** Vector of histograms depicting Cluster U Position Pull (Reduced length!) */
    TH1F* m_histo_ClusterVPositionPull[m_NsetsRed] = {nullptr};
    /** Vector of 2D histograms depicting Time Residuals Vs Position (U/V) Residuals for Histos */
    TH2F* m_histo2D_TresVsPosres[m_Nsets] = {nullptr};
    /** Vector of histograms depicting Cluster Internal Purity (TM Recos over Reco inside a Cluster) */
    TH1F* m_histo_PurityInsideTMCluster[m_Nsets] = {nullptr};
    /** Vector of 2D histograms depicting TM Reco Vs Total Reco inside a TM Cluster */
    TH2F* m_histo2D_PurityInsideTMCluster[m_Nsets] = {nullptr};
    /** Vector of histograms depicting TM Cluster Internal Purity (TM Recos over Reco inside a Cluster) */
    TH1F* m_histo_PurityInsideNOTMCluster[m_Nsets] = {nullptr};
    /** Vector of histograms depicting Number of TH inside a Cluster */
    TH1F* m_histo_THinCluster[m_Nsets] = {nullptr};
    /** Vector of histograms depicting Number of TH inside a TM Cluster */
    TH1F* m_histo_THinClusterTM[m_Nsets] = {nullptr};
    /** Vector of histograms depicting Number of Good TH inside a TM Cluster */
    TH1F* m_histo_GoodTHinClusterTM[m_Nsets] = {nullptr};
    /** Vector of histograms depicting Number of Good TH inside a Good TM Cluster */
    TH1F* m_histo_GoodTHinClusterTMGood[m_Nsets] = {nullptr};

    /** Vectors of floats containing the mean and the RMS from the corresponding histo */
    float m_mean_StripTimeResolution[m_Nsets] = {0}; /**<average strip time residual*/
    float m_RMS_StripTimeResolution[m_Nsets] = {0}; /**<rms of strip time residual*/
    float m_mean_ClusterTimeResolution[m_Nsets] = {0}; /**<average cl time resid*/
    float m_RMS_ClusterTimeResolution[m_Nsets] = {0}; /**<rms cluster time resid*/
    float m_mean_ClusterUPositionResolution[m_Nsets] = {0}; /**<average cl U position resol*/
    float m_RMS_ClusterUPositionResolution[m_Nsets] = {0}; /**<rms cl U position resol*/
    float m_mean_ClusterVPositionResolution[m_Nsets] = {0}; /**<average cl V position reosl*/
    float m_RMS_ClusterVPositionResolution[m_Nsets] = {0}; /**<rms cl V position resol*/
    float m_mean_PurityInsideTMCluster[m_Nsets] = {0}; /**<cluster purity average*/
    float m_RMS_PurityInsideTMCluster[m_Nsets] = {0}; /**<cluster purity rms*/
    float m_mean_THinCluster[m_Nsets] = {0}; /**<true hits in cluster average*/
    float m_RMS_THinCluster[m_Nsets] = {0};/**<true hits in cluster rms*/
    float m_mean_THinClusterTM[m_Nsets] = {0}; /**<true hits in truth matched cluster average*/
    float m_RMS_THinClusterTM[m_Nsets] = {0}; /**<true hits in truth matched cluster rms*/
    float m_mean_GoodTHinClusterTM[m_Nsets] = {0}; /**<good true hits in cluster truth matched average*/
    float m_RMS_GoodTHinClusterTM[m_Nsets] = {0}; /**<good true hits in cluster truth matched rms*/
    float m_mean_GoodTHinClusterTMGood[m_Nsets] = {0}; /**<good true hits in cluster truth match good average*/
    float m_RMS_GoodTHinClusterTMGood[m_Nsets] = {0}; /**<good true hits in cluster truth match good rms*/

    /** Vectors used to Draw the TGraphs (defined in the cc) depicting the averages and the means of the histos above for the various sets of sensor*/
    float m_OrderingVec[m_Nsets] = {1., 2., 3., 4., 5., 6.};
    float m_NullVec[m_Nsets] = {0.}; /**< null vector*/

    /** Lists used to easily Draw the corresponding histos; last one is used to draw the TGraphs */
    TList* m_histoList_StripTimeResolution = nullptr; /**< histo list strip time resolution*/
    TList* m_histoList_ClusterTimeResolution = nullptr; /**< histo list cluster time resolution*/
    TList* m_histoList_ClusterTimePull = nullptr; /**< histo list cluster time pull*/
    TList* m_histoList_ClusterPositionResolution = nullptr; /**< histo list cluster position resolution*/
    TList* m_histoList_ClusterPositionPull = nullptr; /**< histo list cluster position pull*/
    TList* m_histo2DList_TresVsPosres = nullptr; /**< histo list ime tresol VS position resol*/
    TList* m_histoList_PurityInsideTMCluster = nullptr; /**< histo list truth matched cluster purity (2D)*/
    TList* m_histo2DList_PurityInsideTMCluster = nullptr; /**< histo list truth matched cluster purity (2D)*/
    TList* m_histoList_PurityInsideNOTMCluster = nullptr; /**< histo list not truth matched cluster purity*/
    TList* m_histoList_THinCluster = nullptr; /**< histo list true hits in cluster*/
    TList* m_histoList_THinClusterTM = nullptr; /**< histo list true hits in clsuter truth match*/
    TList* m_histoList_GoodTHinClusterTM = nullptr; /**< histo list good true hits in cluster truth matched*/
    TList* m_histoList_GoodTHinClusterTMGood = nullptr; /**< histo list goo true hits in cluster truth match good*/
    TList* m_graphList = nullptr; /**< histo list, TGraphs*/

    /** Vectors used to compute the quantities depicted in Histos and Graphs */
    int m_NumberOfShaperDigit[m_Nsets] = {0};
    int m_NumberOfRecoDigit[m_Nsets] = {0}; /**< number of reco digits*/
    int m_NumberOfTH[m_Nsets] = {0}; /**<  numner of true hits*/
    int m_NumberOfClustersRelatedToTH[m_Nsets] = {0};  /**< number of clusters related to true hits*/
    int m_NumberOfClusters[m_Nsets] = {0}; /**< number of clusters*/
    int m_NumberOfTMClusters[m_Nsets] = {0}; /**< number of truth matched clusters*/
    int m_NumberOfTMRecoInTMCluster = 0; /**<numnber of true match reco digit in truth match cluster*/
    int m_NumberOfTMRecoInNOTMCluster = 0; /**<number of truth matched reco digits in not truth matched clusters*/

    /** Control Histos and List to check if the function used to define a TH as "good" is working fine */
    TH1F* m_histoControl_MCcharge = nullptr; /**<control histo: MC charge*/
    TH1F* m_histoControl_MCisPrimary = nullptr; /**<control histo: MC is primary*/
    TH1F* m_histoControl_THToMCsize = nullptr; /**<control histo: true hit to mc size*/
    TList* m_histoList_Control = nullptr; /**<control histo*/


  protected:
    /** Storage for SVDEventInfo object */
    StoreObjPtr<SVDEventInfo> m_storeSVDEvtInfo;

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


























