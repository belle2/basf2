// $Id$
//---------------------------------------------------------------
// Filename : TRGECLModule.cc
// Section  : TRG ECL
// Owner    : InSoo Lee / Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A trigger module for TRG ECL
//---------------------------------------------------------------
// 1.00 : 2017/05/08 : First version
//---------------------------------------------------------------
#include <trg/ecl/modules/trgeclDQM/TRGECLDQMModule.h>
#include <trg/ecl/TrgEclMapping.h>
#include <trg/ecl/TrgEclCluster.h>
#include <trg/ecl/TrgEclMaster.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>


#include <TDirectory.h>
#include <TRandom3.h>
#include <unistd.h>

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

  h_TCId           = new TH1D("h_TCId",          "Hit TC ID",                   578, 0, 577);
  h_TCthetaId      = new TH1D("h_TCthetaId",     "Hit TC #theta ID",             19, 0, 18);
  h_TCphiId_FWD    = new TH1D("h_TCphiId_FWD",   "Hit TC #phi ID in FWD",        34, 0, 33);
  h_TCphiId_BR     = new TH1D("h_TCphiId_BR",    "Hit TC #phi ID in BR",         38, 0, 37);
  h_TCphiId_BWD    = new TH1D("h_TCphiId_BWD",   "Hit TC #phi ID in BWF",        34, 0, 33);
  h_TotalEnergy    = new TH1D("h_TotalEnergy",   "Total TC Energy (ADC)",       100, 0, 3000);
  h_TCEnergy       = new TH1D("h_TCEnergy",      "TC Energy / event (ADC)",     100, 0, 1500);
  h_Narrow_TotalEnergy    = new TH1D("h_Narrow_TotalEnergy",   "Total TC Energy (ADC)",       100, 0, 500);
  h_Narrow_TCEnergy       = new TH1D("h_Narrow_TCEnergy",      "TC Energy / event (ADC)",     100, 0, 100);
  h_n_TChit_event  = new TH1D("h_n_TChit_event", "N(TC) / Event",                41, 0, 40);
  h_Cluster        = new TH1D("h_Cluster",       "N(Cluster) / event",           21, 0, 20);
  h_TCTiming       = new TH1D("h_TCTiming",      "TC Timing / event (ns)",      100, 3020, 3200);
  h_TRGTiming      = new TH1D("h_TRGTiming",     "TRG Timing / event (ns)",     100, 3020, 3200);
  h_Cal_TCTiming       = new TH1D("h_Cal_TCTiming",      "Cal TC Timing / event (ns)",      100, -400, 400);
  h_Cal_TRGTiming      = new TH1D("h_Cal_TRGTiming",     "TRG Timing / event (ns)",     100, -400, 400);
  h_ECL_TriggerBit      = new TH1D("h_ECL_TriggerBit",     "ECL Trigger Bit",     26, 0, 26);


  oldDir->cd();
}


void TRGECLDQMModule::initialize()
{

  REG_HISTOGRAM
  trgeclHitArray.registerInDataStore();
  trgeclEvtArray.registerInDataStore();
  trgeclCluster.registerInDataStore();
  trgeclSumArray.registerInDataStore();
  defineHisto();

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
  double HitTiming = 0;
  double HitEnergy = 0;
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

  int phy    = 0;
  int b1     = 0;
  int b2v    = 0;
  int b2s    = 0;
  int mu     = 0;
  int pre    = 0;
  int clover = 0;
  int tsource = 0;
  int b1type = 0;
  int etot = 0;
  int vlm = 0;
  //  int s_hit_win= 0;
  std::vector<int> trgbit ;
  trgbit.resize(41, 0);
  for (int iii = 0; iii < trgeclSumArray.getEntries(); iii++) {
    TRGECLUnpackerSumStore* aTRGECLUnpackerSumStore = trgeclSumArray[iii];
    //    s_hit_win = aTRGECLUnpackerSumStore ->getSumNum();
    //    if(s_hit_win != 3|| s_hit_win!=4){continue;}

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

    for (int j = 0; j < 12; j++) {
      trgbit[12 + j] = (vlm >> j) & 0x1;
    }

    trgbit[24] = mu;
    trgbit[25] = pre;

    trgbit[26] = b1type & 0x1;
    trgbit[27] = (b1type >> 1) & 0x1;
    trgbit[28] = (b1type >> 2) & 0x1;
    trgbit[29] = (b1type >> 3) & 0x1;
    trgbit[30] = (b1type >> 4) & 0x1;
    trgbit[31] = (b1type >> 5) & 0x1;
    trgbit[32] = (b1type >> 6) & 0x1;
    trgbit[33] = (b1type >> 7) & 0x1;
    trgbit[34] = (b1type >> 8) & 0x1;
    trgbit[35] = (b1type >> 9) & 0x1;
    trgbit[36] = (b1type >> 10) & 0x1;
    trgbit[37] = (b1type >> 11) & 0x1;
    trgbit[38] = (b1type >> 12) & 0x1;
    trgbit[39] = (b1type >> 13) & 0x1;
    trgbit[40] = (b1type >> 14) & 0x1;

  }

  const char* label[41] = {"Hit", "Timing Source(FWD)", "Timing Source(BR)", "Timing Source(BWD)", "physics Trigger", "2D Bhabha Veto", "3D Bhabha veto", "3D Bhabha Selection", "E Low", "E High", "E LOM", "Cluster Overflow", "Low multi bit 0", "Low multi bit 1", "Low multi bit 2", "Low multi bit 3", "Low multi bit 4", "Low multi bit 5", "Low multi bit 6", "Low multi bit 7", "Low multi bit 8", "Low multi bit 9", "Low multi bit 10", "Low multi bit 11", "mumu bit", "prescale bit", "2D Bhabha bit 1", "2D Bhabha bit 2"  , "2D Bhabha bit 3", "2D Bhabha bit 4", "2D Bhabha bit 5", "2D Bhabha bit 6", "2D Bhabha bit 7", "2D Bhabha bit 8", "2D Bhabha bit 9", "2D Bhabha bit 10", "2D Bhabha bit 11", "2D Bhabha bit 12", "2D Bhabha bit 13", "2D Bhabha bit 14"};



  for (int j = 0; j < 26; j++) {
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

  //
  const int NofTCHit = TCId.size();

  double totalEnergy = 0;
  TrgEclMapping* a = new TrgEclMapping();
  double max = 0;
  double caltrgtiming = 0;
  double timing = 0;
  double trgtiming = 0;

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
    timing = 8 * HitRevoTrg - (128 * RevoFAM[ihit] + TCTiming[ihit]);
    if (timing < 0) {timing = timing + 10240;}
    h_TCTiming->Fill(timing);
  }
  trgtiming = 8 * HitRevoTrg - (128 *     HitRevoEvtTiming + HitFineTiming);

  if (trgtiming < 0) {trgtiming = trgtiming + 10240;}
  h_TRGTiming -> Fill(trgtiming);
  h_Cal_TRGTiming -> Fill(caltrgtiming);
  h_TotalEnergy -> Fill(totalEnergy);
  h_Narrow_TotalEnergy -> Fill(totalEnergy);


  // usleep(100);
}
