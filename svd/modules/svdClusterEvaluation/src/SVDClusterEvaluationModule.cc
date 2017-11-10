/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdClusterEvaluation/SVDClusterEvaluationModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <TFile.h>
#include <TText.h>
#include <TH1F.h>

#include <string>
#include "TMath.h"
#include <algorithm>
#include <functional>


using namespace Belle2;


REG_MODULE(SVDClusterEvaluation)


SVDClusterEvaluationModule::SVDClusterEvaluationModule() : Module()
{
  setDescription("This modules generates a TTree containing the hit profile for sensor 6, test beam.");

  addParam("outputFileName", m_outputFileName, "output rootfile name", std::string("ClusterEvaluation.root"));
  //addParam("RMS_overPeak_Average", m_RMS_overPeak_Average, "RMS threshold to cut away peak-signals from noisy strip - both sides", float(18.0));
}


SVDClusterEvaluationModule::~SVDClusterEvaluationModule()
{
}


void SVDClusterEvaluationModule::initialize()
{

  /* initialize of useful store array */
  StoreArray<SVDShaperDigit> SVDShaperDigits;
  StoreArray<SVDRecoDigit> SVDRecoDigits;
  StoreArray<SVDCluster> SVDClusters;
  StoreArray<SVDTrueHit> SVDTrueHits;

  SVDShaperDigits.isRequired();
  SVDRecoDigits.isRequired();
  SVDClusters.isRequired();
  SVDTrueHits.isRequired();

  m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");

  m_histoList_ClusterPositionResolution = new TList;
  m_histoList_StripTimeResolution = new TList;
  m_histoList_ClusterTimeResolution = new TList;
  m_histoList_PurityInsideCluster = new TList;
  m_histoList_Puddlyness = new TList;
  m_graphList = new TList;

  for (int i = 0; i < m_Nsets; i ++) {

    if (i % 2 == 0) { //even index, U side
      NameOfHisto = "histo_ClusterUPositionResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i);
      TitleOfHisto = "Cluster U Position Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ")";
      m_histo_ClusterUPositionResolution[i / 2] = createHistogram1D(NameOfHisto, TitleOfHisto, 200, -0.1, 0.1,
                                                  "U_Reconstructed - U_TrueHit",
                                                  m_histoList_ClusterPositionResolution);
    } else { //odd index, V side
      NameOfHisto = "histo_ClusterVPositionResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i);
      TitleOfHisto = "Cluster V Position Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ")";
      m_histo_ClusterVPositionResolution[(i - 1) / 2] = createHistogram1D(NameOfHisto, TitleOfHisto, 200, -0.1, 0.1,
                                                        "V_Reconstructed - V_TrueHit", m_histoList_ClusterPositionResolution);
    }

    //
    NameOfHisto = "histo_StripTimeResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Strip Time Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    m_histo_StripTimeResolution[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, -100, 100, "t_Reconstructed - t_TrueHit",
                                                       m_histoList_StripTimeResolution);

    NameOfHisto = "histo_ClusterTimeResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Cluster Time Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    m_histo_ClusterTimeResolution[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, -100, 100, "t_Reconstructed - t_TrueHit",
                                                         m_histoList_ClusterTimeResolution);

    NameOfHisto = "histo_PurityInsideCluster_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Fraction of Truth-matched Recos inside a Truth-matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histo_PurityInsideCluster[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 101, 0, 1.01, "number of TM recos / cluster size",
                                                       m_histoList_PurityInsideCluster);

    NameOfHisto = "m_histoList_Puddlyness_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Number of True Hits inside a Truth-matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histo_Puddlyness[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 10, 0 , 10, "number of TH per cluster",
                                              m_histoList_Puddlyness);
  }

}


void SVDClusterEvaluationModule::beginRun()
{
}


void SVDClusterEvaluationModule::event()
{

  StoreArray<SVDShaperDigit> SVDShaperDigits;
  StoreArray<SVDRecoDigit> SVDRecoDigits;
  StoreArray<SVDCluster> SVDClusters;
  StoreArray<SVDTrueHit> SVDTrueHits;

  //////////
  //STRIPS
  //////////

  //loop on ShaperDigits
  for (const SVDShaperDigit& shape : SVDShaperDigits) {
    indexForHistosAndGraphs = indexFromLayerSensorSide(shape.getSensorID().getLayerNumber() , shape.getSensorID().getSensorNumber() ,
                                                       shape.isUStrip());

    RelationVector<SVDRecoDigit> relatVectorShaperToReco = DataStore::getRelationsWithObj<SVDRecoDigit>(&shape);

    //efficiency shaper to reco
    m_NumberOfShaperDigit[indexForHistosAndGraphs] ++;
    if (relatVectorShaperToReco.size() > 0)
      m_NumberOfRecoDigit[indexForHistosAndGraphs] ++;
  } //close loop on ShaperDigits

  //loop on RecoDigits
  for (const SVDRecoDigit& reco : SVDRecoDigits) {
    indexForHistosAndGraphs = indexFromLayerSensorSide(reco.getSensorID().getLayerNumber() , reco.getSensorID().getSensorNumber() ,
                                                       reco.isUStrip());

    RelationVector<SVDTrueHit> relatVectorRecoToTH = DataStore::getRelationsWithObj<SVDTrueHit>(&reco);

    //strip time resolution
    if (relatVectorRecoToTH.size() > 0)
      m_histo_StripTimeResolution[indexForHistosAndGraphs]->Fill(reco.getTime() - (relatVectorRecoToTH[0])->getGlobalTime());

  } //close loop on RecoDigits

  //////////
  //CLUSTERS
  //////////

  //loop on TrueHits
  for (const SVDTrueHit& trhi : SVDTrueHits) {
    indexForHistosAndGraphs = indexFromLayerSensorSide(trhi.getSensorID().getLayerNumber() , trhi.getSensorID().getSensorNumber() , 1);

    RelationVector<SVDCluster> relatVectorTHToClus = DataStore::getRelationsWithObj<SVDCluster>(&trhi);

    //efficiencies TH to cluster
    m_NumberOfTH[indexForHistosAndGraphs] ++; //U
    m_NumberOfTH[indexForHistosAndGraphs + 1] ++; //V
    for (int j = 0; j < (int) relatVectorTHToClus.size(); j ++) {
      indexForHistosAndGraphs = indexFromLayerSensorSide(relatVectorTHToClus[j]->getSensorID().getLayerNumber() ,
                                                         relatVectorTHToClus[j]->getSensorID().getSensorNumber() , relatVectorTHToClus[j]->isUCluster());
      m_NumberOfClustersRelatedToTH[indexForHistosAndGraphs] ++;
    }
  } //close loop on TrueHits

  //loop on Clusters
  for (const SVDCluster& clus : SVDClusters) {
    indexForHistosAndGraphs = indexFromLayerSensorSide(clus.getSensorID().getLayerNumber() , clus.getSensorID().getSensorNumber() ,
                                                       clus.isUCluster());

    RelationVector<SVDTrueHit> relatVectorClusToTH = DataStore::getRelationsWithObj<SVDTrueHit>(&clus);

    //purity "outside" clusters
    m_NumberOfClusters[indexForHistosAndGraphs] ++;
    if (relatVectorClusToTH.size() > 0)
      m_NumberOfTMClusters[indexForHistosAndGraphs] ++;

    //fill the puddlyness histo with the number of TH a cluster is composed of
    m_histo_Puddlyness[indexForHistosAndGraphs]->Fill(relatVectorClusToTH.size());

    for (int q = 0; q < (int)relatVectorClusToTH.size(); q ++) { //loop on the TH related to the cluster
      //cluster time resolution
      m_histo_ClusterTimeResolution[indexForHistosAndGraphs]->Fill(clus.getClsTime() - (relatVectorClusToTH[q])->getGlobalTime());

      //cluster position resolution
      if (clus.isUCluster()) {
        m_histo_ClusterUPositionResolution[indexForHistosAndGraphs / 2]->Fill(clus.getPosition((relatVectorClusToTH[q])->getV()) -
            (relatVectorClusToTH[q])->getU());
      } else {
        m_histo_ClusterVPositionResolution[(indexForHistosAndGraphs - 1) / 2]->Fill(clus.getPosition() -
            (relatVectorClusToTH[q])->getV());
      }
    }

    //count number of recodigit, composing the Truth-matched cluster, that are linked with a TH
    if (relatVectorClusToTH.size() > 0) {
      RelationVector<SVDRecoDigit> relatVectorClusToReco = DataStore::getRelationsWithObj<SVDRecoDigit>(&clus);

      m_NumberOfTMRecoInTMCluster = 0;
      for (int k = 0; k < (int)relatVectorClusToReco.size(); k++) { //loop on the recodigits composing the cluster
        RelationVector<SVDTrueHit> relatVectorRecoFromClusToTH = DataStore::getRelationsWithObj<SVDTrueHit>(relatVectorClusToReco[k]);

        if (relatVectorRecoFromClusToTH.size() > 0)
          m_NumberOfTMRecoInTMCluster ++;
      }

      m_histo_PurityInsideCluster[indexForHistosAndGraphs]->Fill(m_NumberOfTMRecoInTMCluster / (clus.getSize()));
    }
  } //close loop on clusters

}


void SVDClusterEvaluationModule::endRun()
{

  createEfficiencyGraph("recoEff", "fraction of fitted ShaperDigits", m_NumberOfRecoDigit, m_NumberOfShaperDigit, "set", "efficiency",
                        m_graphList);

  createEfficiencyGraph("clusterEff", "fraction of clusters related to TrueHits", m_NumberOfClustersRelatedToTH, m_NumberOfTH, "set",
                        "efficiency", m_graphList);

  createEfficiencyGraph("extenralPur", "fraction of truth matched clusters", m_NumberOfTMClusters, m_NumberOfClusters, "set",
                        "purity", m_graphList);


  //urait de uorld

  if (m_outputFile != NULL) {
    m_outputFile->cd();

    TDirectory* oldDir = gDirectory;
    TObject* obj;

    TDirectory* dir_strtime = oldDir->mkdir("strip_time");
    dir_strtime->cd();
    TIter nextH_strtime(m_histoList_StripTimeResolution);
    while ((obj = nextH_strtime()))
      obj->Write();

    TDirectory* dir_cltime = oldDir->mkdir("cluster_time");
    dir_cltime->cd();
    TIter nextH_cltime(m_histoList_ClusterTimeResolution);
    while ((obj = nextH_cltime()))
      obj->Write();

    TDirectory* dir_clpos = oldDir->mkdir("cluster_position");
    dir_clpos->cd();
    TIter nextH_clpos(m_histoList_ClusterPositionResolution);
    while ((obj = nextH_clpos()))
      obj->Write();

    TDirectory* dir_clinpur = oldDir->mkdir("intra_cluster_purity");
    dir_clinpur->cd();
    TIter nextH_clinpur(m_histoList_PurityInsideCluster);
    while ((obj = nextH_clinpur()))
      obj->Write();

    TDirectory* dir_puddle = oldDir->mkdir("trueHits_in_cluster");
    dir_puddle->cd();
    TIter nextH_puddle(m_histoList_Puddlyness);
    while ((obj = nextH_puddle()))
      obj->Write();

    TDirectory* dir_graph = oldDir->mkdir("graphs");
    dir_graph->cd();
    TIter nextH_graph(m_graphList);
    while ((obj = nextH_graph()))
      obj->Write();


    m_outputFile->Close();
  }
}


void SVDClusterEvaluationModule::terminate()
{
}


//Extra functions

TH1F* SVDClusterEvaluationModule::createHistogram1D(const char* name, const char* title,
                                                    Int_t nbins, Double_t min, Double_t max,
                                                    const char* xtitle, TList* histoList)
{
  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}

int SVDClusterEvaluationModule::indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber)
{
  int Index;

  if (LayerNumber == 3) { //internal
    if (UVNumber) //U
      Index = 0;
    else //V
      Index = 1;
  } else { //external
    if (SensorNumber == 1) { //FW
      if (UVNumber) //U
        Index = 2;
      else //V
        Index = 3;
    } else { //barrel
      if (UVNumber) //U
        Index = 4;
      else //V
        Index = 5;
    }
  }

  return Index;
}

TString SVDClusterEvaluationModule::IntExtFromIndex(int idx)
{
  TString name = "";

  if (idx < 2)
    name = "Layer3";
  else
    name = "Layers456";

  return name;
}

TString SVDClusterEvaluationModule::FWFromIndex(int idx)
{
  TString name = "";

  if (idx == 2 || idx == 3)
    name = "FWD";
  else
    name = "Barrel";

  return name;
}

TString SVDClusterEvaluationModule::UVFromIndex(int idx)
{
  TString name = "";

  if (idx % 2 == 0)
    name = "U";
  else
    name = "V";

  return name;
}

void  SVDClusterEvaluationModule::createEfficiencyGraph(const char* name, const char* title, int vNum[m_Nsets], int vDen[m_Nsets],
                                                        TString xTitle, TString yTitle, TList* list)
{

  float ratio[m_Nsets];
  float ratioErr[m_Nsets];
  float x[m_Nsets];
  float xErr[m_Nsets];

  for (int set = 0; set < m_Nsets; set++) {

    x[set] = set + 1;
    xErr[set] = 0;

    if (vDen[set] > 0) {
      ratio[set] = (float)vNum[set] / (float)vDen[set];
      ratioErr[set] = sqrt(ratio[set] * (1 - ratio[set])) / sqrt(vDen[set]);
    }

  }

  TCanvas* c = new TCanvas();
  TGraphErrors* g = new TGraphErrors(m_Nsets, x, ratio, xErr, ratioErr);
  g->SetName(name);
  g->SetTitle(title);
  g->GetXaxis()->SetTitle(xTitle.Data());
  g->GetYaxis()->SetTitle(yTitle.Data());
  g->GetYaxis()->SetRangeUser(0.00001, 1.01);
  g->Draw();
  TAxis* xAxis = g->GetXaxis();

  TText* t = new TText();
  t->SetTextAlign(32);
  t->SetTextSize(0.035);
  t->SetTextFont(72);
  TString labels[m_Nsets] = {"L3 U", "L3 V", "L456 FW U", "L456 FW V", "L456 BAR U", "L456 BAR V"};
  for (Int_t i = 0; i < m_Nsets; i++) {
    xAxis->SetBinLabel(i + 1, labels[i].Data());
  }

  if (list)
    list->Add(c);

}



