/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/ecl/modules/trgeclDQM/TRGECLDQMModule.h>
#include <trg/ecl/TrgEclMapping.h>
#include <trg/ecl/TrgEclCluster.h>
#include <trg/ecl/TrgEclDataBase.h>

#include <framework/datastore/StoreArray.h>

#include <TDirectory.h>

using namespace Belle2;

REG_MODULE(TRGECLDQM);


TRGECLDQMModule::TRGECLDQMModule() : HistoModule()
{


  setDescription("DQM for ECL Trigger system");
  setPropertyFlags(c_ParallelProcessingCertified);

  TCId.clear();
  TCEnergy.clear();
  TCTiming.clear();
  RevoFAM.clear();
  RevoTrg.clear();
  FineTiming.clear();

}


TRGECLDQMModule::~TRGECLDQMModule()
{

}


void TRGECLDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  TDirectory* dirDQM = (TDirectory*)gDirectory->Get("TRG");
  if (!dirDQM) {
    dirDQM = oldDir->mkdir("TRG");
  }
  dirDQM->cd();

  h_TCId           = new TH1D("h_TCId",          "Hit TC ID",                   578, 0, 578);
  h_TCthetaId      = new TH1D("h_TCthetaId",     "Hit TC #theta ID",             19, 0, 19);
  h_TCphiId_FWD    = new TH1D("h_TCphiId_FWD",   "Hit TC #phi ID in FWD",        34, 0, 34);
  h_TCphiId_BR     = new TH1D("h_TCphiId_BR",    "Hit TC #phi ID in BR",         38, 0, 38);
  h_TCphiId_BWD    = new TH1D("h_TCphiId_BWD",   "Hit TC #phi ID in BWD",        34, 0, 34);
  h_TotalEnergy    = new TH1D("h_TotalEnergy",   "Total TC Energy (ADC)",       100, 0, 3000);
  h_TCEnergy       = new TH1D("h_TCEnergy",      "TC Energy (ADC)",     100, 0, 1500);
  h_Narrow_TotalEnergy    = new TH1D("h_Narrow_TotalEnergy",   "Total TC Energy (ADC)",       100, 0, 500);
  h_Narrow_TCEnergy       = new TH1D("h_Narrow_TCEnergy",      "TC Energy (ADC)",     100, 0, 100);
  h_n_TChit_event  = new TH1D("h_n_TChit_event", "N(TC) ",                40, 0, 40);
  h_Cluster        = new TH1D("h_Cluster",       "N(Cluster) ",           20, 0, 20);
  h_TCTiming       = new TH1D("h_TCTiming",      "TC Timing  (ns)",      100, 3010, 3210);
  h_TRGTiming      = new TH1D("h_TRGTiming",     "TRG Timing  (ns)",     100, 3010, 3210);
  h_Cal_TCTiming       = new TH1D("h_Cal_TCTiming",      "Cal TC Timing  (ns)",      100, -400, 400);
  h_Cal_TRGTiming      = new TH1D("h_Cal_TRGTiming",     "TRG Timing  (ns)",     100, -400, 400);
  h_ECL_TriggerBit      = new TH1D("h_ECL_TriggerBit",     "ECL Trigger Bit",     29, 0, 29);
  h_Cluster_Energy_Sum    = new TH1D("h_Cluster_Energy_Sum",   "Energy Sum of 2 Clusters (ADC)",       300, 0, 3000);

  oldDir->cd();
}


void TRGECLDQMModule::initialize()
{

  // calls back the defineHisto() function, but the HistoManager module has to be in the path
  REG_HISTOGRAM

  trgeclHitArray.registerInDataStore();
  trgeclEvtArray.registerInDataStore();
  trgeclCluster.registerInDataStore();
  trgeclSumArray.registerInDataStore();

}


void TRGECLDQMModule::beginRun()
{
}

void TRGECLDQMModule::endRun() { } void TRGECLDQMModule::terminate()
{
  //    delete h_TCId;
}

void TRGECLDQMModule::event()
{
  TCId.clear();
  TCEnergy.clear();
  TCTiming.clear();
  RevoFAM.clear();
  RevoTrg.clear();
  FineTiming.clear();

  //    StoreArray<TRGECLUnpackerStore> trgeclHitArray;
  /* cppcheck-suppress variableScope */
  double HitTiming;
  /* cppcheck-suppress variableScope */
  double HitEnergy;
  double HitRevoFam = 0;
  double HitRevoTrg = 0;
  double HitFineTiming = 0;
  double HitRevoEvtTiming = 0;
  double HitCalTiming = 0;
  int CheckSum = 0;

  for (int iii = 0; iii < trgeclEvtArray.getEntries(); iii++) {
    TRGECLUnpackerEvtStore* aTRGECLUnpackerEvtStore = trgeclEvtArray[iii];

    HitFineTiming = aTRGECLUnpackerEvtStore ->  getEvtTime();
    HitRevoTrg = aTRGECLUnpackerEvtStore -> getL1Revo();
    HitRevoEvtTiming = aTRGECLUnpackerEvtStore -> getEvtRevo();
    CheckSum =  aTRGECLUnpackerEvtStore -> getEvtExist() ;


    RevoTrg.push_back(HitRevoTrg);



  }
  if (CheckSum == 0) {return;}



  for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {
    TRGECLUnpackerStore* aTRGECLUnpackerStore = trgeclHitArray[ii];
    int TCID = (aTRGECLUnpackerStore->getTCId());
    int hit_win =  aTRGECLUnpackerStore -> getHitWin();
    HitEnergy =  aTRGECLUnpackerStore -> getTCEnergy();
    HitTiming    = aTRGECLUnpackerStore ->getTCTime();

    if (TCID < 1 || TCID > 576 || HitEnergy == 0) {continue;}
    if (!(hit_win == 3 || hit_win == 4)) {continue;}
    HitCalTiming = aTRGECLUnpackerStore ->getTCCALTime() ;
    HitRevoFam = aTRGECLUnpackerStore-> getRevoFAM() ;

    TCId.push_back(TCID);
    TCEnergy.push_back(HitEnergy);
    TCTiming.push_back(HitTiming);
    RevoFAM.push_back(HitRevoFam);
    FineTiming.push_back(HitCalTiming);
  }
  //
  //
  if (TCId.size() == 0) {return;}

  /* cppcheck-suppress variableScope */
  int phy;
  /* cppcheck-suppress variableScope */
  int b1;
  /* cppcheck-suppress variableScope */
  int b2v;
  /* cppcheck-suppress variableScope */
  int b2s;
  /* cppcheck-suppress variableScope */
  int mu;
  /* cppcheck-suppress variableScope */
  int pre;
  /* cppcheck-suppress variableScope */
  int clover;
  /* cppcheck-suppress variableScope */
  int tsource;
  /* cppcheck-suppress variableScope */
  int b1type;
  /* cppcheck-suppress variableScope */
  int etot;
  /* cppcheck-suppress variableScope */
  int vlm;
  /* cppcheck-suppress variableScope */
  int eclburst;
  //  int s_hit_win= 0;
  std::vector<int> trgbit ;
  trgbit.resize(44, 0);
  for (int iii = 0; iii < trgeclSumArray.getEntries(); iii++) {
    TRGECLUnpackerSumStore* aTRGECLUnpackerSumStore = trgeclSumArray[iii];

    tsource = aTRGECLUnpackerSumStore ->getTimeType();
    phy     = aTRGECLUnpackerSumStore ->getPhysics();
    b1      = aTRGECLUnpackerSumStore ->get2DBhabha();
    b1type  = aTRGECLUnpackerSumStore -> getBhabhaType();
    b2v     = aTRGECLUnpackerSumStore -> get3DBhabhaV();
    b2s     = aTRGECLUnpackerSumStore -> get3DBhabhaS() ;
    etot    = aTRGECLUnpackerSumStore ->  getEtotType();
    clover  = aTRGECLUnpackerSumStore ->  getICNOver();
    vlm     = aTRGECLUnpackerSumStore ->  getLowMulti();
    mu      = aTRGECLUnpackerSumStore ->  getMumu();
    pre     = aTRGECLUnpackerSumStore ->  getPrescale();
    eclburst = aTRGECLUnpackerSumStore -> getECLBST();

    //
    trgbit[0] = 1;
    trgbit[1] = tsource & 0x1;
    trgbit[2] = (tsource >> 1) & 0x1;
    trgbit[3] = (tsource >> 2) & 0x1;
    trgbit[4] = phy;
    trgbit[5] = b1;
    trgbit[6] = b2v;
    trgbit[7] = b2s;
    trgbit[8] = etot & 0x1;
    trgbit[9] = (etot >> 1) & 0x1;
    trgbit[10] = (etot >> 2) & 0x1;
    trgbit[11] = clover;

    for (int j = 0; j < 14; j++) {
      trgbit[12 + j] = (vlm >> j) & 0x1;
    }

    trgbit[26] = mu;
    trgbit[27] = pre;
    trgbit[28] = eclburst;

    trgbit[29] = b1type & 0x1;
    trgbit[30] = (b1type >> 1) & 0x1;
    trgbit[31] = (b1type >> 2) & 0x1;
    trgbit[32] = (b1type >> 3) & 0x1;
    trgbit[33] = (b1type >> 4) & 0x1;
    trgbit[34] = (b1type >> 5) & 0x1;
    trgbit[35] = (b1type >> 6) & 0x1;
    trgbit[36] = (b1type >> 7) & 0x1;
    trgbit[37] = (b1type >> 8) & 0x1;
    trgbit[38] = (b1type >> 9) & 0x1;
    trgbit[39] = (b1type >> 10) & 0x1;
    trgbit[40] = (b1type >> 11) & 0x1;
    trgbit[41] = (b1type >> 12) & 0x1;
    trgbit[42] = (b1type >> 13) & 0x1;
    trgbit[43] = (b1type >> 14) & 0x1;


  }

  const char* label[44] = {"Hit", "Timing Source(FWD)", "Timing Source(BR)", "Timing Source(BWD)", "physics Trigger", "2D Bhabha Veto", "3D Bhabha veto", "3D Bhabha Selection", "E Low", "E High", "E LOM", "Cluster Overflow", "Low multi bit 0", "Low multi bit 1", "Low multi bit 2", "Low multi bit 3", "Low multi bit 4", "Low multi bit 5", "Low multi bit 6", "Low multi bit 7", "Low multi bit 8", "Low multi bit 9", "Low multi bit 10", "Low multi bit 11", "Low multi bit 12", "Low multi bit 13", "mumu bit", "prescale bit", "ECL burst bit" , "2D Bhabha bit 1", "2D Bhabha bit 2"  , "2D Bhabha bit 3", "2D Bhabha bit 4", "2D Bhabha bit 5", "2D Bhabha bit 6", "2D Bhabha bit 7", "2D Bhabha bit 8", "2D Bhabha bit 9", "2D Bhabha bit 10", "2D Bhabha bit 11", "2D Bhabha bit 12", "2D Bhabha bit 13", "2D Bhabha bit 14"};



  for (int j = 0; j < 29; j++) {
    if (trgbit[j] == 0x1) {h_ECL_TriggerBit->Fill(j, 1);}
    h_ECL_TriggerBit->GetXaxis()-> SetBinLabel(j + 1, label[j]);

  }
  h_ECL_TriggerBit->SetStats(0);



  //----------------------
  //Clustering
  //----------------------
  //

  TrgEclCluster  _TCCluster ;
  _TCCluster.setICN(TCId, TCEnergy, TCTiming);

  int c = _TCCluster.getNofCluster();
  h_Cluster->Fill(c);
  std::vector<double> ClusterTiming;
  std::vector<double> ClusterEnergy;
  std::vector<int> MaxTCId;
  ClusterTiming.clear();
  ClusterEnergy.clear();
  MaxTCId.clear();

  for (int iii = 0; iii < trgeclCluster.getEntries(); iii++) {
    TRGECLCluster* aTRGECLCluster = trgeclCluster[iii];
    int maxTCId    = aTRGECLCluster ->getMaxTCId();
    double clusterenergy  = aTRGECLCluster ->getEnergyDep();
    double clustertiming  =  aTRGECLCluster -> getTimeAve();
    ClusterTiming.push_back(clustertiming);
    ClusterEnergy.push_back(clusterenergy);
    MaxTCId.push_back(maxTCId);
  }


  std::vector<double> maxClusterEnergy;
  std::vector<double> maxClusterTiming;
  std::vector<int> maxCenterTCId;
  maxClusterTiming.clear();
  maxClusterEnergy.clear();
  maxCenterTCId.clear();

  maxClusterEnergy.resize(2, 0.0);
  maxClusterTiming.resize(2, 0.0);
  maxCenterTCId.resize(2, 0.0);
  const int cl_size = ClusterEnergy.size();
  for (int icl = 0; icl < cl_size; icl++) {
    if (maxClusterEnergy[0] < ClusterEnergy[icl]) {
      maxClusterEnergy[0] = ClusterEnergy[icl];
      maxClusterTiming[0] = ClusterTiming[icl];
      maxCenterTCId[0] = MaxTCId[icl];
    } else if (maxClusterEnergy[1] < ClusterEnergy[icl]) {
      maxClusterEnergy[1] = ClusterEnergy[icl];
      maxClusterTiming[1] = ClusterTiming[icl];
      maxCenterTCId[1] = MaxTCId[icl];

    }

  }
  TrgEclDataBase _database;

  std::vector<double> _3DBhabhaThreshold;
  _3DBhabhaThreshold = {30, 45}; //  /10 MeV


  bool BtoBFlag = false;
  bool BhabhaFlag = false;
  int lut1 = _database.Get3DBhabhaLUT(maxCenterTCId[0]);
  int lut2 = _database.Get3DBhabhaLUT(maxCenterTCId[1]);
  int energy1 = 15 & lut1;
  int energy2 = 15 & lut2;
  lut1 >>= 4;
  lut2 >>= 4;
  int phi1 = 511 & lut1;
  int phi2 = 511 & lut2;
  lut1 >>= 9;
  lut2 >>= 9;
  int theta1 = lut1;
  int theta2 = lut2;
  int dphi = abs(phi1 - phi2);
  if (dphi > 180) {dphi = 360 - dphi;}
  int thetaSum = theta1 + theta2;
  if (dphi > 160 && thetaSum > 165 && thetaSum < 190) {BtoBFlag = true;}

  if ((maxClusterEnergy[0] * 0.1) > _3DBhabhaThreshold[0] * energy1
      && (maxClusterEnergy[1] * 0.1) > _3DBhabhaThreshold[0] * (energy2)
      && ((maxClusterEnergy[0] * 0.1) > _3DBhabhaThreshold[1] * energy1
          || (maxClusterEnergy[1] * 0.1) > _3DBhabhaThreshold[1] * (energy2))) {
    if (BtoBFlag) {BhabhaFlag = true;}
  }


  if (BhabhaFlag) {
    h_Cluster_Energy_Sum -> Fill((maxClusterEnergy[0] + maxClusterEnergy[1]) / 5.25);
  }


  const int NofTCHit = TCId.size();

  double totalEnergy = 0;
  TrgEclMapping* a = new TrgEclMapping();
  double max = 0;
  double caltrgtiming = 0;

  for (int ihit = 0; ihit < NofTCHit ; ihit ++) {
    h_TCId -> Fill(TCId[ihit]);
    h_TCthetaId -> Fill(a -> getTCThetaIdFromTCId(TCId[ihit]));
    {
      if (a->getTCThetaIdFromTCId(TCId[ihit]) < 4) {
        h_TCphiId_FWD -> Fill(a->getTCPhiIdFromTCId(TCId[ihit]));
      } else if (a->getTCThetaIdFromTCId(TCId[ihit]) > 3 && a->getTCThetaIdFromTCId(TCId[ihit]) < 16) {
        h_TCphiId_BR -> Fill(a->getTCPhiIdFromTCId(TCId[ihit]));
      } else {
        h_TCphiId_BWD -> Fill(a->getTCPhiIdFromTCId(TCId[ihit]));

      }
    }
    h_TCEnergy -> Fill(TCEnergy[ihit]);
    h_Narrow_TCEnergy -> Fill(TCEnergy[ihit]);
    h_Cal_TCTiming -> Fill(FineTiming[ihit]);

    if (max < TCEnergy[ihit]) {
      max = TCEnergy[ihit];
      caltrgtiming = FineTiming[ihit];
    }

    totalEnergy += TCEnergy[ihit];
    h_n_TChit_event -> Fill(NofTCHit);
    double timing = 8 * HitRevoTrg - (128 * RevoFAM[ihit] + TCTiming[ihit]);
    if (timing < 0) {timing = timing + 10240;}
    h_TCTiming->Fill(timing);
  }
  double trgtiming = 8 * HitRevoTrg - (128 *     HitRevoEvtTiming + HitFineTiming);

  if (trgtiming < 0) {trgtiming = trgtiming + 10240;}
  h_TRGTiming -> Fill(trgtiming);
  h_Cal_TRGTiming -> Fill(caltrgtiming);
  h_TotalEnergy -> Fill(totalEnergy);
  h_Narrow_TotalEnergy -> Fill(totalEnergy);

  // usleep(100);
}
