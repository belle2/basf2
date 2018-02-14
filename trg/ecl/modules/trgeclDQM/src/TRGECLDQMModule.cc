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

  _TCCluster = new TrgEclCluster();
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
  delete _TCCluster;
}


void TRGECLDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  TDirectory* dirDQM = NULL;
  dirDQM = oldDir->mkdir("TRG");
  dirDQM->cd();
  h_TCId = new TH1D("h_TCId", "Hit TC ID", 100, 0, 600);
  //  h_TC2d = new TH2D("h_TC2d","Hit TC ID;x;Hit TC #phi",17,0,17,36,0,36);
  h_TCthetaId = new TH1D("h_TCthetaId", "Hit TC #theta ID", 17, 0, 17);
  h_TCphiId_FWD = new TH1D("h_TCphiId_FWD", "Hit TC #phi ID", 33, 0, 32);
  h_TCphiId_BR = new TH1D("h_TCphiId_BR", "Hit TC #phi ID", 36, 0, 36);
  h_TCphiId_BWD = new TH1D("h_TCphiId_BWD", "Hit TC #phi ID", 33, 0, 32);
  h_TotalEnergy = new TH1D("h_TotalEnergy", "Total TC Energy (FADC Count)", 100, 0, 1000);
  h_TCEnergy = new TH1D("h_TCEnergy", "TC Energy/event", 100, 0, 300);
  h_n_TChit_event = new TH1D("h_n_TChit_event", "Number of TC / Event", 20, 0, 20);
  h_Cluster = new TH1D("h_Cluster", "N(Cluster)/event", 10, 0, 10);
  h_TCTiming = new TH1D("h_TCTiming", "TCTiming/event", 100, 2000, 4000);
  h_TRGTiming = new TH1D("h_TRGTiming", "TRGTiming/event", 500, 2000, 4000);



  oldDir->cd();
}


void TRGECLDQMModule::initialize()
{

  REG_HISTOGRAM
  trgeclHitArray.registerInDataStore();
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
  for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {
    TRGECLUnpackerStore* aTRGECLUnpackerStore = trgeclHitArray[ii];
    int TCID = (aTRGECLUnpackerStore->getTCId());
    HitEnergy =  aTRGECLUnpackerStore -> getTCEnergy();
    if (TCID < 1 || TCID > 576 || HitEnergy == 0) {continue;}

    HitTiming    = aTRGECLUnpackerStore ->getTCTime();
    HitRevoFam = aTRGECLUnpackerStore -> getRevoFAM();
    HitRevoTrg = aTRGECLUnpackerStore -> getRevoTRG();
    HitFineTiming = aTRGECLUnpackerStore -> getFineTime();


    TCId.push_back(TCID);
    TCEnergy.push_back(HitEnergy);
    TCTiming.push_back(HitTiming);
    RevoFAM.push_back(HitRevoFam);
    RevoTrg.push_back(HitRevoTrg);
    FineTiming.push_back(HitFineTiming);

  }
  //----------------------
  //Clustering
  //----------------------
  //

  _TCCluster = new TrgEclCluster();
  _TCCluster -> setICN(TCId, TCEnergy, TCTiming);
  int icnfwd = _TCCluster -> getICNSub(0);
  int icnbr = _TCCluster -> getICNSub(1);
  int icnbwd = _TCCluster -> getICNSub(2);
  int c = icnfwd + icnbr + icnbwd;
  h_Cluster->Fill(c);

  //
  const int NofTCHit = TCId.size();

  double totalEnergy = 0;
  TrgEclMapping* a = new TrgEclMapping();

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
    totalEnergy += TCEnergy[ihit];
    h_n_TChit_event -> Fill(NofTCHit);
    timing = 8 * RevoTrg[ihit] - (128 * RevoFAM[ihit] + TCTiming[ihit]);
    trgtiming = 8 * RevoTrg[ihit] - (128 * RevoFAM[ihit] + FineTiming[ihit]);

    h_TCTiming->Fill(timing);
  }
  h_TRGTiming -> Fill(trgtiming);
  h_TotalEnergy -> Fill(totalEnergy);


  // usleep(100);
}
