//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGRLProjectsModule.cc
// Section  : TRG GRL
// Owner    : Yoshihito Iwasaki, Junhao Yin
// Email    : yoshihito.iwasaki@kek.jp, yinjh@ihep.ac.cn
//-----------------------------------------------------------------------------
// Description : A trigger module for GRL
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------


#include <stdlib.h>
#include <iostream>

#include <trg/trg/Debug.h>
#include <trg/grl/modules/trggrl/TRGGRLProjectsModule.h>
#include <trg/grl/dataobjects/TRGGRLMATCH.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>
#include <framework/gearbox/Unit.h>
//#include "trg/cdc/dataobjects/CDCTriggerTrack.h"
#include <trg/grl/dataobjects/TRGGRLMATCH.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>
#include <trg/ecl/dataobjects/TRGECLTrg.h>
#include <trg/klm/dataobjects/KLMTriggerHit.h>
#include <trg/klm/dataobjects/KLMTriggerTrack.h>

using namespace std;

namespace Belle2 {

  /** Register module for TRGGRL */
  REG_MODULE(TRGGRLProjects);

  string
  TRGGRLProjectsModule::version() const
  {
    return string("TRGGRLProjectsModule 0.00");
  }

  TRGGRLProjectsModule::TRGGRLProjectsModule(): Module()
  {

    string desc = "TRGGRLProjectsModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("DebugLevel", _debugLevel, "TRGGRL debug level", _debugLevel);
    addParam("ConfigFile",
             _configFilename,
             "The filename of CDC trigger config file",
             _configFilename);
    addParam("SimulationMode",
             _simulationMode,
             "TRGGRL simulation switch",
             _simulationMode);
    addParam("FastSimulationMode",
             _fastSimulationMode,
             "TRGGRL fast simulation mode",
             _fastSimulationMode);
    addParam("FirmwareSimulationMode",
             _firmwareSimulationMode,
             "TRGGRL firmware simulation mode",
             _firmwareSimulationMode);
    addParam("2DfinderCollection", m_2DfinderCollectionName,
             "Name of the StoreArray holding the tracks made by the 2D finder to be used as input.",
             string("Trg2DFinderTracks"));
    addParam("2DfitterCollection", m_2DfitterCollectionName,
             "Name of the StoreArray holding the tracks made by the 2D fitter to be used as input.",
             string("Trg2DFitterTracks"));
    addParam("3DfitterCollection", m_3DfitterCollectionName,
             "Name of the StoreArray holding the tracks made by the 3D fitter to be used as input.",
             string("Trg3DFitterTracks"));
    addParam("NNCollection", m_NNCollectionName,
             "Name of the StoreArray holding the tracks made by the neural network (NN).",
             string("TrgNNTracks"));
    addParam("2DmatchCollection", m_2DmatchCollectionName,
             "Name of the StoreArray holding the macthed tracks and clusters made by the 2D fitter.",
             string("Trg2DMatchTracks"));
    addParam("3DmatchCollection", m_3DmatchCollectionName,
             "Name of the StoreArray holding the matched 3D NN tracks and clusters made",
             string("Trg3DMatchTracks"));
    addParam("TrgGrlInformation", m_TrgGrlInformationName,
             "Name of the StoreArray holding the information of tracks and clusters from cdc ecl klm.",
             string("TrgGrlInformation"));
    addParam("TRGECLClusters", m_TrgECLClusterName,
             "Name of the StoreArray holding the information of trigger ecl clusters ",
             string("TRGECLClusters"));
    addParam("TRGECLTrgs", m_TrgECLTrgsName,
             "Name of the StoreArray holding the information of ecl trigger",
             string("TRGECLTrgs"));
//    addParam("KLM", m_KLM,
//             "Name of the StoreArray holding the information of trigger ecl clusters ",
//            string("KLMHits"));
    addParam("ECLClusterTimeWindow", m_eclClusterTimeWindow,
             "The time window of the signal eclclusters",
             100.0);
    std::vector<double> thre;
    thre.push_back(0.1);
    thre.push_back(0.3);
    thre.push_back(1.0);
    addParam("ClusEngThreshold", m_energythreshold, "The energy threshold of clusters", thre);

    if (TRGDebug::level())
      cout << "TRGGRLProjectsModule ... created" << endl;
  }

  TRGGRLProjectsModule::~TRGGRLProjectsModule()
  {
  }

  void
  TRGGRLProjectsModule::initialize()
  {

//  TRGDebug::level(_debugLevel);

    if (TRGDebug::level()) {
      cout << "TRGGRLProjectsModule::initialize ... options" << endl;
      cout << TRGDebug::tab(4) << "debug level = " << TRGDebug::level()
           << endl;
    }

    StoreArray<CDCTriggerTrack>::required(m_2DfinderCollectionName);
    StoreArray<CDCTriggerTrack>::required(m_2DfitterCollectionName);
    StoreArray<CDCTriggerTrack>::required(m_3DfitterCollectionName);
    StoreArray<CDCTriggerTrack>::required(m_NNCollectionName);
    StoreArray<TRGGRLMATCH>::required(m_2DmatchCollectionName);
    StoreArray<TRGGRLMATCH>::required(m_3DmatchCollectionName);
    StoreArray<TRGECLCluster>::required(m_TrgECLClusterName);
    StoreArray<TRGECLTrg>::required(m_TrgECLTrgsName);

    StoreArray<KLMTriggerHit>::required();
    StoreArray<KLMTriggerTrack>::required();

    StoreArray<TRGGRLInfo>::registerPersistent();

  }

  void
  TRGGRLProjectsModule::beginRun()
  {

    //...GDL config. name...
    string cfn = _configFilename;

    if (TRGDebug::level()) {
      cout << "TRGGDLModule ... beginRun called " << endl;
      cout << "                 configFile = " << cfn << endl;
    }
  }

  void
  TRGGRLProjectsModule::event()
  {
    TRGDebug::enterStage("TRGGRLProjectsModule event");
    StoreArray<CDCTriggerTrack> tracks2Dfinder(m_2DfinderCollectionName);
    StoreArray<CDCTriggerTrack> tracks2Dfitter(m_2DfitterCollectionName);
    StoreArray<CDCTriggerTrack> tracks3Dfitter(m_3DfitterCollectionName);
    StoreArray<CDCTriggerTrack> tracksNN(m_NNCollectionName);
    StoreArray<TRGGRLMATCH>     tracks2Dmatch(m_2DmatchCollectionName);
    StoreArray<TRGGRLMATCH>     tracks3Dmatch(m_3DmatchCollectionName);
    StoreArray<TRGECLCluster> eclclusters(m_TrgECLClusterName);
    StoreArray<TRGECLTrg> ecltrgs;
    StoreArray<KLMTriggerTrack> klmtrk;
    StoreArray<KLMTriggerHit> klmhit;
    int ntrk_2dfinder = tracks2Dfinder.getEntries();
    int ntrk_2dfitter = tracks2Dfitter.getEntries();
    int ntrk_3dfitter = tracks3Dfitter.getEntries();
    int ntrk_NN = tracksNN.getEntries();
    int ntrk_mat2d = tracks2Dmatch.getEntries();
    int ntrk_mat3d = tracks3Dmatch.getEntries();
    int n_klmtrk = klmtrk.getEntries();
    int n_klmhit = klmhit.getEntries();
    //number of ecl clusters without time requirement
    // int ncluster_ori = eclclusters.getEntries();
    StoreArray<TRGGRLInfo> trgInfos;
    TRGGRLInfo* trgInfo = trgInfos.appendNew();
    trgInfo->setN2Dfindertrk(ntrk_2dfinder);
    trgInfo->setN2Dfittertrk(ntrk_2dfitter);
    trgInfo->setN3Dfittertrk(ntrk_3dfitter);
    trgInfo->setNNNtrk(ntrk_NN);
    trgInfo->setN2Dmatchtrk(ntrk_mat2d);
    trgInfo->setN3Dmatchtrk(ntrk_mat3d);
    trgInfo->setNklmtrk(n_klmtrk);
    trgInfo->setNklmhit(n_klmhit);
    //trgInfo->setNcluster(ncluster);

    TRGDebug::leaveStage("TRGGRLProjectsModule event");

    //get the clusters list in Time Window to reduce beam induced background
    std::vector<TRGECLCluster*> clustersinwindow;
    ///get event time for ECL firstly
    double eventtime = -999999.;
    double energytot = 9999.;
    for (int i = 0; i < ecltrgs.getEntries(); i++) {
      double evt_time = ecltrgs[i]->m_eventtiming;
      double evt_energy = ecltrgs[i]->m_etot;
      if (energytot > evt_energy) {
        energytot = evt_energy;
        eventtime = evt_time;
      }
    }

    for (int i = 0; i < eclclusters.getEntries(); i++) {
      double ctime = eclclusters[i]->getTimeAve() - eventtime;
      if (fabs(ctime) < m_eclClusterTimeWindow) clustersinwindow.push_back(eclclusters[i]);
    }
    //get the number of clusers with specific threshold in the time window
    int nclus[3] = {0, 0, 0};
    for (unsigned int i = 0; i < clustersinwindow.size(); i++) {
      double energy_clu = clustersinwindow[i]->getEnergyDep();
      if (energy_clu > m_energythreshold[0]) nclus[0]++;
      if (energy_clu > m_energythreshold[1]) nclus[1]++;
      if (energy_clu > m_energythreshold[2]) {
        double  x1 = clustersinwindow[i]->getPositionX();
        double  y1 = clustersinwindow[i]->getPositionY();
        double  z1 = clustersinwindow[i]->getPositionZ();
        TVector3 vec1(x1, y1, z1);
        double  theta1 = vec1.Theta() * Unit::deg;
        if (theta1 > 30. && theta1 < 140.)nclus[2]++;
      }

    }
    trgInfo->setNhighcluster1(nclus[0]);
    trgInfo->setNhighcluster2(nclus[1]);
    trgInfo->setNneucluster(nclus[2]);

    //Bhabha----------------begin
    int bhabha_bit = 0;
    if (ntrk_mat3d >= 2) {
      for (int i = 0; i < ntrk_mat3d - 1; i++) {
        const TRGECLCluster* eclcluster1 = tracks3Dmatch[i]->getRelatedTo<TRGECLCluster>();
        const CDCTriggerTrack* cdctrk1 = tracks3Dmatch[i]->getRelatedTo<CDCTriggerTrack>();
        if (eclcluster1 == nullptr || cdctrk1 == nullptr) continue;
        double e1 = eclcluster1->getEnergyDep();
        double phi1 = cdctrk1->getPhi0() * Unit::deg;
        double tanLam1 = cdctrk1->getTanLambda();
        double theta1 = acos(tanLam1 / sqrt(1. + tanLam1 * tanLam1)) * Unit::deg;

        for (int j = i; j < ntrk_mat3d; j++) {
          const TRGECLCluster* eclcluster2 = tracks3Dmatch[j]->getRelatedTo<TRGECLCluster>();
          const CDCTriggerTrack* cdctrk2 = tracks3Dmatch[j]->getRelatedTo<CDCTriggerTrack>();
          if (eclcluster2 == nullptr || cdctrk2 == nullptr) continue;
          double e2 = eclcluster2->getEnergyDep();
          double phi2 = cdctrk2->getPhi0() * Unit::deg;
          double tanLam2 = cdctrk2->getTanLambda();
          double theta2 = acos(tanLam2 / sqrt(1. + tanLam2 * tanLam2)) * Unit::deg;

          double deltphi = fabs(fabs(phi1 - phi2) - 180.);
          double delttheta = fabs(theta1 + theta2 - 180.);
          bool ang = (deltphi < 20. && delttheta > 10. && deltphi < 50.);
          bool eng = ((e1 > 3.0 && e2 > 2.0) || (e1 > 2.0 && e2 > 3.0)) && (e1 + e2 > 6.0);
          if (ang && eng)bhabha_bit = 1;
        }
      }
    }
    trgInfo->setBhabhaVeto(bhabha_bit);
//Bhabha--------------end

//eclBhabhaVeto--------begin
//count the back-to-back cluster pair here as well
    int eclbhabha_bit = 0;
    int nbb_cluster = 0;
    if (clustersinwindow.size() >= 2) {
      for (unsigned i = 0; i < clustersinwindow.size() - 1; i++) {

        TRGECLCluster* cluster1 = clustersinwindow[i];
        double e1 = cluster1->getEnergyDep();
        double  x1 = cluster1->getPositionX();
        double  y1 = cluster1->getPositionY();
        double  z1 = cluster1->getPositionZ();
        TVector3 vec1(x1, y1, z1);
        double  theta1 = vec1.Theta() * Unit::deg;
        double  phi1 = vec1.Phi() * Unit::deg;
        for (unsigned j = i; j < clustersinwindow.size(); j++) {
          TRGECLCluster* cluster2 = clustersinwindow[j];
          double e2 = cluster2->getEnergyDep();
          double  x2 = cluster2->getPositionX();
          double  y2 = cluster2->getPositionY();
          double  z2 = cluster2->getPositionZ();
          TVector3 vec2(x2, y2, z2);
          double  theta2 = vec2.Theta() * Unit::deg;
          double  phi2 = vec2.Phi() * Unit::deg;

          double etot = e1 + e2;
          double deltphi = fabs(fabs(phi1 - phi2) - 180);
          double delttheta = fabs(theta1 + theta2 - 180);
          bool ang = (delttheta < 50. && deltphi < 50.);
          bool eng = (etot > 6.0 && (e1 > 3.0 || e2 > 3.0) && (e1 > 2.0 && e2 > 2.0));
          if (ang && eng) eclbhabha_bit = 1;

          if (deltphi < 100. && delttheta < 100.) nbb_cluster++;
        }
      }
    }
    trgInfo->seteclBhabhaVeto(eclbhabha_bit);
    trgInfo->setNbbCluster(nbb_cluster);
//eclBhabhaVeto---------end

//single track bahbhaveto-------begin
    int sbhabha_bit = 0;
    for (int i = 0; i < ntrk_mat3d; i++) {
      const TRGECLCluster* eclcluster1 = tracks3Dmatch[i]->getRelatedTo<TRGECLCluster>();
      const CDCTriggerTrack* cdctrk1 = tracks3Dmatch[i]->getRelatedTo<CDCTriggerTrack>();
      if (eclcluster1 == nullptr || cdctrk1 == nullptr) continue;
      double e1 = eclcluster1->getEnergyDep();
      if (e1 > 1.0 && eclbhabha_bit == 1)sbhabha_bit = 1;
    }
    trgInfo->setsBhabhaVeto(sbhabha_bit);
//single track bahbhaveto-------end

//the neutral cluster counting
    int nneu_cluster = 0;
    if (!clustersinwindow.empty()) {
      for (unsigned int i = 0; i < clustersinwindow.size(); i++) {
        if (clustersinwindow[i]->getRelatedTo<CDCTriggerTrack>()) continue;
        else nneu_cluster++;
      }
    }
    trgInfo->setNneucluster(nneu_cluster);

//bhabha accept 1
//bhabha accept 2
//gg accept 1
//gg accept 2


  }

  void
  TRGGRLProjectsModule::endRun()
  {
    if (TRGDebug::level())
      cout << "TRGGRLProjectsModule ... endRun called " << endl;
  }


  void
  TRGGRLProjectsModule::terminate()
  {
    if (TRGDebug::level())
      cout << "TRGGRLProjectsModule ... terminate called " << endl;
  }


} // namespace Belle2
