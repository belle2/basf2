/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/ClusterEvaluation/ClusterEvaluation.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <TFile.h>
#include <TH1F.h>

#include <string>
#include "TMath.h"
#include <algorithm>
#include <functional>


using namespace Belle2;


REG_MODULE(ClusterEvaluation)


ClusterEvaluationModule::ClusterEvaluationModule() : Module()
{
  setDescription("This modules generates a TTree containing the hit profile for sensor 6, test beam.");

  //addParam("numberOfCurvesToDraw", m_numberOfCurvesToDraw, "Number of curves to draw, the same for supposed Bkg and supposed Signal", 7);
  //addParam("RMS_overPeak_Average", m_RMS_overPeak_Average, "RMS threshold to cut away peak-signals from noisy strip - both sides", float(18.0));
}


ClusterEvaluationModule::~ClusterEvaluationModule()
{
}


void ClusterEvaluationModule::initialize()
{
  /* initialize of useful store array */
  StoreArray<SVDShaperDigit> storeShaper("");

  storeShaper.isRequired();

  /** inizialize output TFile (ttree, with own-class (ClusterEvaluation) branch)
   * nb: is not possibile to completely access to entries of this tree using
   * external (out of basf2) scripts
   */
  m_outputFile = new TFile("TFile_ClusterEvaluation.root", "RECREATE");
  /*
  ca_U_bkg = new TCanvas("ca_U_bkg", "U background shape", 1200, 900);
  ca_U_signal = new TCanvas("ca_U_signal", "U signal shape", 1200, 900);
  ca_U_peak = new TCanvas("ca_U_peak", "U peak shape", 1200, 900);
  ca_V_bkg = new TCanvas("ca_V_bkg", "V background shape", 1200, 900);
  ca_V_signal = new TCanvas("ca_V_signal", "V signal shape", 1200, 900);
  ca_V_peak = new TCanvas("ca_V_peak", "V peak shape", 1200, 900);

  gr_U_bkg = new TGraph*[m_numberOfCurvesToDraw];
  gr_U_signal = new TGraph*[m_numberOfCurvesToDraw];
  gr_U_peak = new TGraph*[m_numberOfCurvesToDraw];
  gr_V_bkg = new TGraph*[m_numberOfCurvesToDraw];
  gr_V_signal = new TGraph*[m_numberOfCurvesToDraw];
  gr_V_peak = new TGraph*[m_numberOfCurvesToDraw];

  OmniLegend_U_bkg = new TLegend(0.8,0.4,0.9,0.1); //x1, y1, x2, y2?
  OmniLegend_U_signal = new TLegend(0.8,0.4,0.9,0.1); //x1, y1, x2, y2?
  OmniLegend_U_peak = new TLegend(0.8,0.4,0.9,0.1); //x1, y1, x2, y2?
  OmniLegend_V_bkg = new TLegend(0.8,0.4,0.9,0.1); //x1, y1, x2, y2?
  OmniLegend_V_signal = new TLegend(0.8,0.4,0.9,0.1); //x1, y1, x2, y2?
  OmniLegend_V_peak = new TLegend(0.8,0.4,0.9,0.1); //x1, y1, x2, y2?
  */

  for (int i = 0; i < 6; i ++) {
    if (i % 2 == 0) { //even index, U side
      NameOfHisto = "histo_ClusterUPositionResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i);
      TitleOfHisto = "Cluster U Position Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ")";
      histo_ClusterUPositionResolution[i / 2] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, -42, 42, "U_Reconstructed - U_TrueHit",
                                                                  m_histoList_ClusterUPositionResolution);
    } else { //odd index, V side
      NameOfHisto = "histo_ClusterVPositionResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i);
      TitleOfHisto = "Cluster V Position Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ")";
      histo_ClusterVPositionResolution[(i - 1) / 2] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, -42, 42,
                                                      "V_Reconstructed - V_TrueHit", m_histoList_ClusterVPositionResolution);
    }

    NameOfHisto = "histo_StripTimeResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Strip Time Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    histo_StripTimeResolution[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, -42, 42, "t_Reconstructed - t_TrueHit",
                                                     m_histoList_StripTimeResolution);

    NameOfHisto = "histo_ClusterTimeResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Cluster Time Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    histo_ClusterTimeResolution[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, -42, 42, "t_Reconstructed - t_TrueHit",
                                                       m_histoList_ClusterTimeResolution);

    NameOfHisto = "histo_PurityInsideCluster_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Fraction of Truth-matched Recos inside a Truth-matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    histo_PurityInsideCluster[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, -42, 42, "number of TM recos / cluster size",
                                                     m_histoList_PurityInsideCluster);

    NameOfHisto = "m_histoList_Puddlyness_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Number of True Hits inside a Truth-matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histoList_Puddlyness[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 500, -42, 42, "number of TH per cluster",
                                                  m_histoList_Puddlyness);
  }
}


void ClusterEvaluationModule::beginRun()
{
}


void ClusterEvaluationModule::event()
{
  StoreArray<SVDShaperDigit> SVDShaperDigits;
  StoreArray<SVDCluster> SVDClusters;

  //STRIPS

  for (const SVDShaperDigit& shape : SVDShaperDigits) { //cycle on SVDShaperDigit
    indexForHistosAndGraphs = indexFromLayerSensorSide(shape.getSensorID().getLayerNumber() , shape.getSensorID().getSensorNumber() ,
                                                       shape.isUStrip());

    RelationVector<SVDRecoDigit> relatVectorShaperToReco = DataStore::getRelationsWithObj<SVDRecoDigit>(&shape);

    //efficiency shaper to reco
    m_NumberOfShaperDigit[indexForHistosAndGraphs] ++;
    if (relatVectorShaperToReco.size() > 0)
      m_NumberOfRecoDigit[indexForHistosAndGraphs] ++;
  }

  for (const SVDRecoDigit& reco : SVDRecoDigits) { //cycle on SVDRecoDigit
    indexForHistosAndGraphs = indexFromLayerSensorSide(reco.getSensorID().getLayerNumber() , reco.getSensorID().getSensorNumber() ,
                                                       reco.isUStrip());

    RelationVector<SVDTrueHit> relatVectorRecoToTH = DataStore::getRelationsWithObj<SVDTrueHit>(&reco);

    //strip time resolution
    histo_StripTimeResolution[indexForHistosAndGraphs]->Fill(reco.getTime() - (relatVectorRecoToTH[0]).getGlobalTime());
  }

  //CLUSTERS

  for (const SVDTrueHit& trhi : SVDTrueHits) { //cycle on SVDTrueHit
    indexForHistosAndGraphs = indexFromLayerSensorSide(trhi.getSensorID().getLayerNumber() , trhi.getSensorID().getSensorNumber() , 1);

    RelationVector<SVDCluster> relatVectorTHToClus = DataStore::getRelationsWithObj<SVDCluster>(&trhi);

    //efficiencies TH to cluster
    m_NumberOfTH[indexForHistosAndGraphs] ++;
    m_NumberOfTH[indexForHistosAndGraphs + 1] ++;
    for (int j = 0; j < relatVectorTHToClus.size(); j ++) {
      indexForHistosAndGraphs = indexFromLayerSensorSide(relatVectorTHToClus[j].getSensorID().getLayerNumber() ,
                                                         relatVectorTHToClus[j].getSensorID().getSensorNumber() , relatVectorTHToClus[j].isUCluster());
      m_NumberOfClustersRelatedToTH[indexForHistosAndGraphs] ++;
    }
  }

  for (const SVDCluster& clus : SVDClusters) { //cycle on SVDCluster
    indexForHistosAndGraphs = indexFromLayerSensorSide(clus.getSensorID().getLayerNumber() , clus.getSensorID().getSensorNumber() ,
                                                       clus.isUCluster());

    RelationVector<SVDTrueHit> relatVectorClusToTH = DataStore::getRelationsWithObj<SVDTrueHit>(&clus);

    //purity "outside" clusters
    m_NumberOfClusters[indexForHistosAndGraphs] ++;
    if (relatVectorClusToTH.size() > 0)
      m_NumberOfTMClusters[indexForHistosAndGraphs] ++;

    //fill the puddlyness histo with the number of TH a cluster is composed of
    histo_Puddlyness[indexForHistosAndGraphs]->Fill(relatVectorClusToTH.size());

    for (int q = 0; q < relatVectorClusToTH.size(); q ++) { //cycle on the TH related to the cluster
      //cluster time resolution
      histo_ClusterTimeResolution[indexForHistosAndGraphs]->Fill(clus.getTime() - (relatVectorClusToTH[q]).getGlobalTime());

      //cluster position resolution
      if (clus.isUCluster()) {
        histo_ClusterUPositionResolution[indexForHistosAndGraphs / 2]->Fill(clus.getPosition() -
            (relatVectorClusToTH[q]).getEntryU() /*Entry? Exit? Average? Other? (??)*/);
      } else {
        histo_ClusterUPositionResolution[(indexForHistosAndGraphs - 1) / 2]->Fill(clus.getPosition() -
            (relatVectorClusToTH[q]).getEntryV() /*Entry? Exit? Average? Other? (??)*/);
      }
    }

    //count number of recodigit, composing the Truth-matched cluster, that are linked with a TH (??)
    if (relatVectorClusToTH.size() > 0) {
      RelationVector<SVDRecoDigit> relatVectorClusToReco = DataStore::getRelationsWithObj<SVDRecoDigit>(&clus);

      m_NumberOfTMRecoInTMCluster = 0;
      for (int k = 0; k < relatVectorClusToReco.size(); k++) { //cycle on the recodigits composing the cluster
        RelationVector<SVDTrueHit> relatVectorRecoFromClusToTH = DataStore::getRelationsWithObj<SVDTrueHit>(&(relatVectorClusToReco[k]));

        if (relatVectorRecoFromClusToTH.size() > 0)
          m_NumberOfTMRecoInTMCluster ++;
      }

      histo_PurityInsideCluster[indexForHistosAndGraphs]->Fill(m_NumberOfTMRecoInTMCluster / (clus.getSize()));
    }
  }
}


void ClusterEvaluationModule::endRun()
{
  for (int k = 0; k < 6; k ++) {
    m_EfficiencyShaperToReco[k] = (float)m_NumberOfRecoDigit[k] / (float)m_NumberOfShaperDigit[k]; //(??)
    // (??)
  }

  m_outputFile->cd();

  //urait de uorld

  m_outputFile->Close();
}


void ClusterEvaluationModule::terminate()
{
}


//Extra functions

TH1F* SVDPerformanceModule::createHistogram1D(const char* name, const char* title,
                                              Int_t nbins, Double_t min, Double_t max,
                                              const char* xtitle, TList* histoList)
{
  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}

int SVDPerformanceModule::indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber)
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
    } else { //barrell
      if (UVNumber) //U
        Index = 4;
      else //V
        Index = 5;
    }
  }

  return Index;
}

TString SVDPerformanceModule::IntExtFromIndex(int idx)
{
  TString name = "";

  if (idx < 2)
    name = "Internal";
  else
    name = "External";

  return name;
}

TString SVDPerformanceModule::FWFromIndex(int idx)
{
  TString name = "";

  if (idx == 2 || idx == 3)
    name = "FW";
  else
    name = "Barrell";

  return name;
}

TString SVDPerformanceModule::UVFromIndex(int idx)
{
  TString name = "";

  if (idx % 2 == 0)
    name = "U";
  else
    name = "V";

  return name;
}


































