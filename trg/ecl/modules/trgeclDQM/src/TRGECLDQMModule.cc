//---------------------------------------------------------------
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

#include <framework/datastore/StoreObjPtr.h>


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
}



TRGECLDQMModule::~TRGECLDQMModule()
{
}

void TRGECLDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  TDirectory* dirDQM = NULL;
  dirDQM = oldDir->mkdir("TRG");
  dirDQM->cd();
  h_TCId = new TH1D("h_TCId", "Hit TC Id", 100, 0, 600);
  h_TotalEnergy = new TH1D("h_TotalEnergy", "Total TC Energy(GeV)", 100, 0, 3000);
  oldDir->cd();
}


void TRGECLDQMModule::initialize()
{

  REG_HISTOGRAM
  m_TRGECLUnpackerStore.registerInDataStore();
  defineHisto();

}


void TRGECLDQMModule::beginRun()
{
}

void TRGECLDQMModule::endRun()
{
}


void TRGECLDQMModule::terminate()
{
}

void TRGECLDQMModule::event()
{
  TCId.clear();
  TCEnergy.clear();
  TCTiming.clear();

  StoreArray<TRGECLUnpackerStore> trgeclHitArray;
  for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {

    TRGECLUnpackerStore* aTRGECLUnpackerStore = trgeclHitArray[ii];
    int TCID = (aTRGECLUnpackerStore->getTCId());
    double HitTiming    = aTRGECLUnpackerStore ->getTCTime();
    double HitEnergy =  aTRGECLUnpackerStore -> getTCEnergy();

    TCId.push_back(TCID);
    TCEnergy.push_back(HitEnergy);
    TCTiming.push_back(HitTiming);

  }
  //
  const int NofTCHit = TCId.size();

  double totalEnergy = 0;


  for (int ihit = 0; ihit < NofTCHit ; ihit ++) {

    //    cout << TCId[ihit] << endl;
    h_TCId -> Fill(TCId[ihit]);
    totalEnergy += TCEnergy[ihit];

  }

  h_TotalEnergy -> Fill(totalEnergy);



}
