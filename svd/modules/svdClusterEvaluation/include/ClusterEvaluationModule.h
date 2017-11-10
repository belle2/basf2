/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ClusterEvaluation_H
#define ClusterEvaluation_H


#include <framework/core/Module.h>
#include <TTree.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <TList.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/geometry/SensorInfo.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TLegend.h>


namespace Belle2 {

  /*
  */

  class ClusterEvaluationModule : public Module {
  public:

    ClusterEvaluationModule();

    virtual ~ClusterEvaluationModule();

    /** Initialize the ClusterEvaluation.
     * This method is called only once before the actual event processing starts.   */
    virtual void initialize() override;

    /** Called when entering a new run.     */
    virtual void beginRun() override;

    /** This method is the core of the ClusterEvaluationle.
     * This method is called for each event. All processing of the event has to take place in this method.   */
    virtual void event() override;

    /** This method is called if the current run ends. */
    virtual void endRun() override;

    /** This method is called at the end of the event processing.   */
    virtual void terminate() override;


  private:
    TTree* m_tree; /**< full output tree */
    /*
    TGraph** gr_U_bkg;
    TGraph** gr_U_signal;
    TGraph** gr_U_peak;
    TGraph** gr_V_bkg;
    TGraph** gr_V_signal;
    TGraph** gr_V_peak;

    TCanvas* ca_U_bkg;
    TCanvas* ca_U_signal;
    TCanvas* ca_U_peak;
    TCanvas* ca_V_bkg;
    TCanvas* ca_V_signal;
    TCanvas* ca_V_peak;

    TLegend* OmniLegend_U_bkg;
    TLegend* OmniLegend_U_signal;
    TLegend* OmniLegend_U_peak;
    TLegend* OmniLegend_V_bkg;
    TLegend* OmniLegend_V_signal;
    TLegend* OmniLegend_V_peak;
    */
    ///

    int indexForHistosAndGraphs;

    TH1F* histo_ClusterUPositionResolution[3];
    TH1F* histo_ClusterVPositionResolution[3];
    TH1F* histo_StripTimeResolution[6];
    TH1F* histo_ClusterTimeResolution[6];
    TH1F* histo_PurityInsideCluster[6];
    TH1F* histo_Puddlyness[6];

    TString NameOfHisto, TitleOfHisto;

    TList m_histoList_ClusterUPositionResolution;
    TList m_histoList_ClusterVPositionResolution;
    TList m_histoList_StripTimeResolution;
    TList m_histoList_ClusterTimeResolution;
    TList m_histoList_PurityInsideCluster;
    TList m_histoList_Puddlyness;

    int m_NumberOfShaperDigit[6] = {0};
    int m_NumberOfRecoDigit[6] = {0};
    float m_EfficiencyShaperToReco[6]; //(??)

    int m_NumberOfTH[6] = {0};
    int m_NumberOfClustersRelatedToTH[6] = {0};
    float ?? [6]; //(??)

    int m_NumberOfClusters[6] = {0};
    int m_NumberOfTMClusters[6] = {0};
    float ?? [6]; //(??)

    int m_NumberOfTMRecoInTMCluster = 0;

  protected:

    int SVDPerformanceModule::indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber);

    TString SVDPerformanceModule::IntExtFromIndex(int idx);

    TString SVDPerformanceModule::FWFromIndex(int idx);

    TString SVDPerformanceModule::UVFromIndex(int idx);

    TH1F* SVDPerformanceModule::createHistogram1D(const char* name, const char* title,
                                                  Int_t nbins, Double_t min, Double_t max,
                                                  const char* xtitle, TList* histoList);
  };
}
#endif


























