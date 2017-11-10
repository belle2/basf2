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

    TH1F* m_histo_ClusterUPositionResolution[m_NsetsRed];
    TH1F* m_histo_ClusterVPositionResolution[m_NsetsRed];
    TH1F* m_histo_StripTimeResolution[m_Nsets];
    TH1F* m_histo_ClusterTimeResolution[m_Nsets];
    TH1F* m_histo_PurityInsideCluster[m_Nsets];
    TH1F* m_histo_Puddlyness[m_Nsets];

    TString NameOfHisto, TitleOfHisto;

    TList* m_histoList_StripTimeResolution;
    TList* m_histoList_ClusterTimeResolution;
    TList* m_histoList_ClusterPositionResolution;
    TList* m_histoList_PurityInsideCluster;
    TList* m_histoList_Puddlyness;
    TList* m_graphList;

    int m_NumberOfShaperDigit[m_Nsets] = {0};
    int m_NumberOfRecoDigit[m_Nsets] = {0};

    int m_NumberOfTH[m_Nsets] = {0};
    int m_NumberOfClustersRelatedToTH[m_Nsets] = {0};

    int m_NumberOfClusters[m_Nsets] = {0};
    int m_NumberOfTMClusters[m_Nsets] = {0};

    int m_NumberOfTMRecoInTMCluster = 0;

  protected:

    int indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber);

    TString IntExtFromIndex(int idx);

    TString FWFromIndex(int idx);

    TString UVFromIndex(int idx);

    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList);



    void createEfficiencyGraph(const char* name, const char* title, int vNum[m_Nsets], int vDen[m_Nsets], TString xTitle,
                               TString yTitle, TList* list);


  };
}


#endif


























