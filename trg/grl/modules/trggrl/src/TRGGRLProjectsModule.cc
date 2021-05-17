/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: czhearty                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <trg/grl/modules/trggrl/TRGGRLProjectsModule.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>
#include <trg/ecl/dataobjects/TRGECLTrg.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/grl/dataobjects/TRGGRLMATCH.h>
#include <trg/grl/dataobjects/TRGGRLMATCHKLM.h>
#include <trg/grl/dataobjects/TRGGRLPHOTON.h>
#include <trg/klm/dataobjects/KLMTriggerTrack.h>
#include <framework/dataobjects/BinnedEventT0.h>
#include <trg/ecl/TrgEclMapping.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>

#include <TLorentzVector.h>
#include <TMath.h>

#include <iostream>
#include <fstream>


using namespace std;
using namespace Belle2;
using namespace ECL;


//..ECL look up tables
std::vector<int> TCThetaID;
std::vector<float> TCPhiLab;
std::vector<float> TCcotThetaLab;
std::vector<float> TCPhiCOM;
std::vector<float> TCThetaCOM;
std::vector<float> TC1GeV;


//..Other
double radtodeg;
//int iEvent(0);
//int nInAcc=0;

//..Trigger counters
//const int ntrig = 18;
//int trigbit[ntrig];
//int prescale[ntrig];
//int RawCount[ntrig];
//bool passBeforePrescale[ntrig];
//int pass2[ntrig][ntrig]={};
//int pass2Acc[ntrig][ntrig]={};
//std::vector<string> trigName;

//int nUseful;
//std::vector<int> itUseful;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TRGGRLProjects)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TRGGRLProjectsModule::TRGGRLProjectsModule() : Module()
{
  // Set module properties

  // string desc = "TRGGRLProjectsModule(" + version() + ")";
  setDescription("TRGGRLProjectsModule");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("DebugLevel", _debugLevel, "TRGGRL debug level", _debugLevel);
  addParam("ConfigFile",
           m_configFilename,
           "The filename of CDC trigger config file",
           m_configFilename);
  addParam("SimulationMode",
           m_simulationMode,
           "TRGGRL simulation switch",
           1);
  addParam("FastSimulationMode",
           m_fastSimulationMode,
           "TRGGRL fast simulation mode",
           m_fastSimulationMode);
  addParam("FirmwareSimulationMode",
           m_firmwareSimulationMode,
           "TRGGRL firmware simulation mode",
           m_firmwareSimulationMode);
  addParam("2DfinderCollection", m_2DfinderCollectionName,
           "Name of the StoreArray holding the tracks made by the 2D finder to be used as input.",
           string("TRGCDC2DFinderTracks"));
  addParam("2DfitterCollection", m_2DfitterCollectionName,
           "Name of the StoreArray holding the tracks made by the 2D fitter to be used as input.",
           string("TRGCDC2DFitterTracks"));
  addParam("3DfitterCollection", m_3DfitterCollectionName,
           "Name of the StoreArray holding the tracks made by the 3D fitter to be used as input.",
           string("TRGCDC3DFitterTracks"));
  addParam("NNCollection", m_NNCollectionName,
           "Name of the StoreArray holding the tracks made by the neural network (NN).",
           string("TRGCDCNeuroTracks"));
  addParam("2DmatchCollection", m_2DmatchCollectionName,
           "Name of the StoreArray holding the macthed tracks and clusters made by the 2D fitter.",
           string("TRG2DMatchTracks"));
  addParam("PhimatchCollection", m_phimatch_tracklist, "the 2d tracklist with associated cluster", std::string("TRGPhiMatchTracks"));
  addParam("3DmatchCollection", m_3DmatchCollectionName,
           "Name of the StoreArray holding the matched 3D NN tracks and clusters made",
           string("TRG3DMatchTracks"));
  addParam("KLMmatchCollection", m_klmmatch_tracklist, "the 2d tracklist with associated KLM track",
           std::string("TRGKLMMatchTracks"));
  addParam("TrgGrlInformation", m_TrgGrlInformationName,
           "Name of the StoreArray holding the information of tracks and clusters from cdc ecl klm.",
           string("TRGGRLObjects"));
  addParam("TRGECLClusters", m_TrgECLClusterName,
           "Name of the StoreArray holding the information of trigger ecl clusters ",
           string("TRGECLClusters"));
  addParam("TRGECLTrgs", m_TrgECLTrgsName,
           "Name of the StoreArray holding the information of ecl trigger",
           string("TRGECLTrgs"));
  addParam("TrgKLMTracks", m_KLMTrackName,
           "Name of the StoreArray holding the information of klm track ",
           string("TRGKLMTracks"));
  addParam("TrgKLMHits", m_KLMHitName,
           "Name of the StoreArray holding the information of klm hit",
           string("TRGKLMHits"));
  addParam("ECLClusterTimeWindow", m_eclClusterTimeWindow,
           "The time window of the signal eclclusters",
           100.0);
  addParam("ClusEngThreshold", m_energythreshold, "The energy threshold of clusters", {0.1, 0.3, 1.0, 2.0});
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void TRGGRLProjectsModule::initialize()
{
  radtodeg = 180. / TMath::Pi();
  //..Trigger ThetaID for each trigger cell. Could be replaced by getMaxThetaId() for newer MC
  TrgEclMapping* trgecl_obj = new TrgEclMapping();
  for (int tc = 1; tc <= 576; tc++) {
    TCThetaID.push_back(trgecl_obj->getTCThetaIdFromTCId(tc));
  }

  //-----------------------------------------------------------------------------------------
  //..ECL look up tables
  PCmsLabTransform boostrotate;
  for (int tc = 1; tc <= 576; tc++) {

    //..Four vector of a 1 GeV lab photon at this TC
    TVector3 CellPosition = trgecl_obj->getTCPosition(tc);
    TLorentzVector CellLab(1., 1., 1., 1.);
    CellLab.SetTheta(CellPosition.Theta());
    CellLab.SetPhi(CellPosition.Phi());
    CellLab.SetRho(1.);
    CellLab.SetE(1.);

    //..cotan Theta and phi in lab
    TCPhiLab.push_back(CellPosition.Phi()*radtodeg);
    double tantheta = tan(CellPosition.Theta());
    TCcotThetaLab.push_back(1. / tantheta);

    //..Corresponding 4 vector in the COM frame
    TLorentzVector CellCOM = boostrotate.rotateLabToCms() * CellLab;
    TCThetaCOM.push_back(CellCOM.Theta()*radtodeg);
    TCPhiCOM.push_back(CellCOM.Phi()*radtodeg);

    //..Scale to give 1 GeV in the COM frame
    TC1GeV.push_back(1. / CellCOM.E());
  }

  //m_TRGGRLInfo.registerInDataStore(m_TrgGrlInformationName);

}

void
TRGGRLProjectsModule::beginRun()
{
  B2DEBUG(20, "TRGGDLModule ... beginRun called ");
  //...GDL config. name...
}
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void TRGGRLProjectsModule::event()
{
  //if(iEvent%1000==0) {cout << "TRGGRLProjects event " << iEvent << endl;}
  //iEvent++;

  //---------------------------------------------------------------------
  //..Read in the necessary arrays
  StoreArray<TRGECLTrg> trgArray;
  //StoreArray<MCParticle> MCParticleArray;
  StoreArray<CDCTriggerTrack> cdc2DTrkArray("TRGCDC2DFinderTracks");
  StoreArray<CDCTriggerTrack> cdc3DTrkArray("TRGCDC3DFitterTracks");
  StoreArray<CDCTriggerTrack> cdcNNTrkArray("TRGCDCNeuroTracks");
  StoreArray<TRGECLCluster> eclTrgClusterArray("TRGECLClusters");
  StoreArray<TRGGRLMATCH> trackphimatch("TRGPhiMatchTracks");
  StoreArray<TRGGRLMATCHKLM> trackKLMmatch("TRGKLMMatchTracks");
  StoreArray<KLMTriggerTrack> klmtracklist("TRGKLMTracks");
  StoreArray<TRGGRLPHOTON> grlphoton("TRGGRLPhotons");
  StoreObjPtr<BinnedEventT0> m_eventTime;
  StoreObjPtr<TRGGRLInfo> trgInfo(m_TrgGrlInformationName);
  //trgInfo.create();
  //---------------------------------------------------------------------
  //..Use only clusters within 100 ns of event timing (from ECL).
  int ntrgArray = trgArray.getEntries();
  double EventTiming = -9999.;
  if (ntrgArray > 0) {EventTiming = trgArray[0]->getEventTiming();}
  std::vector<int> selTC;
  std::vector<int> selTheta;
  std::vector<float> selE;
  for (int ic = 0; ic < eclTrgClusterArray.getEntries(); ic++) {
    double tcT = abs(eclTrgClusterArray[ic]->getTimeAve() - EventTiming);
    if (tcT < 100.) {
      int TC = eclTrgClusterArray[ic]->getMaxTCId();
      selTC.push_back(TC);
      selTheta.push_back(TCThetaID[TC - 1]);
      selE.push_back(eclTrgClusterArray[ic]->getEnergyDep());
    }
  }

  //---------------------------------------------------------------------
  //..Trigger objects from CDC alone
  //  nTrk3D  nTrkZ10  nTrkZ25
  int nTrk2D = cdc2DTrkArray.getEntries();
  int nTrk3D = cdc3DTrkArray.getEntries();
  int nTrkZ10 = 0;
  int nTrkZ25 = 0;
  int nTrkZ35 = 0;
  for (int itrk = 0; itrk < nTrk3D; itrk++) {
    double z0 = cdc3DTrkArray[itrk]->getZ0();
    if (abs(z0) < 10.) {nTrkZ10++;}
    if (abs(z0) < 25.) {nTrkZ25++;}
    if (abs(z0) < 35.) {nTrkZ35++;}
  }
  int nTrkNN = cdcNNTrkArray.getEntries();
  int nTrkNNSTT = 0;
  int nTrkNN20 = 0;
  int nTrkNN40 = 0;
  for (int itrk = 0; itrk < nTrkNN; itrk++) {
    double z0 = cdcNNTrkArray[itrk]->getZ0();

    double omega = 100 * cdcNNTrkArray[itrk]->getOmega();
    int omega_bin = omega / 0.044;
    omega = omega_bin * 0.044;

    double cottheta = cdcNNTrkArray[itrk]->getCotTheta();
    double theta = 0;
    if (cottheta != 0)theta = atan(1. / cottheta);
    int theta_bin = theta / 0.098125;
    theta = theta_bin * 0.098125;
    int p = abs(10 * 0.3 * 1.5 / omega / sin(theta)); //0.1GeV unit

    double pt    = cdcNNTrkArray[itrk]->getPt();
    double p_abs = cdcNNTrkArray[itrk]->getPt() / sin(theta);
    B2DEBUG(20, "NN momentum " << omega << " " << theta * 180 / 3.14 << " " << p * 0.1 << " " << p_abs << " " << pt);

    if (abs(z0) < 15. && p > 7) {nTrkNNSTT++;}
    if (abs(z0) < 20.) {nTrkNN20++;}
    if (abs(z0) < 40.) {nTrkNN40++;}
  }

  trgInfo->setN3Dfittertrk(nTrk3D);
  trgInfo->setN3DfittertrkZ10(nTrkZ10);
  trgInfo->setN3DfittertrkZ25(nTrkZ25);
  trgInfo->setN3DfittertrkZ35(nTrkZ35);
  trgInfo->setNNNtrk(nTrkNN);
  trgInfo->setNNNtrkZ20(nTrkNN20);
  trgInfo->setNNNtrkZ40(nTrkNN40);
  trgInfo->setNNNtrkSTT(nTrkNNSTT);
  //---------------------------------------------------------------------
  //..Trigger objects using single ECL clusters
  // nClust n300MeV n2GeV n2GeV414 n2GeV231516 n2GeV117 n1GeV415 n1GeV2316 n1GeV117
  int nClust = selTC.size();
  int n300MeV = 0;
  int n2GeV = 0;
  int n2GeV414 = 0;
  int n2GeV231516 = 0;
  int n2GeV117 = 0;
  int n1GeV415 = 0;
  int n1GeV2316 = 0;
  int n1GeV117 = 0;
  for (int ic = 0; ic < nClust; ic++) {
    if (selE[ic] > 0.3) {n300MeV++;}
    float thresh = TC1GeV[selTC[ic] - 1];
    if (selE[ic] > (thresh + thresh)) {
      n2GeV++;
      if (selTheta[ic] >= 4 && selTheta[ic] <= 14) {n2GeV414++;}
      if (selTheta[ic] == 2 || selTheta[ic] == 3 || selTheta[ic] == 15 || selTheta[ic] == 16) {n2GeV231516++;}
      if (selTheta[ic] == 1 || selTheta[ic] == 17) {n2GeV117++;}
    }
    if (selE[ic] > thresh) {
      if (selTheta[ic] >= 4 && selTheta[ic] <= 15) {n1GeV415++;}
      if (selTheta[ic] == 2 || selTheta[ic] == 3 || selTheta[ic] == 16) {n1GeV2316++;}
      if (selTheta[ic] == 1 || selTheta[ic] == 17) {n1GeV117++;}
    }
  }
  trgInfo->setNcluster(nClust);
  trgInfo->setNhigh300cluster(n300MeV);
  trgInfo->setNhigh1GeVcluster415(n1GeV415);
  trgInfo->setNhigh1GeVcluster2316(n1GeV2316);
  trgInfo->setNhigh1GeVcluster117(n1GeV117);
  trgInfo->setNhigh2GeVcluster(n2GeV);
  trgInfo->setNhigh2GeVcluster414(n2GeV414);
  trgInfo->setNhigh2GeVcluster231516(n2GeV231516);
  trgInfo->setNhigh2GeVcluster117(n2GeV117);

  //---------------------------------------------------------------------
  //..Trigger objects using back-to-back ECL clusters, plus Bhabha vetoes
  //  nPhiPairHigh nPhiPairLow n3DPair nECLBhabha nTrkBhabha
  int nPhiPairHigh = 0;
  int nPhiPairLow = 0;
  int n3DPair = 0;
  int nECLBhabha = 0;
  int nTrkBhabha = 0;
  for (int i0 = 0; i0 < nClust - 1; i0++) {
    for (int i1 = i0 + 1; i1 < nClust; i1++) {

      //..back to back in phi
      float dphi = abs(TCPhiCOM[selTC[i1] - 1] - TCPhiCOM[selTC[i0] - 1]);
      if (dphi > 180.) {dphi = 360 - dphi;}
      if (dphi > 170. && selE[i0] > 0.25 && selE[i1] > 0.25) {nPhiPairHigh++;}
      if (dphi > 170. && (selE[i0] < 0.25 || selE[i1] < 0.25)) {nPhiPairLow++;}

      //..3D
      float thetaSum = TCThetaCOM[selTC[i0] - 1] + TCThetaCOM[selTC[i1] - 1];
      if (dphi > 160. && thetaSum > 160. && thetaSum < 200.) {n3DPair++;}

      //..ecl Bhabha
      if (dphi > 160. && thetaSum > 165. && thetaSum < 190. && selE[i0] > 3.*TC1GeV[selTC[i0] - 1] && selE[i1] > 3.*TC1GeV[selTC[i1] - 1]
          && (selE[i0] > 4.5 * TC1GeV[selTC[i0] - 1] ||  selE[i1] > 4.5 * TC1GeV[selTC[i1] - 1])) {
        nECLBhabha++;

        //..Bhabha also using the CDC
        bool c0matched = false;
        bool c1matched = false;
        for (int itrk = 0; itrk < nTrk3D; itrk++) {
          double phiTrk = cdc3DTrkArray[itrk]->getPhi0() * radtodeg;
          double ptTrk = cdc3DTrkArray[itrk]->getTransverseMomentum(1.5);
          float dphi0 = abs(phiTrk - TCPhiLab[selTC[i0] - 1]);
          if (dphi0 > 180.) {dphi0 = 360. - dphi0;}
          float dphi1 = abs(phiTrk - TCPhiLab[selTC[i1] - 1]);
          if (dphi1 > 180.) {dphi1 = 360. - dphi1;}
          if (ptTrk > 1. && dphi0 < 15.) {c0matched = true;}
          if (ptTrk > 1. && dphi1 < 15.) {c1matched = true;}
        }
        if (c0matched && c1matched) {nTrkBhabha++;}
      }
    }
  }

  trgInfo->setBhabhaVeto(nTrkBhabha);
  trgInfo->seteclBhabhaVeto(nECLBhabha);
  trgInfo->setPairHigh(nPhiPairHigh);
  trgInfo->setPairLow(nPhiPairLow);
  trgInfo->set3DPair(n3DPair);


  //---------------------------------------------------------------------
  //..Trigger objects using 1 track and at least 1 cluster
  // nSameHem1Trk nOppHem1Trk
  int nSameHem1Trk = 0;
  int nOppHem1Trk = 0;
  if (nTrk3D == 1) {
    double phiTrk = cdc3DTrkArray[0]->getPhi0() * radtodeg;
    double cotTrk = cdc3DTrkArray[0]->getCotTheta();
    for (int i0 = 0; i0 < nClust; i0++) {
      float dphi = abs(phiTrk - TCPhiLab[selTC[i0] - 1]);
      if (dphi > 180.) {dphi = 360. - dphi;}
      float dCot = cotTrk - TCcotThetaLab[selTC[i0] - 1];
      if (dphi > 80.) {nOppHem1Trk++;}
      if (dphi < 80. && (dCot < -0.8 || dCot > 0.6)) {nSameHem1Trk++;}
    }
  }

  trgInfo->setNSameHem1Trk(nSameHem1Trk);
  trgInfo->setNOppHem1Trk(nOppHem1Trk);

  //---------------------------------------------------------------------
  //..Trk b2b
  int Trk_b2b_1to3 = 0;
  int Trk_b2b_1to5 = 0;
  int Trk_b2b_1to7 = 0;
  int Trk_b2b_1to9 = 0;
  int Trk_open90 = 0;
  int Trk_open30 = 0;
  for (int itrk = 0; itrk < cdc2DTrkArray.getEntries(); itrk++) {

    int phi_i_itrk = (int)((cdc2DTrkArray[itrk]->getPhi0()) * (180 / M_PI) / 10);

    for (int jtrk = 0; jtrk < cdc2DTrkArray.getEntries(); jtrk++) {
      if (itrk <= jtrk) continue;

      int phi_i_jtrk = (int)((cdc2DTrkArray[jtrk]->getPhi0()) * (180 / M_PI) / 10);
      if (abs(phi_i_itrk - phi_i_jtrk) >= 17 && abs(phi_i_itrk - phi_i_jtrk) <= 19) {Trk_b2b_1to3 = 1;}
      if (abs(phi_i_itrk - phi_i_jtrk) >= 16 && abs(phi_i_itrk - phi_i_jtrk) <= 20) {Trk_b2b_1to5 = 1;}
      if (abs(phi_i_itrk - phi_i_jtrk) >= 15 && abs(phi_i_itrk - phi_i_jtrk) <= 21) {Trk_b2b_1to7 = 1;}
      if (abs(phi_i_itrk - phi_i_jtrk) >= 14 && abs(phi_i_itrk - phi_i_jtrk) <= 22) {Trk_b2b_1to9 = 1;}
      if (abs(phi_i_itrk - phi_i_jtrk) >= 9 && abs(phi_i_itrk - phi_i_jtrk) <= 27) {Trk_open90 = 1;}
      if (abs(phi_i_itrk - phi_i_jtrk) >= 3 && abs(phi_i_itrk - phi_i_jtrk) <= 33) {Trk_open30 = 1;}
    }
  }
  trgInfo->setTrk_b2b_1to3(Trk_b2b_1to3);
  trgInfo->setTrk_b2b_1to5(Trk_b2b_1to5);
  trgInfo->setTrk_b2b_1to7(Trk_b2b_1to7);
  trgInfo->setTrk_b2b_1to9(Trk_b2b_1to9);
  trgInfo->setTrk_open90(Trk_open90);
  trgInfo->setTrk_open30(Trk_open30);

  //---------------------------------------------------------------------
  //..cluster b2b
  int cluster_b2b_1to3 = 0;
  int cluster_b2b_1to5 = 0;
  int cluster_b2b_1to7 = 0;
  int cluster_b2b_1to9 = 0;
  for (int iclu = 0; iclu < eclTrgClusterArray.getEntries(); iclu++) {

    double x_iclu = eclTrgClusterArray[iclu]->getPositionX();
    double y_iclu = eclTrgClusterArray[iclu]->getPositionY();

    int phi_iclu = 0;
    if (x_iclu >= 0 && y_iclu >= 0) {phi_iclu = (int)(atan(y_iclu / x_iclu) * (180.0 / M_PI) / 10);}
    else if (x_iclu < 0 && y_iclu >= 0) {phi_iclu = (int)((atan(y_iclu / x_iclu) * (180.0 / M_PI) + 180.0) / 10);}
    else if (x_iclu < 0 && y_iclu < 0) {phi_iclu = (int)((atan(y_iclu / x_iclu) * (180.0 / M_PI) + 180.0) / 10);}
    else if (x_iclu >= 0 && y_iclu < 0) {phi_iclu = (int)((atan(y_iclu / x_iclu) * (180.0 / M_PI) + 360.0) / 10);}

    for (int jclu = 0; jclu < eclTrgClusterArray.getEntries(); jclu++) {
      if (iclu <= jclu) continue;

      double x_jclu = eclTrgClusterArray[jclu]->getPositionX();
      double y_jclu = eclTrgClusterArray[jclu]->getPositionY();

      int phi_jclu = 0;
      if (x_jclu >= 0 && y_jclu >= 0) {phi_jclu = (int)(atan(y_jclu / x_jclu) * (180.0 / M_PI) / 10);}
      else if (x_jclu < 0 && y_jclu >= 0) {phi_jclu = (int)((atan(y_jclu / x_jclu) * (180.0 / M_PI) + 180.0) / 10);}
      else if (x_jclu < 0 && y_jclu < 0) {phi_jclu = (int)((atan(y_jclu / x_jclu) * (180.0 / M_PI) + 180.0) / 10);}
      else if (x_jclu >= 0 && y_jclu < 0) {phi_jclu = (int)((atan(y_jclu / x_jclu) * (180.0 / M_PI) + 360.0) / 10);}

      if (abs(phi_iclu - phi_jclu) >= 17 && abs(phi_iclu - phi_jclu) <= 19) {cluster_b2b_1to3 = 1;}
      if (abs(phi_iclu - phi_jclu) >= 16 && abs(phi_iclu - phi_jclu) <= 20) {cluster_b2b_1to5 = 1;}
      if (abs(phi_iclu - phi_jclu) >= 15 && abs(phi_iclu - phi_jclu) <= 21) {cluster_b2b_1to7 = 1;}
      if (abs(phi_iclu - phi_jclu) >= 14 && abs(phi_iclu - phi_jclu) <= 22) {cluster_b2b_1to9 = 1;}
    }
  }
  trgInfo->setcluster_b2b_1to3(cluster_b2b_1to3);
  trgInfo->setcluster_b2b_1to5(cluster_b2b_1to5);
  trgInfo->setcluster_b2b_1to7(cluster_b2b_1to7);
  trgInfo->setcluster_b2b_1to9(cluster_b2b_1to9);


  //---------------------------------------------------------------------
  //..eed, fed

  int eed = 0, fed = 0;
  if (cdc2DTrkArray.getEntries() == 2 && trackphimatch.getEntries() == 2 && cluster_b2b_1to5 == 1) {eed = 1;}
  if (cdc2DTrkArray.getEntries() == 1 && trackphimatch.getEntries() == 1 && cluster_b2b_1to5 == 1) {fed = 1;}
  trgInfo->seteed(eed);
  trgInfo->setfed(fed);

  //---------------------------------------------------------------------
  //..Track-cluster b2b
  int Trkcluster_b2b_1to3 = 0;
  int Trkcluster_b2b_1to5 = 0;
  int Trkcluster_b2b_1to7 = 0;
  int Trkcluster_b2b_1to9 = 0;
  for (int itrk = 0; itrk < cdc2DTrkArray.getEntries(); itrk++) {
    double    _r = 1.0 / cdc2DTrkArray[itrk]->getOmega() ;
    double    _phi = cdc2DTrkArray[itrk]->getPhi0() ;
    double phi_p = acos(126.0 / (2 * fabs(_r)));
    int charge = 0;
    if (_r > 0) {charge = 1;}
    else if (_r < 0) {charge = -1;}
    else {charge = 0;}

    double phi_CDC = 0.0;
    if (charge == 1) {
      phi_CDC = _phi + phi_p - 0.5 * M_PI;
    } else if (charge == -1) {
      phi_CDC = _phi - phi_p + 0.5 * M_PI;
    } else {
      phi_CDC = _phi;
    }

    if (phi_CDC > 2 * M_PI) {phi_CDC = phi_CDC - 2 * M_PI;}
    else if (phi_CDC < 0) {phi_CDC = phi_CDC + 2 * M_PI;}
    int phi_itrk = (int)(phi_CDC * (180.0 / M_PI) / 10);

    for (int jclu = 0; jclu < eclTrgClusterArray.getEntries(); jclu++) {

      double x_jclu = eclTrgClusterArray[jclu]->getPositionX();
      double y_jclu = eclTrgClusterArray[jclu]->getPositionY();

      int phi_jclu = 0;
      if (x_jclu >= 0 && y_jclu >= 0) {phi_jclu = (int)(atan(y_jclu / x_jclu) * (180.0 / M_PI) / 10);}
      else if (x_jclu < 0 && y_jclu >= 0) {phi_jclu = (int)((atan(y_jclu / x_jclu) * (180.0 / M_PI) + 180.0) / 10);}
      else if (x_jclu < 0 && y_jclu < 0) {phi_jclu = (int)((atan(y_jclu / x_jclu) * (180.0 / M_PI) + 180.0) / 10);}
      else if (x_jclu >= 0 && y_jclu < 0) {phi_jclu = (int)((atan(y_jclu / x_jclu) * (180.0 / M_PI) + 360.0) / 10);}

      if (abs(phi_itrk - phi_jclu) >= 17 && abs(phi_itrk - phi_jclu) <= 19) {Trkcluster_b2b_1to3 = 1;}
      if (abs(phi_itrk - phi_jclu) >= 16 && abs(phi_itrk - phi_jclu) <= 20) {Trkcluster_b2b_1to5 = 1;}
      if (abs(phi_itrk - phi_jclu) >= 15 && abs(phi_itrk - phi_jclu) <= 21) {Trkcluster_b2b_1to7 = 1;}
      if (abs(phi_itrk - phi_jclu) >= 14 && abs(phi_itrk - phi_jclu) <= 22) {Trkcluster_b2b_1to9 = 1;}
    }
  }

  trgInfo->setTrkcluster_b2b_1to3(Trkcluster_b2b_1to3);
  trgInfo->setTrkcluster_b2b_1to5(Trkcluster_b2b_1to5);
  trgInfo->setTrkcluster_b2b_1to7(Trkcluster_b2b_1to7);
  trgInfo->setTrkcluster_b2b_1to9(Trkcluster_b2b_1to9);

  //---------------------------------------------------------------------
  //..fp, eeb, fep

  int fp = 0;
  if (cdc2DTrkArray.getEntries() == 1 && Trkcluster_b2b_1to5 == 1) {fp = 1;}
  trgInfo->setfp(fp);

  int eeb = 0;
  if (trackphimatch.getEntries() == 2 && Trk_b2b_1to5 == 1) {eeb = 1;}
  trgInfo->seteeb(eeb);

  int fep = 0;
  if (cdc2DTrkArray.getEntries() == 1 && trackphimatch.getEntries() == 1 && Trkcluster_b2b_1to5 == 1) {fep = 1;}
  trgInfo->setfep(fep);


  //---------------------------------------------------------------------
  //..Get input bits from StoreArray<TRGECLTrg> trgArray
  //..Bit order is hard-coded in trg/ecl/src/TrgEclMaster.cc

  int ECLtoGDL[4] = {0, 0, 0, 0};
  if (ntrgArray > 0) {
    ECLtoGDL[0] = trgArray[0]->getECLtoGDL(0);
    ECLtoGDL[1] = trgArray[0]->getECLtoGDL(1);
    ECLtoGDL[2] = trgArray[0]->getECLtoGDL(2);
    ECLtoGDL[3] = trgArray[0]->getECLtoGDL(3);
  }

  // elow: 47
  bool elow = (ECLtoGDL[1] & (1 << (47 - 32 * 1))) != 0;
  // ehigh: 48
  bool ehigh = (ECLtoGDL[1] & (1 << (48 - 32 * 1))) != 0;
  // elum: 49
  bool elum = (ECLtoGDL[1] & (1 << (49 - 32 * 1))) != 0;
  // ecl_bha: 19
  bool ecl_bha = (ECLtoGDL[0] & (1 << (19 - 32 * 0))) != 0;
  // bha_type0: 20
  bool bha_type0 = (ECLtoGDL[0] & (1 << (20 - 32 * 0))) != 0;
  // bha_type1: 21
  bool bha_type1 = (ECLtoGDL[0] & (1 << (21 - 32 * 0))) != 0;
  // bha_type2: 22
  bool bha_type2 = (ECLtoGDL[0] & (1 << (22 - 32 * 0))) != 0;
  // bha_type3: 23
  bool bha_type3 = (ECLtoGDL[0] & (1 << (23 - 32 * 0))) != 0;
  // bha_type4: 24
  bool bha_type4 = (ECLtoGDL[0] & (1 << (24 - 32 * 0))) != 0;
  // bha_type5: 25
  bool bha_type5 = (ECLtoGDL[0] & (1 << (25 - 32 * 0))) != 0;
  // bha_type6: 26
  bool bha_type6 = (ECLtoGDL[0] & (1 << (26 - 32 * 0))) != 0;
  // bha_type7: 27
  bool bha_type7 = (ECLtoGDL[0] & (1 << (27 - 32 * 0))) != 0;
  // bha_type8: 28
  bool bha_type8 = (ECLtoGDL[0] & (1 << (28 - 32 * 0))) != 0;
  // bha_type9: 29
  bool bha_type9 = (ECLtoGDL[0] & (1 << (29 - 32 * 0))) != 0;
  // bha_type10: 30
  bool bha_type10 = (ECLtoGDL[0] & (1 << (30 - 32 * 0))) != 0;
  // bha_type11: 31
  bool bha_type11 = (ECLtoGDL[0] & (1u << (31 - 32 * 0))) != 0;
  // bha_type12: 32
  bool bha_type12 = (ECLtoGDL[1] & (1 << (32 - 32 * 1))) != 0;
  // bha_type13: 33
  bool bha_type13 = (ECLtoGDL[1] & (1 << (33 - 32 * 1))) != 0;

  bool nclst_0 = (eclTrgClusterArray.getEntries() & (1 << 0)) != 0;
  bool nclst_1 = (eclTrgClusterArray.getEntries() & (1 << 1)) != 0;
  bool nclst_2 = (eclTrgClusterArray.getEntries() & (1 << 2)) != 0;
  bool nclst_3 = (eclTrgClusterArray.getEntries() & (1 << 3)) != 0;

  // ecl_bg_0: 57
  bool ecl_bg_0 = (ECLtoGDL[1] & (1 << (57 - 32 * 1))) != 0;
  // ecl_bg_1: 58
  bool ecl_bg_1 = (ECLtoGDL[1] & (1 << (58 - 32 * 1))) != 0;
  // ecl_bg_2: 59
  bool ecl_bg_2 = (ECLtoGDL[1] & (1 << (59 - 32 * 1))) != 0;

  bool ecl_active = ntrgArray > 0;

  // ecl_timing_fwd: 15
  bool ecl_timing_fwd = (ECLtoGDL[0] & (1 << (15 - 32 * 0))) != 0;
  // ecl_timing_brl: 16
  bool ecl_timing_brl = (ECLtoGDL[0] & (1 << (16 - 32 * 0))) != 0;
  // ecl_timing_bwd: 17
  bool ecl_timing_bwd = (ECLtoGDL[0] & (1 << (17 - 32 * 0))) != 0;
  // ecl_phys: 18
  bool ecl_phys = (ECLtoGDL[0] & (1 << (18 - 32 * 0))) != 0;
  // ecl_oflo: 60
  bool ecl_oflo = (ECLtoGDL[1] & (1 << (60 - 32 * 1))) != 0;
  // ecl_3dbha: 61
  bool ecl_3dbha = (ECLtoGDL[1] & (1 << (61 - 32 * 1))) != 0;
  // ecl_lml_0: 62
  bool ecl_lml_0 = (ECLtoGDL[1] & (1 << (62 - 32 * 1))) != 0;
  // ecl_lml_1: 63
  bool ecl_lml_1 = (ECLtoGDL[1] & (1u << (63 - 32 * 1))) != 0;
  // ecl_lml_2: 64
  bool ecl_lml_2 = (ECLtoGDL[2] & (1 << (64 - 32 * 2))) != 0;
  // ecl_lml_3: 65
  bool ecl_lml_3 = (ECLtoGDL[2] & (1 << (65 - 32 * 2))) != 0;
  // ecl_lml_4: 66
  bool ecl_lml_4 = (ECLtoGDL[2] & (1 << (66 - 32 * 2))) != 0;
  // ecl_lml_5: 67
  bool ecl_lml_5 = (ECLtoGDL[2] & (1 << (67 - 32 * 2))) != 0;
  // ecl_lml_6: 68
  bool ecl_lml_6 = (ECLtoGDL[2] & (1 << (68 - 32 * 2))) != 0;
  // ecl_lml_7: 69
  bool ecl_lml_7 = (ECLtoGDL[2] & (1 << (69 - 32 * 2))) != 0;
  // ecl_lml_8: 70
  bool ecl_lml_8 = (ECLtoGDL[2] & (1 << (70 - 32 * 2))) != 0;
  // ecl_lml_9: 71
  bool ecl_lml_9 = (ECLtoGDL[2] & (1 << (71 - 32 * 2))) != 0;
  // ecl_lml_10: 72
  bool ecl_lml_10 = (ECLtoGDL[2] & (1 << (72 - 32 * 2))) != 0;
  // ecl_lml_11: 73
  bool ecl_lml_11 = (ECLtoGDL[2] & (1 << (73 - 32 * 2))) != 0;
  // ecl_lml_11: 78
  bool ecl_lml_12 = (ECLtoGDL[2] & (1 << (78 - 32 * 2))) != 0;
  // ecl_lml_11: 79
  bool ecl_lml_13 = (ECLtoGDL[2] & (1 << (79 - 32 * 2))) != 0;
  // ecl_mumu: 75
  bool ecl_mumu = (ECLtoGDL[2] & (1 << (75 - 32 * 2))) != 0;
  // ecl_bst: 77
  bool ecl_bst = (ECLtoGDL[2] & (1 << (77 - 32 * 2))) != 0;

  //---------------------------------------------------------------------
  //..Other input bits

  bool cdc_active = false;
  if (m_eventTime->hasBinnedEventT0(Const::CDC)) {
    if (m_eventTime->getBinnedEventT0(Const::CDC) != 0) { cdc_active = true; }
  }

  bool klm_hit = klmtracklist.getEntries() > 0;
  bool klm_0 = (klmtracklist.getEntries() & (1 << 0)) != 0;
  bool klm_1 = (klmtracklist.getEntries() & (1 << 1)) != 0;
  bool klm_2 = (klmtracklist.getEntries() & (1 << 2)) != 0;

  bool cdcklm_0 = (trackKLMmatch.getEntries() == 1);
  bool cdcklm_1 = (trackKLMmatch.getEntries() > 1);
  bool cdcklm_2 = (trackKLMmatch.getEntries() == 3);
  bool cdcklm_3 = (trackKLMmatch.getEntries() > 3);

  bool cdcecl_0 = (trackphimatch.getEntries() == 1);
  bool cdcecl_1 = (trackphimatch.getEntries() == 2);
  bool cdcecl_2 = (trackphimatch.getEntries() == 3);
  bool cdcecl_3 = (trackphimatch.getEntries() > 3);

  int n_c2gev = 0;
  for (int i = 0; i < trackphimatch.getEntries(); i++) {
    if (trackphimatch[i]->get_e() >= 2.0) {n_c2gev++;}
  }

  bool c2gev_0 = (n_c2gev == 1);
  bool c2gev_1 = (n_c2gev == 2);
  bool c2gev_2 = (n_c2gev == 3);
  bool c2gev_3 = (n_c2gev > 3);

  int N_KLMb2b = 0;
  for (int i = 0; i < klmtracklist.getEntries(); i++) {
    for (int j = 0; j < klmtracklist.getEntries(); j++) {
      if (i <= j) continue;
      int sector_i = klmtracklist[i]->getSector();
      int sector_j = klmtracklist[j]->getSector();
      if (abs(sector_i - sector_j) == 4) { N_KLMb2b++; }
    }
  }

  bool klmb2b_0 = (N_KLMb2b == 1);
  bool klmb2b_1 = (N_KLMb2b == 2);
  bool klmb2b_2 = (N_KLMb2b > 2);

  int N_clst1 = 0, N_clst2 = 0;
  for (int i = 0 ; i < grlphoton.getEntries() ; i++) {
    if (grlphoton[i]->get_e() > 1.0) { N_clst1++; }
    if (grlphoton[i]->get_e() > 2.0) { N_clst2++; }
  }

  bool nclst1_0 = (N_clst1 == 1);
  bool nclst1_1 = (N_clst1 == 2);
  bool nclst1_2 = (N_clst1 == 3);
  bool nclst1_3 = (N_clst1 > 3);

  bool nclst2_0 = (N_clst2 == 1);
  bool nclst2_1 = (N_clst2 == 2);
  bool nclst2_2 = (N_clst2 == 3);
  bool nclst2_3 = (N_clst2 > 3);

  int N_ST = trgInfo->getNshorttrk();
  int N_ST_fwd = trgInfo->getNshorttrk_fwd();
  int N_ST_bwd = trgInfo->getNshorttrk_bwd();
  int s2s3 = trgInfo->gets2s3();
  int s2s5 = trgInfo->gets2s5();
  int s2so = trgInfo->gets2so();
  int s2f3 = trgInfo->gets2f3();
  int s2f5 = trgInfo->gets2f5();
  int s2fo = trgInfo->gets2fo();
  int s2f30 = trgInfo->gets2f30();
  int s2s30 = trgInfo->gets2s30();
  int bwdsb  = trgInfo->getbwdsb();
  int bwdnb  = trgInfo->getbwdnb();
  int fwdsb  = trgInfo->getfwdsb();
  int fwdnb  = trgInfo->getfwdnb();
  int brlfb  = trgInfo->getbrlfb();
  int brlnb  = trgInfo->getbrlnb();
  int N_IT   = trgInfo->getNinnertrk();
  int i2fo   = trgInfo->geti2fo();
  int n_secl = trgInfo->getNsecl();
  int n_iecl = trgInfo->getNiecl();
  int n_seklm = trgInfo->getNsklm();
  int n_ieklm = trgInfo->getNiklm();

  //---------------------------------------------------------------------
  //..Filling InputBits
  //..Naming is based on trg/gdl/dbobjects/log/


  if (!m_InputBitsDB)B2INFO("no database of gdl input bits");

  int N_InputBits = m_InputBitsDB->getninbit();

  for (int i = 0; i < N_InputBits; i++) {
    std::string bitname(m_InputBitsDB->getinbitname(i));

    bool bit = false;
    if (bitname == "t3_0") {bit = nTrkZ35 == 1;}
    else if (bitname == "t3_1") {bit = nTrkZ35 == 2;}
    else if (bitname == "t3_2") {bit = nTrkZ35 == 3;}
    else if (bitname == "t3_3") {bit = nTrkZ35 > 3;}
    else if (bitname == "ty_0") {bit = nTrkNN20 == 1;}
    else if (bitname == "ty_1") {bit = nTrkNN20 == 2;}
    else if (bitname == "ty_2") {bit = nTrkNN20 == 3;}
    else if (bitname == "ty_3") {bit = nTrkNN20 > 3;}
    else if (bitname == "typ")  {bit = nTrkNNSTT > 0;}
    else if (bitname == "t2_0") {bit = nTrk2D == 1;}
    else if (bitname == "t2_1") {bit = nTrk2D == 2;}
    else if (bitname == "t2_2") {bit = nTrk2D == 3;}
    else if (bitname == "t2_3") {bit = nTrk2D > 3;}
    else if (bitname == "ts_0") {bit = N_ST == 1;}
    else if (bitname == "ts_1") {bit = N_ST == 2;}
    else if (bitname == "ts_2") {bit = N_ST == 3;}
    else if (bitname == "ts_3") {bit = N_ST > 3;}
    else if (bitname == "fwd_s") {bit = N_ST_fwd > 0;}
    else if (bitname == "bwd_s") {bit = N_ST_bwd > 0;}
    else if (bitname == "cdc_open90") {bit = Trk_open90 == 1;}
    else if (bitname == "f2f30") {bit = Trk_open30 == 1;}
    else if (bitname == "cdc_active") {bit = cdc_active;}
    else if (bitname == "cdc_b2b3") {bit = Trk_b2b_1to3;}
    else if (bitname == "cdc_b2b5") {bit = Trk_b2b_1to5;}
    else if (bitname == "cdc_b2b7") {bit = Trk_b2b_1to7;}
    else if (bitname == "cdc_b2b9") {bit = Trk_b2b_1to9;}
    else if (bitname == "s2s3") {bit = s2s3 > 0;}
    else if (bitname == "s2s5") {bit = s2s5 > 0;}
    else if (bitname == "s2so") {bit = s2so > 0;}
    else if (bitname == "s2f3") {bit = s2f3 > 0;}
    else if (bitname == "s2f5") {bit = s2f5 > 0;}
    else if (bitname == "s2fo") {bit = s2fo > 0;}
    else if (bitname == "s2f30") {bit = s2f30;}
    else if (bitname == "s2s30") {bit = s2s30;}
    else if (bitname == "bwdsb") {bit = bwdsb > 0;}
    else if (bitname == "bwdnb") {bit = bwdnb > 0;}
    else if (bitname == "fwdsb") {bit = fwdsb > 0;}
    else if (bitname == "fwdnb") {bit = fwdnb > 0;}
    else if (bitname == "brlfb1") {bit = brlfb == 1;}
    else if (bitname == "brlfb2") {bit = brlfb == 2;}
    else if (bitname == "brlnb1") {bit = brlnb == 1;}
    else if (bitname == "brlnb2") {bit = brlnb == 2;}
    else if (bitname == "seklm_0") {bit = n_seklm == 0;}
    else if (bitname == "seklm_1") {bit = n_seklm > 0;}
    else if (bitname == "ieklm") {bit = n_ieklm > 0;}
    else if (bitname == "secl") {bit = n_secl > 0;}
    else if (bitname == "iecl") {bit = n_iecl > 0;}
    else if (bitname == "ti") {bit = N_IT > 0;}
    else if (bitname == "i2fo") {bit = i2fo > 0;}
    else if (bitname == "ehigh") {bit = ehigh;}
    else if (bitname == "elow") {bit = elow;}
    else if (bitname == "elum") {bit = elum;}
    else if (bitname == "ecl_bha") {bit = ecl_bha;}
    else if (bitname == "bha_type_0") {bit = bha_type0;}
    else if (bitname == "bha_type_1") {bit = bha_type1;}
    else if (bitname == "bha_type_2") {bit = bha_type2;}
    else if (bitname == "bha_type_3") {bit = bha_type3;}
    else if (bitname == "bha_type_4") {bit = bha_type4;}
    else if (bitname == "bha_type_5") {bit = bha_type5;}
    else if (bitname == "bha_type_6") {bit = bha_type6;}
    else if (bitname == "bha_type_7") {bit = bha_type7;}
    else if (bitname == "bha_type_8") {bit = bha_type8;}
    else if (bitname == "bha_type_9") {bit = bha_type9;}
    else if (bitname == "bha_type_10") {bit = bha_type10;}
    else if (bitname == "bha_type_11") {bit = bha_type11;}
    else if (bitname == "bha_type_12") {bit = bha_type12;}
    else if (bitname == "bha_type_13") {bit = bha_type13;}
    else if (bitname == "clst_0") {bit = nclst_0;}
    else if (bitname == "clst_1") {bit = nclst_1;}
    else if (bitname == "clst_2") {bit = nclst_2;}
    else if (bitname == "clst_3") {bit = nclst_3;}
    else if (bitname == "ecl_bg_0") {bit = ecl_bg_0;}
    else if (bitname == "ecl_bg_1") {bit = ecl_bg_1;}
    else if (bitname == "ecl_bg_2") {bit = ecl_bg_2;}
    else if (bitname == "ecl_active") {bit = ecl_active;}
    else if (bitname == "ecl_timing_fwd") {bit = ecl_timing_fwd;}
    else if (bitname == "ecl_timing_brl") {bit = ecl_timing_brl;}
    else if (bitname == "ecl_timing_bwd") {bit = ecl_timing_bwd;}
    else if (bitname == "ecl_phys") {bit = ecl_phys;}
    else if (bitname == "ecl_oflo") {bit = ecl_oflo;}
    else if (bitname == "ecl_3dbha") {bit = ecl_3dbha;}
    else if (bitname == "bha_veto") {bit = ecl_3dbha;}
    else if (bitname == "ecl_lml_0") {bit = ecl_lml_0;}
    else if (bitname == "ecl_lml_1") {bit = ecl_lml_1;}
    else if (bitname == "ecl_lml_2") {bit = ecl_lml_2;}
    else if (bitname == "ecl_lml_3") {bit = ecl_lml_3;}
    else if (bitname == "ecl_lml_4") {bit = ecl_lml_4;}
    else if (bitname == "ecl_lml_5") {bit = ecl_lml_5;}
    else if (bitname == "ecl_lml_6") {bit = ecl_lml_6;}
    else if (bitname == "ecl_lml_7") {bit = ecl_lml_7;}
    else if (bitname == "ecl_lml_8") {bit = ecl_lml_8;}
    else if (bitname == "ecl_lml_9") {bit = ecl_lml_9;}
    else if (bitname == "ecl_lml_10") {bit = ecl_lml_10;}
    else if (bitname == "ecl_lml_11") {bit = ecl_lml_11;}
    else if (bitname == "ecl_lml_12") {bit = ecl_lml_12;}
    else if (bitname == "ecl_lml_13") {bit = ecl_lml_13;}
    else if (bitname == "ecl_mumu") {bit = ecl_mumu;}
    else if (bitname == "ecl_bst") {bit = ecl_bst;}
    else if (bitname == "top_0") {bit = false;}
    else if (bitname == "top_1") {bit = false;}
    else if (bitname == "top_2") {bit = false;}
    else if (bitname == "top_bb") {bit = false;}
    else if (bitname == "top_active") {bit = false;}
    else if (bitname == "klm_hit") {bit = klm_hit;}
    else if (bitname == "klm_0") {bit = klm_0;}
    else if (bitname == "klm_1") {bit = klm_1;}
    else if (bitname == "klm_2") {bit = klm_2;}
    else if (bitname == "klmb2b_0") {bit = klmb2b_0;}
    else if (bitname == "klmb2b_1") {bit = klmb2b_1;}
    else if (bitname == "klmb2b_2") {bit = klmb2b_2;}
    else if (bitname == "revo") {bit = false;}
    else if (bitname == "her_kick") {bit = false;}
    else if (bitname == "ler_kick") {bit = false;}
    else if (bitname == "bha_delay") {bit = false;}
    else if (bitname == "pseud_rand") {bit = false;}
    else if (bitname == "plsin") {bit = false;}
    else if (bitname == "poissonin") {bit = false;}
    else if (bitname == "periodin") {bit = false;}
    else if (bitname == "veto") {bit = false;}
    else if (bitname == "samhem") {bit = nSameHem1Trk > 0;}
    else if (bitname == "opohem") {bit = nOppHem1Trk > 0;}
    else if (bitname == "n1gev_0") {bit = nclst1_0;}
    else if (bitname == "n1gev_1") {bit = nclst1_1;}
    else if (bitname == "n1gev_2") {bit = nclst1_2;}
    else if (bitname == "n1gev_3") {bit = nclst1_3;}
    else if (bitname == "n2gev_0") {bit = nclst2_0;}
    else if (bitname == "n2gev_1") {bit = nclst2_1;}
    else if (bitname == "n2gev_2") {bit = nclst2_2;}
    else if (bitname == "n2gev_3") {bit = nclst2_3;}
    else if (bitname == "cdcecl_0") {bit = cdcecl_0;}
    else if (bitname == "cdcecl_1") {bit = cdcecl_1;}
    else if (bitname == "cdcecl_2") {bit = cdcecl_2;}
    else if (bitname == "cdcecl_3") {bit = cdcecl_3;}
    else if (bitname == "c2gev_0") {bit = c2gev_0;}
    else if (bitname == "c2gev_1") {bit = c2gev_1;}
    else if (bitname == "c2gev_2") {bit = c2gev_2;}
    else if (bitname == "c2gev_3") {bit = c2gev_3;}
    else if (bitname == "cdcklm_0") {bit = cdcklm_0;}
    else if (bitname == "cdcklm_1") {bit = cdcklm_1;}
    else if (bitname == "cdcklm_2") {bit = cdcklm_2;}
    else if (bitname == "cdcklm_3") {bit = cdcklm_3;}
    else if (bitname == "d3") {bit = cluster_b2b_1to3 > 0;}
    else if (bitname == "d5") {bit = cluster_b2b_1to5 > 0;}
    else if (bitname == "d7") {bit = cluster_b2b_1to7 > 0;}
    else if (bitname == "d9") {bit = cluster_b2b_1to9 > 0;}
    else if (bitname == "p3") {bit = Trkcluster_b2b_1to3 > 0;}
    else if (bitname == "p5") {bit = Trkcluster_b2b_1to5 > 0;}
    else if (bitname == "p7") {bit = Trkcluster_b2b_1to7 > 0;}
    else if (bitname == "p9") {bit = Trkcluster_b2b_1to9 > 0;}
    else if (bitname == "track") {bit = false;}
    else if (bitname == "trkfit") {bit = false;}


    //GRL related bits, not ready
    else if (bitname == "ta_0") {bit = false;}
    else if (bitname == "ta_1") {bit = false;}
    else if (bitname == "ta_2") {bit = false;}
    else if (bitname == "ta_3") {bit = false;}
    else if (bitname == "trkflt") {bit = false;}
    else if (bitname == "tsf0b2b") {bit = false;}
    else if (bitname == "tsf1b2b") {bit = false;}
    else if (bitname == "tsf2b2b") {bit = false;}
    else if (bitname == "trkbha1") {bit = false;}
    else if (bitname == "trkbha2") {bit = false;}
    else if (bitname == "grlgg1") {bit = false;}
    else if (bitname == "grlgg2") {bit = false;}

    //KLM TOP ECL not ready
    else if (bitname == "ecl_bhauni") {bit = false;}
    else if (bitname == "ecl_bhapur") {bit = false;}
    else if (bitname == "klmb2b") {bit = false;}
    else if (bitname == "eklm_hit") {bit = false;}
    else if (bitname == "eklm_0") {bit = false;}
    else if (bitname == "eklm_1") {bit = false;}
    else if (bitname == "eklm_2") {bit = false;}
    else if (bitname == "eklmb2b") {bit = false;}
    else if (bitname == "cdctop_0") {bit = false;}
    else if (bitname == "cdctop_1") {bit = false;}
    else if (bitname == "cdctop_2") {bit = false;}
    else if (bitname == "cdctop_3") {bit = false;}

    //random trigger
    else if (bitname == "injv") {bit = false;}
    else if (bitname == "nimin0") {bit = false;}
    else if (bitname == "nimin1") {bit = false;}
    else if (bitname == "inp159") {bit = false;}

    //other trigger bits
    else if (bitname == "itsfb2b") {bit = false;}

    //DITTO: please don't change the WARNING message below.
    //If you change it, please update the test trg_tsim_check_warnings.py accordingly.
    else B2WARNING("Unknown bitname" << LogVar("bitname", bitname));

    trgInfo->setInputBits(i, bit);
  }

}

void
TRGGRLProjectsModule::endRun()
{
  B2DEBUG(20, "TRGGRLProjectsModule ... endRun called ");
}



void TRGGRLProjectsModule::terminate()
{

}


