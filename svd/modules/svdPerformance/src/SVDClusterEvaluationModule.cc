/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio, Giulia Casarosa                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdPerformance/SVDClusterEvaluationModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <TFile.h>
#include <TText.h>
#include <TH1F.h>
#include <TH2F.h>

#include <string>
#include "TMath.h"
#include <algorithm>
#include <functional>


using namespace Belle2;


REG_MODULE(SVDClusterEvaluation)


SVDClusterEvaluationModule::SVDClusterEvaluationModule() : Module()
{
  setDescription("This modules generates performance plots on SVD clustering.");

  addParam("outputFileName", m_outputFileName, "output rootfile name", std::string("SVDClusterEvaluation.root"));
}


SVDClusterEvaluationModule::~SVDClusterEvaluationModule()
{
}


void SVDClusterEvaluationModule::initialize()
{

  /* initialize useful store array */
  StoreArray<SVDShaperDigit> SVDShaperDigits;
  StoreArray<SVDRecoDigit> SVDRecoDigits;
  StoreArray<SVDCluster> SVDClusters;
  StoreArray<SVDTrueHit> SVDTrueHits;

  SVDShaperDigits.isRequired();
  SVDRecoDigits.isRequired();
  SVDClusters.isRequired();
  SVDTrueHits.isRequired();

  m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");

  m_histoList_StripTimeResolution = new TList;
  m_histoList_ClusterTimeResolution = new TList;
  m_histoList_ClusterTimePull = new TList;
  m_histoList_ClusterPositionResolution = new TList;
  m_histoList_ClusterPositionPull = new TList;
  m_histo2DList_TresVsPosres = new TList;
  m_histoList_PurityInsideTMCluster = new TList;
  m_histo2DList_PurityInsideTMCluster = new TList;
  m_histoList_PurityInsideNOTMCluster = new TList;
  m_histoList_THinCluster = new TList;
  m_histoList_THinClusterTM = new TList;
  m_histoList_GoodTHinClusterTM = new TList;
  m_histoList_GoodTHinClusterTMGood = new TList;
  m_graphList = new TList;
  //Control List
  m_histoList_Control = new TList;

  for (int i = 0; i < m_Nsets; i ++) {

    if (i % 2 == 0) { //even index, U side
      NameOfHisto = "histo_ClusterUPositionResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i);
      TitleOfHisto = "U-Cluster Position Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ")";
      m_histo_ClusterUPositionResolution[i / 2] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -0.1, 0.1,
                                                  "U_reco - U_true (cm)",
                                                  m_histoList_ClusterPositionResolution);

      NameOfHisto = "histo_ClusterUPositionPull_" + IntExtFromIndex(i) + "_" + FWFromIndex(i);
      TitleOfHisto = "U-Cluster Position Pull (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ")";
      m_histo_ClusterUPositionPull[i / 2] = createHistogram1D(NameOfHisto, TitleOfHisto, 210, -10, 11,
                                                              "(U_reco - U_true)/U_sigma",
                                                              m_histoList_ClusterPositionPull);
    } else { //odd index, V side
      NameOfHisto = "histo_ClusterVPositionResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i);
      TitleOfHisto = "V-Cluster Position Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ")";
      m_histo_ClusterVPositionResolution[(i - 1) / 2] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -0.1, 0.1,
                                                        "V_reco - V_true (cm)", m_histoList_ClusterPositionResolution);

      NameOfHisto = "histo_ClusterVPositionPull_" + IntExtFromIndex(i) + "_" + FWFromIndex(i);
      TitleOfHisto = "Cluster V Position Pull (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ")";
      m_histo_ClusterVPositionPull[(i - 1) / 2] = createHistogram1D(NameOfHisto, TitleOfHisto, 210, -10, 11,
                                                  "(V_reco- V_true)/V_sigma", m_histoList_ClusterPositionPull);
    }

    NameOfHisto = "histo_StripTimeResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Strip Time Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    m_histo_StripTimeResolution[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -100, 100, "t_reco - t_true (ns)",
                                                       m_histoList_StripTimeResolution);

    NameOfHisto = "histo_ClusterTimeResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Cluster Time Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    m_histo_ClusterTimeResolution[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -100, 100, "t_reco - t_true (ns)",
                                                         m_histoList_ClusterTimeResolution);

    NameOfHisto = "histo_ClusterTimeResolution_bin1_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Cluster Time Resolution TriggerBin=1(" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(
                     i) + ")";
    m_histo_ClusterTimeResolution_bin1[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -100, 100, "t_reco - t_true (ns)",
                                                              m_histoList_ClusterTimeResolution);
    NameOfHisto = "histo_ClusterTimeResolution_bin2_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Cluster Time Resolution TriggerBin=2(" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(
                     i) + ")";
    m_histo_ClusterTimeResolution_bin2[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -100, 100, "t_reco - t_true (ns)",
                                                              m_histoList_ClusterTimeResolution);
    NameOfHisto = "histo_ClusterTimeResolution_bin3_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Cluster Time Resolution TriggerBin=3(" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(
                     i) + ")";
    m_histo_ClusterTimeResolution_bin3[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -100, 100, "t_reco - t_true (ns)",
                                                              m_histoList_ClusterTimeResolution);
    NameOfHisto = "histo_ClusterTimeResolution_bin4_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Cluster Time Resolution TriggerBin=4(" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(
                     i) + ")";
    m_histo_ClusterTimeResolution_bin4[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -100, 100, "t_reco - t_true (ns)",
                                                              m_histoList_ClusterTimeResolution);

    NameOfHisto = "histo_ClusterTimePull_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Cluster Time Pull (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    m_histo_ClusterTimePull[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 210, -10, 11, "(t_reco - t_true)/t_sigma",
                                                   m_histoList_ClusterTimePull);

    NameOfHisto = "histo2D_TresVsPosres_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Time Residuals Vs U/V Position Residuals (" + IntExtFromIndex(
                     i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    m_histo2D_TresVsPosres[i] = createHistogram2D(NameOfHisto, TitleOfHisto, 200, -0.1, 0.1, "U/V_reco - U/V_true (cm)", 180, -120, 60,
                                                  "t_reco - t_true (ns)", m_histo2DList_TresVsPosres);

    NameOfHisto = "histo_PurityInsideTMCluster_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Fraction of Truth-Matched RecoDigits inside a Truth-Matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histo_PurityInsideTMCluster[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 110, 0, 1.10,
                                                         "number of TM recoDigits / cluster size",
                                                         m_histoList_PurityInsideTMCluster);

    NameOfHisto = "histo2D_PurityInsideTMCluster_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Number of Truth-matched Recos vs Number of Recos inside a Truth-matched Cluster (" + IntExtFromIndex(
                     i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    m_histo2D_PurityInsideTMCluster[i] = createHistogram2D(NameOfHisto, TitleOfHisto, 42, 0, 42, "cluster size", 42, 0, 42,
                                                           "number of TM recos", m_histo2DList_PurityInsideTMCluster);

    NameOfHisto = "histo_PurityInsideNOTMCluster_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Fraction of Truth-matched RecoDigits inside a NOT Truth-matched Cluster (" + IntExtFromIndex(
                     i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histo_PurityInsideNOTMCluster[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 110, 0, 1.10,
                                                           "number of TM recoDigits / cluster size",
                                                           m_histoList_PurityInsideNOTMCluster);

    NameOfHisto = "histo_THinCluster_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Number of True Hits inside a Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(
                     i) + ")";
    m_histo_THinCluster[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 15, 0 , 15, "number of TH per cluster",
                                               m_histoList_THinCluster);

    NameOfHisto = "histo_THinClusterTM_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Number of True Hits inside a Truth-matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histo_THinClusterTM[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 15, 0 , 15, "number of TH per TM cluster",
                                                 m_histoList_THinClusterTM);

    NameOfHisto = "histo_GoodTHinClusterTM_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Number of Good True Hits inside a Truth-matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histo_GoodTHinClusterTM[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 15, 0 , 15, "number of Good TH per TM cluster",
                                                     m_histoList_GoodTHinClusterTM);

    NameOfHisto = "histo_GoodTHinClusterTMGood_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Number of Good True Hits inside a Good Truth-matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histo_GoodTHinClusterTMGood[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 15, 0 , 15, "number of Good TH per Good TM cluster",
                                                         m_histoList_GoodTHinClusterTMGood);
  }

  //Control Histos
  m_histoControl_MCcharge = createHistogram1D("m_histoControl_MCcharge", "m_histoControl_MCcharge", 5, -2, 3,
                                              "charge of the first MC particle related to a True Hit", m_histoList_Control);
  m_histoControl_MCisPrimary = createHistogram1D("m_histoControl_MCisPrimary", "m_histoControl_MCisPrimary", 2, 0, 2,
                                                 "isPrimary of the first MC particle related to a True Hit", m_histoList_Control);
  m_histoControl_THToMCsize = createHistogram1D("m_histoControl_THToMCsize", "m_histoControl_THToMCsize", 10, -1, 9,
                                                "size of the THToMC relation arrau", m_histoList_Control);
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
  //STRIPS//
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
  }
  //close loop on ShaperDigits

  //loop on RecoDigits
  for (const SVDRecoDigit& reco : SVDRecoDigits) {
    indexForHistosAndGraphs = indexFromLayerSensorSide(reco.getSensorID().getLayerNumber() , reco.getSensorID().getSensorNumber() ,
                                                       reco.isUStrip());

    RelationVector<SVDTrueHit> relatVectorRecoToTH = DataStore::getRelationsWithObj<SVDTrueHit>(&reco);

    //strip time resolution
    if (relatVectorRecoToTH.size() > 0)
      m_histo_StripTimeResolution[indexForHistosAndGraphs]->Fill(reco.getTime() - (relatVectorRecoToTH[0])->getGlobalTime());

  }
  //close loop on RecoDigits

  ////////////
  //CLUSTERS//
  ////////////

  //loop on TrueHits
  for (const SVDTrueHit& trhi : SVDTrueHits) {

    if (goodTrueHit(&trhi)) { //enter only if the TH is related to a primary and charged MC particle
      indexForHistosAndGraphs = indexFromLayerSensorSide(trhi.getSensorID().getLayerNumber() , trhi.getSensorID().getSensorNumber() , 1);

      RelationVector<SVDCluster> relatVectorTHToClus = DataStore::getRelationsWithObj<SVDCluster>(&trhi);

      //efficiencies TH to cluster
      m_NumberOfTH[indexForHistosAndGraphs] ++; //U
      m_NumberOfTH[indexForHistosAndGraphs + 1] ++; //V

      bool hasU = false;
      bool hasV = false;

      for (int j = 0; j < (int) relatVectorTHToClus.size(); j ++) {
        indexForHistosAndGraphs = indexFromLayerSensorSide(relatVectorTHToClus[j]->getSensorID().getLayerNumber() ,
                                                           relatVectorTHToClus[j]->getSensorID().getSensorNumber() , relatVectorTHToClus[j]->isUCluster());

        if (relatVectorTHToClus[j]->isUCluster() && ! hasU) {
          m_NumberOfClustersRelatedToTH[indexForHistosAndGraphs] ++;
          hasU = true;
        } else if (!relatVectorTHToClus[j]->isUCluster() && ! hasV) {
          m_NumberOfClustersRelatedToTH[indexForHistosAndGraphs] ++;
          hasV = true;
        }
      }
    }
  }
  //close loop on TrueHits

  //loop on Clusters
  for (const SVDCluster& clus : SVDClusters) {
    indexForHistosAndGraphs = indexFromLayerSensorSide(clus.getSensorID().getLayerNumber() , clus.getSensorID().getSensorNumber() ,
                                                       clus.isUCluster());

    RelationVector<SVDTrueHit> relatVectorClusToTH = DataStore::getRelationsWithObj<SVDTrueHit>(&clus);

    //purity "outside" clusters
    m_NumberOfClusters[indexForHistosAndGraphs] ++;
    if (relatVectorClusToTH.size() > 0)
      m_NumberOfTMClusters[indexForHistosAndGraphs] ++;

    //fill the THinCluster histo with the number of TH a cluster is composed of
    m_histo_THinCluster[indexForHistosAndGraphs]->Fill(relatVectorClusToTH.size());

    //loop on the TH related to the cluster
    for (int q = 0; q < (int)relatVectorClusToTH.size(); q ++) {
      //cluster time resolution and pull
      m_histo_ClusterTimeResolution[indexForHistosAndGraphs]->Fill(clus.getClsTime() - (relatVectorClusToTH[q])->getGlobalTime());

      //get trigger bin
      int triggerBin = 0;
      RelationVector<SVDRecoDigit> relatVectorClusToRD = DataStore::getRelationsWithObj<SVDRecoDigit>(&clus);
      SVDModeByte modeByte = relatVectorClusToRD[0]->getModeByte();
      triggerBin = (int)modeByte.getTriggerBin();

      if (triggerBin == 0)
        m_histo_ClusterTimeResolution_bin1[indexForHistosAndGraphs]->Fill(clus.getClsTime() - (relatVectorClusToTH[q])->getGlobalTime());
      else if (triggerBin == 1)
        m_histo_ClusterTimeResolution_bin2[indexForHistosAndGraphs]->Fill(clus.getClsTime() - (relatVectorClusToTH[q])->getGlobalTime());
      else if (triggerBin == 2)
        m_histo_ClusterTimeResolution_bin3[indexForHistosAndGraphs]->Fill(clus.getClsTime() - (relatVectorClusToTH[q])->getGlobalTime());
      else if (triggerBin == 3)
        m_histo_ClusterTimeResolution_bin4[indexForHistosAndGraphs]->Fill(clus.getClsTime() - (relatVectorClusToTH[q])->getGlobalTime());
      else



        m_histo_ClusterTimePull[indexForHistosAndGraphs]->Fill((clus.getClsTime() - (relatVectorClusToTH[q])->getGlobalTime()) /
                                                               (clus.getClsTimeSigma()));

      //cluster position resolution and pull, also correlation between time res and position res
      if (clus.isUCluster()) {
        m_histo_ClusterUPositionResolution[indexForHistosAndGraphs / 2]->Fill(clus.getPosition((relatVectorClusToTH[q])->getV()) -
            (relatVectorClusToTH[q])->getU());
        m_histo_ClusterUPositionPull[indexForHistosAndGraphs / 2]->Fill((clus.getPosition((relatVectorClusToTH[q])->getV()) -
            (relatVectorClusToTH[q])->getU()) / (clus.getPositionSigma()));
        m_histo2D_TresVsPosres[indexForHistosAndGraphs]->Fill((clus.getPosition((relatVectorClusToTH[q])->getV()) -
                                                               (relatVectorClusToTH[q])->getU()) , (clus.getClsTime() - (relatVectorClusToTH[q])->getGlobalTime()));
      } else {
        m_histo_ClusterVPositionResolution[(indexForHistosAndGraphs - 1) / 2]->Fill(clus.getPosition() - (relatVectorClusToTH[q])->getV());
        m_histo_ClusterVPositionPull[(indexForHistosAndGraphs - 1) / 2]->Fill((clus.getPosition() - (relatVectorClusToTH[q])->getV()) /
            (clus.getPositionSigma()));
        m_histo2D_TresVsPosres[indexForHistosAndGraphs]->Fill((clus.getPosition() - (relatVectorClusToTH[q])->getV()) ,
                                                              (clus.getClsTime() - (relatVectorClusToTH[q])->getGlobalTime()));
      }
    }

    RelationVector<SVDRecoDigit> relatVectorClusToReco = DataStore::getRelationsWithObj<SVDRecoDigit>(&clus);
    //enter only if the cluster is TM
    if (relatVectorClusToTH.size() > 0) {

      //fill the THinCluster histo with the number of TH (and good TH) a TM cluster (and a Good TM cluster) is composed of
      m_histo_THinClusterTM[indexForHistosAndGraphs]->Fill(relatVectorClusToTH.size());
      int numberOfGoodTHInACluster = 0;
      int numberOfGoodTHInAClusterGood = 0;
      for (int k = 0; k < (int)(relatVectorClusToTH.size()); k ++) {
        if (goodTrueHit(relatVectorClusToTH[k])) {
          numberOfGoodTHInACluster ++;
          numberOfGoodTHInAClusterGood ++;
        }
      }
      m_histo_GoodTHinClusterTM[indexForHistosAndGraphs]->Fill(numberOfGoodTHInACluster);
      if (numberOfGoodTHInAClusterGood > 0)
        m_histo_GoodTHinClusterTMGood[indexForHistosAndGraphs]->Fill(numberOfGoodTHInAClusterGood);

      //count number of recodigit, composing the Truth-matched cluster, that are linked with a TH (internal purity)
      m_NumberOfTMRecoInTMCluster = 0;
      for (int k = 0; k < (int)relatVectorClusToReco.size(); k++) { //loop on the recodigits composing the TM cluster
        RelationVector<SVDTrueHit> relatVectorRecoFromClusToTH = DataStore::getRelationsWithObj<SVDTrueHit>(relatVectorClusToReco[k]);

        if (relatVectorRecoFromClusToTH.size() > 0)
          m_NumberOfTMRecoInTMCluster ++;
      }

      m_histo_PurityInsideTMCluster[indexForHistosAndGraphs]->Fill((float)m_NumberOfTMRecoInTMCluster / (float)(clus.getSize()));
      m_histo2D_PurityInsideTMCluster[indexForHistosAndGraphs]->Fill(clus.getSize(), m_NumberOfTMRecoInTMCluster);

    }
    //count number of recodigit, composing a NOT Truth-matched cluster, that are linked with a TH
    else {

      m_NumberOfTMRecoInNOTMCluster = 0;
      for (int k = 0; k < (int)relatVectorClusToReco.size(); k++) { //loop on the recodigits composing the NOTM cluster
        RelationVector<SVDTrueHit> relatVectorRecoFromClusToTH = DataStore::getRelationsWithObj<SVDTrueHit>(relatVectorClusToReco[k]);

        if (relatVectorRecoFromClusToTH.size() > 0)
          m_NumberOfTMRecoInNOTMCluster ++;
      }

      m_histo_PurityInsideNOTMCluster[indexForHistosAndGraphs]->Fill((float)m_NumberOfTMRecoInNOTMCluster / (float)(clus.getSize()));

    }
  }
  //close loop on clusters
}


void SVDClusterEvaluationModule::endRun()
{

  //extract mean and sigma values from histos to plot them in graphs
  for (int k = 0; k < m_Nsets; k ++) {
    m_mean_StripTimeResolution[k] = m_histo_StripTimeResolution[k]->GetMean();
    m_RMS_StripTimeResolution[k] = m_histo_StripTimeResolution[k]->GetRMS();

    m_mean_ClusterTimeResolution[k] = m_histo_ClusterTimeResolution[k]->GetMean();
    m_RMS_ClusterTimeResolution[k] = m_histo_ClusterTimeResolution[k]->GetRMS();

    m_mean_PurityInsideTMCluster[k] = m_histo_PurityInsideTMCluster[k]->GetMean();
    m_RMS_PurityInsideTMCluster[k] = m_histo_PurityInsideTMCluster[k]->GetRMS() / sqrt(m_histo_PurityInsideTMCluster[k]->GetEntries());

    m_mean_THinCluster[k] = m_histo_THinCluster[k]->GetMean();
    m_RMS_THinCluster[k] = m_histo_THinCluster[k]->GetRMS() / sqrt(m_histo_THinCluster[k]->GetEntries());

    m_mean_THinClusterTM[k] = m_histo_THinClusterTM[k]->GetMean();
    m_RMS_THinClusterTM[k] = m_histo_THinClusterTM[k]->GetRMS() / sqrt(m_histo_THinClusterTM[k]->GetEntries());

    m_mean_GoodTHinClusterTM[k] = m_histo_GoodTHinClusterTM[k]->GetMean();
    m_RMS_GoodTHinClusterTM[k] = m_histo_GoodTHinClusterTM[k]->GetRMS() / sqrt(m_histo_GoodTHinClusterTM[k]->GetEntries());

    m_mean_GoodTHinClusterTMGood[k] = m_histo_GoodTHinClusterTMGood[k]->GetMean();
    m_RMS_GoodTHinClusterTMGood[k] = m_histo_GoodTHinClusterTMGood[k]->GetRMS() / sqrt(m_histo_GoodTHinClusterTMGood[k]->GetEntries());
  }
  for (int k = 0; k < m_NsetsRed; k ++) {
    m_mean_ClusterUPositionResolution[k] = m_histo_ClusterUPositionResolution[k]->GetMean();
    m_RMS_ClusterUPositionResolution[k] = m_histo_ClusterUPositionResolution[k]->GetRMS();

    m_mean_ClusterVPositionResolution[k] = m_histo_ClusterVPositionResolution[k]->GetMean();
    m_RMS_ClusterVPositionResolution[k] = m_histo_ClusterVPositionResolution[k]->GetRMS();
  }

  //GRAPHS
  createEfficiencyGraph("recoEff", "Strip Fit Efficiency ( RecoDigits / ShaperDigits )", m_NumberOfRecoDigit, m_NumberOfShaperDigit,
                        "set", "efficiency", m_graphList);

  createEfficiencyGraph("clusterEff", "Clustering Efficiency ( Truth-Matched Clusters / TrueHits )", m_NumberOfClustersRelatedToTH,
                        m_NumberOfTH, "set", "efficiency", m_graphList);

  createEfficiencyGraph("clusterPurity", "Purity of Clusters ( Truth-Matched Clusters / All Clusters )", m_NumberOfTMClusters,
                        m_NumberOfClusters, "set", "purity", m_graphList);

  //means-from-histos graphs
  createArbitraryGraphErrorChooser("stripTime_Means", "Strip Time Resolution", m_OrderingVec, m_NullVec, m_mean_StripTimeResolution,
                                   m_RMS_StripTimeResolution, "set", "time residuals (ns)", m_graphList, m_Nsets);

  createArbitraryGraphErrorChooser("clusterTime_Means", "Cluster Time Resolution", m_OrderingVec, m_NullVec,
                                   m_mean_ClusterTimeResolution, m_RMS_ClusterTimeResolution, "set", "time residuals (ns)", m_graphList, m_Nsets);

  createArbitraryGraphErrorChooser("clusterUposition_Means", "Cluster U Position Resolution", m_OrderingVec, m_NullVec,
                                   m_mean_ClusterUPositionResolution, m_RMS_ClusterUPositionResolution, "set", "U position residuals (cm)", m_graphList,
                                   m_NsetsRed);

  createArbitraryGraphErrorChooser("clusterVposition_Means", "Cluster V Position Resolution", m_OrderingVec, m_NullVec,
                                   m_mean_ClusterVPositionResolution, m_RMS_ClusterVPositionResolution, "set", "V position residuals (cm)", m_graphList,
                                   m_NsetsRed);

  createArbitraryGraphErrorChooser("clusterInternalPurity_Means", "Fraction of Truth-matched Recos inside a Truth-matched Cluster",
                                   m_OrderingVec, m_NullVec, m_mean_PurityInsideTMCluster, m_RMS_PurityInsideTMCluster, "set",
                                   "number of TM recos / cluster size", m_graphList, m_Nsets);

  createArbitraryGraphErrorChooser("THinCluster_Means", "Number of True Hits inside a Cluster", m_OrderingVec, m_NullVec,
                                   m_mean_THinCluster, m_RMS_THinCluster, "set", "number of TH per cluster", m_graphList, m_Nsets);

  createArbitraryGraphErrorChooser("THinClusterTM_Means", "Number of True Hits inside a TM Cluster", m_OrderingVec, m_NullVec,
                                   m_mean_THinClusterTM, m_RMS_THinClusterTM, "set", "number of TH per TM cluster", m_graphList, m_Nsets);

  createArbitraryGraphErrorChooser("goodTHinClusterTM_Means", "Number of Good True Hits inside a TM Cluster", m_OrderingVec,
                                   m_NullVec,
                                   m_mean_GoodTHinClusterTM, m_RMS_GoodTHinClusterTM, "set", "number of Good TH per TM cluster", m_graphList, m_Nsets);

  createArbitraryGraphErrorChooser("goodTHinClusterTMGood_Means", "Number of Good True Hits inside a Good TM Cluster", m_OrderingVec,
                                   m_NullVec,
                                   m_mean_GoodTHinClusterTMGood, m_RMS_GoodTHinClusterTMGood, "set", "number of Good TH per Good TM cluster", m_graphList, m_Nsets);
  ///////////////////////////
  //WRITE HISTOS AND GRAPHS//
  ///////////////////////////

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

    TDirectory* dir_cltimepull = oldDir->mkdir("cluster_time_pull");
    dir_cltimepull->cd();
    TIter nextH_cltimepull(m_histoList_ClusterTimePull);
    while ((obj = nextH_cltimepull()))
      obj->Write();

    TDirectory* dir_clpos = oldDir->mkdir("cluster_position");
    dir_clpos->cd();
    TIter nextH_clpos(m_histoList_ClusterPositionResolution);
    while ((obj = nextH_clpos()))
      obj->Write();

    TDirectory* dir_clpospull = oldDir->mkdir("cluster_position_pull");
    dir_clpospull->cd();
    TIter nextH_clpospull(m_histoList_ClusterPositionPull);
    while ((obj = nextH_clpospull()))
      obj->Write();

    TDirectory* dir_clpostime = oldDir->mkdir("cluster_timeVSposition");
    dir_clpostime->cd();
    TIter nextH_clpostime(m_histo2DList_TresVsPosres);
    while ((obj = nextH_clpostime()))
      obj->Write();

    TDirectory* dir_clinpurTM = oldDir->mkdir("intra_cluster_purity_TM");
    dir_clinpurTM->cd();
    TIter nextH_clinpurTM(m_histoList_PurityInsideTMCluster);
    while ((obj = nextH_clinpurTM()))
      obj->Write();

    TDirectory* dir_clinpurTM2D = oldDir->mkdir("intra_cluster_purity_TM2D");
    dir_clinpurTM2D->cd();
    TIter nextH_clinpurTM2D(m_histo2DList_PurityInsideTMCluster);
    while ((obj = nextH_clinpurTM2D()))
      obj->Write();

    TDirectory* dir_clinpurNOTM = oldDir->mkdir("intra_cluster_purity_NOTM");
    dir_clinpurNOTM->cd();
    TIter nextH_clinpurNOTM(m_histoList_PurityInsideNOTMCluster);
    while ((obj = nextH_clinpurNOTM()))
      obj->Write();

    TDirectory* dir_puddle = oldDir->mkdir("trueHits_in_cluster");
    dir_puddle->cd();
    TIter nextH_puddle(m_histoList_THinCluster);
    while ((obj = nextH_puddle()))
      obj->Write();

    TDirectory* dir_puddleTM = oldDir->mkdir("trueHits_in_TMcluster");
    dir_puddleTM->cd();
    TIter nextH_puddleTM(m_histoList_THinClusterTM);
    while ((obj = nextH_puddleTM()))
      obj->Write();

    TDirectory* dir_goodPuddleTM = oldDir->mkdir("goodTrueHits_in_TMcluster");
    dir_goodPuddleTM->cd();
    TIter nextH_GoodPuddleTM(m_histoList_GoodTHinClusterTM);
    while ((obj = nextH_GoodPuddleTM()))
      obj->Write();

    TDirectory* dir_goodPuddleTMGood = oldDir->mkdir("goodTrueHits_in_GoodTMcluster");
    dir_goodPuddleTMGood->cd();
    TIter nextH_GoodPuddleTMGood(m_histoList_GoodTHinClusterTMGood);
    while ((obj = nextH_GoodPuddleTMGood()))
      obj->Write();

    TDirectory* dir_graph = oldDir->mkdir("graphs");
    dir_graph->cd();
    TIter nextH_graph(m_graphList);
    while ((obj = nextH_graph()))
      obj->Write();

    TDirectory* dir_controlsMC = oldDir->mkdir("controlMC");
    dir_controlsMC->cd();
    TIter nextH_controlsMC(m_histoList_Control);
    while ((obj = nextH_controlsMC()))
      obj->Write();

    m_outputFile->Close();
  }
}


void SVDClusterEvaluationModule::terminate()
{
}


///////////////////
//EXTRA FUNCTIONS//
///////////////////

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

TH2F* SVDClusterEvaluationModule::createHistogram2D(const char* name, const char* title,
                                                    Int_t nbinsX, Double_t minX, Double_t maxX,
                                                    const char* titleX,
                                                    Int_t nbinsY, Double_t minY, Double_t maxY,
                                                    const char* titleY, TList* histoList)
{

  TH2F* h = new TH2F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);

  if (histoList)
    histoList->Add(h);

  return h;
}

int SVDClusterEvaluationModule::indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber)
{
  int Index;

  if (LayerNumber == 3) { //L3
    if (UVNumber) //U
      Index = 0;
    else //V
      Index = 1;
  } else { //L456
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
    name = "L3";
  else
    name = "L456";

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

void SVDClusterEvaluationModule::createEfficiencyGraph(const char* name, const char* title, int vNum[m_Nsets], int vDen[m_Nsets],
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
      ratioErr[set] = sqrt(ratio[set] * (1 - ratio[set]) / (float)vDen[set]);
    }

  }

  TCanvas* c = new TCanvas(name, title);
  TGraphErrors* g = new TGraphErrors(m_Nsets, x, ratio, xErr, ratioErr);
  g->SetName(name);
  g->SetTitle(title);
  g->GetXaxis()->SetTitle(xTitle.Data());
  g->GetYaxis()->SetTitle(yTitle.Data());
  g->GetYaxis()->SetRangeUser(0.00001, 1.10);
  g->Draw("AP");
  g->SetMarkerStyle(20);
  g->SetMarkerSize(0.8);
  TAxis* xAxis = g->GetXaxis();

  TText* t = new TText();
  t->SetTextAlign(32);
  t->SetTextSize(0.035);
  t->SetTextFont(72);
  TString labels[m_Nsets] = {"3U", "3V", "456FU", "456FV", "456BU", "456BV"};
  for (Int_t i = 0; i < m_Nsets; i++) {
    xAxis->SetBinLabel(xAxis->FindBin(i + 1) , labels[i].Data());
  }

  if (list)
    list->Add(c);

}

void SVDClusterEvaluationModule::createArbitraryGraphErrorChooser(const char* name, const char* title, float x[m_Nsets],
    float xErr[m_Nsets], float y[m_Nsets], float yErr[m_Nsets], TString xTitle, TString yTitle, TList* list, int len)
{
  if (len == m_NsetsRed)
    createArbitraryGraphError_Red(name, title, x, xErr, y, yErr, xTitle, yTitle, list);
  else if (len == m_Nsets)
    createArbitraryGraphError_Std(name, title, x, xErr, y, yErr, xTitle, yTitle, list);
  else
    B2INFO("ERROR, WRONG LENGTH FOR MEANS TGRAPH CREATION!!!");
}

void SVDClusterEvaluationModule::createArbitraryGraphError_Std(const char* name, const char* title, float x[m_Nsets],
    float xErr[m_Nsets], float y[m_Nsets], float yErr[m_Nsets], TString xTitle, TString yTitle, TList* list)
{

  TCanvas* c = new TCanvas(name, title);
  TGraphErrors* g = new TGraphErrors(m_Nsets, x, y, xErr, yErr);
  g->SetName(name);
  g->SetTitle(title);
  g->GetXaxis()->SetTitle(xTitle.Data());
  g->GetYaxis()->SetTitle(yTitle.Data());
  g->Draw("AP");
  g->SetMarkerStyle(20);
  g->SetMarkerSize(0.8);
  TAxis* xAxis = g->GetXaxis();

  TText* t = new TText();
  t->SetTextAlign(32);
  t->SetTextSize(0.035);
  t->SetTextFont(72);
  TString labels[m_Nsets] = {"3U", "3V", "456FU", "456FV", "456BU", "456BV"};
  for (Int_t i = 0; i < m_Nsets; i++) {
    xAxis->SetBinLabel(xAxis->FindBin(i + 1) , labels[i].Data());
  }

  if (list)
    list->Add(c);

}

void SVDClusterEvaluationModule::createArbitraryGraphError_Red(const char* name, const char* title, float x[m_NsetsRed],
    float xErr[m_NsetsRed], float y[m_NsetsRed], float yErr[m_NsetsRed], TString xTitle, TString yTitle, TList* list)
{

  TCanvas* c = new TCanvas(name, title);
  TGraphErrors* g = new TGraphErrors(m_NsetsRed, x, y, xErr, yErr);
  g->SetName(name);
  g->SetTitle(title);
  g->GetXaxis()->SetTitle(xTitle.Data());
  g->GetYaxis()->SetTitle(yTitle.Data());
  g->Draw("AP");
  g->SetMarkerStyle(20);
  g->SetMarkerSize(0.8);
  TAxis* xAxis = g->GetXaxis();

  TText* t = new TText();
  t->SetTextAlign(32);
  t->SetTextSize(0.035);
  t->SetTextFont(72);
  TString labels[m_NsetsRed] = {"3", "456F", "456B"};
  for (Int_t i = 0; i < m_NsetsRed; i++) {
    xAxis->SetBinLabel(xAxis->FindBin(i + 1) , labels[i].Data());
  }

  if (list)
    list->Add(c);

}

bool SVDClusterEvaluationModule::goodTrueHit(const SVDTrueHit* thino)
{

  float charge = 0;
  bool primary = false;

  bool isGood = false;

  RelationVector<MCParticle> relatVectorTHToMC = thino->getRelationsFrom<MCParticle>();

  if (relatVectorTHToMC.size() > 0) {

    m_histoControl_THToMCsize->Fill(relatVectorTHToMC.size());

    charge = relatVectorTHToMC[0]->getCharge();
    primary = relatVectorTHToMC[0]->isPrimaryParticle();

    m_histoControl_MCcharge->Fill(charge);
    m_histoControl_MCisPrimary->Fill(primary);

    if (charge != 0 && primary)
      isGood = true;
  }

  return isGood;
}
























