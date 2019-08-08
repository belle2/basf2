//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGECLQAMModule.cc
// Section  : TRG ECL
// Owner    : HanEol Cho
// Email    : hecho@hep.hanyang.ac.kr & insoo.lee@belle2.org
//---------------------------------------------------------------
// Description : Quality Assurance Monitor Module for TRG ECL
//               Check data quality run by run
//
//              Checking variables
//             - Total Energy > 1 GeV (E High) : PSNM 8
//             - ICN >  3  : PSNM 13
//             - Bhabha Trigger : PSNM 15
//             - Track Trigger : PSNM 7
//             - E High or ICN > 3
//             - ICN > 3 && Bhabha Trigger : Should be 0 % because bhabha trigger logic has ICN < 4
//             - E High && Bhabha
//             - Track Trigger && Bbabha : Should be 0 %
//             - Low Hit TC (Less than 0.1 x Average ) in Forward, Backward endcap and Barrel
//
//
//---------------------------------------------------------------
// 1.00 : 2018/11/29 : First version
//---------------------------------------------------------------
//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
//trg package headers

#include <trg/ecl/modules/trgeclQAM/TRGECLQAMModule.h>
#include <framework/database/IntervalOfValidity.h>

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TString.h>

using namespace std;

namespace Belle2 {
  //
  //

  //! Register module name
  REG_MODULE(TRGECLQAM);
  //
  //
  //
  string
  TRGECLQAMModule::version() const
  {
    return string("TRGECLQAMModule 1.00");
  }
  //
  //
  //
  TRGECLQAMModule::TRGECLQAMModule()
    : Module::Module(),
      m_nevent(1), m_outputfile("")
  {

    string desc = "TRGECLQAMModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("outputFileName", m_outputfile, "TRGECL QAM file", m_outputfile);

    m_nRun   = 0;
    m_nevent    = 1;

    //ECLTRG Physcis Efficiency
    m_psnm8     = 0;
    m_psnm13    = 0;
    m_psnm8_13  = 0;

    //GDL Logic Test
    m_psnm7_15  = 0;
    m_psnm8_15  = 0;
    m_psnm13_15 = 0;

    //Problem TC Check
    m_FWD = 0;
    m_BAR = 0;
    m_BWD = 0;
    m_ALL = 0;
    memset(TCID, 0, sizeof(TCID));


    B2DEBUG(100, "TRGECLQAMModule ... created");
  }
  //
  //
  //
  TRGECLQAMModule::~TRGECLQAMModule()
  {

    B2DEBUG(100, "TRGECLQAMModule ... destructed ");
  }
  //
  //
  //
  void
  TRGECLQAMModule::initialize()
  {

    B2DEBUG(100, "TRGECLQAMModule::initialize ... options");


    m_TRGECLUnpackerStore.registerInDataStore();
    m_TRGSummary.registerInDataStore();
    //    EvtMeta.registterInDataStore();
  }
  //
  //
  //
  void
  TRGECLQAMModule::beginRun()
  {

    B2DEBUG(200, "TRGECLQAMModule ... beginRun called ");

  }
  //
  //
  //
  void
  TRGECLQAMModule::event()
  {

    B2DEBUG(200, "TRGECLFAMMoudle ... event called");


    // QAM efficiency

    int psnmbits = m_TRGSummary->getPsnmBits(0);
    //    cout << psnmbits <<endl;
    if (((psnmbits >> 8) & 1)) {
      m_psnm8++;
      // cout << "8" <<endl;
    }
    if (((psnmbits >> 13) & 1)) {
      m_psnm13++;
    }
    if (((psnmbits >> 8) & 1) || ((psnmbits >> 13) & 1)) {
      m_psnm8_13++;
    }

    // QAM GDL Logic Test
    if (((psnmbits >> 7) & 1) && ((psnmbits >> 15) & 1)) {
      m_psnm7_15++;
    }
    if (((psnmbits >> 8) & 1) && ((psnmbits >> 15) & 1)) {
      m_psnm8_15++;
    }
    if (((psnmbits >> 13) & 1) && ((psnmbits >> 15) & 1)) {
      m_psnm13_15++;
    }

    //QAM ECL Hit Map
    //    StoreArray<TRGECLUnpackerStore> m_TRGECLUnpackerStore;
    for (int ii = 0; ii < m_TRGECLUnpackerStore.getEntries(); ii++) {
      TRGECLUnpackerStore* m_TRGECLUnpacker = m_TRGECLUnpackerStore[ii];
      int TCId = (m_TRGECLUnpacker->getTCId());
      int HitEnergy = (m_TRGECLUnpacker->getTCEnergy());
      if (TCId < 1 || TCId > 576 || HitEnergy == 0) {continue;}
      TCID[TCId - 1]++;

    }
    StoreObjPtr<EventMetaData> bevt;
    m_nRun = bevt->getRun();
    m_nevent++;

  }
  //
  //
  //
  void
  TRGECLQAMModule::endRun()
  {
    //QAM ECLTRG Physics Efficiency
    m_psnm8  /= m_nevent;
    m_psnm13 /= m_nevent;
    m_psnm8_13 /= m_nevent;

    //QAM GDL Logic Test
    m_psnm7_15 /= m_nevent;
    m_psnm8_15 /= m_nevent;
    m_psnm13_15 /= m_nevent;

    double mean_FWD = 0;
    double mean_BAR = 0;
    double mean_BWD = 0;

    //QAM TC Hit Map Check
    for (int TCId = 1; TCId < 577; TCId++) {
      if (TCId < 81) { //Forward Endcap
        mean_FWD += TCID[TCId - 1];
      } else if (TCId > 80 && TCId < 513) { //Barrel
        mean_BAR += TCID[TCId - 1];
      } else if (TCId > 512) { //Backward Endcap
        mean_BWD += TCID[TCId - 1];
      }
    }
    mean_FWD /= 80;
    mean_BAR /= (512 - 80);
    mean_BWD /= (576 - 512);

    for (int TCId = 1; TCId < 577; TCId++) {
      if (TCId < 81) { //Forward Endcap
        if (TCID[TCId - 1] < mean_FWD * 0.1)m_FWD++;
      } else if (TCId > 80 && TCId < 513) { //Barrel
        if (TCID[TCId - 1] < mean_BAR * 0.1)m_BAR++;
      } else if (TCId > 512) { //Backward Endcap
        if (TCID[TCId - 1] < mean_BWD * 0.1)m_BWD++;
      }
    }

    m_ALL = m_FWD + m_BAR + m_BWD;

    TString outputfile = m_outputfile;
    TFile file(outputfile, "RECREATE");
    TTree* tree = new TTree("tree", "tree");
    int nevent;
    double psnm8;
    double psnm13;
    double psnm8_13;
    double psnm7_15;
    double psnm8_15;
    double psnm13_15;
    double FWD;
    double BAR;
    double BWD;
    double ALL;

    nevent = m_nevent;
    psnm8 = m_psnm8;
    psnm13 = m_psnm13;
    psnm8_13 = m_psnm8_13;
    psnm7_15 = m_psnm7_15;
    psnm8_15 = m_psnm8_15;
    psnm13_15 = m_psnm13_15;
    FWD = m_FWD;
    BAR = m_BAR;
    BWD = m_BWD;
    ALL = m_ALL;

    tree->Branch("m_nRun",  &m_nRun,   "m_nRun/I");
    tree->Branch("m_nevent",  &nevent,   "m_nevent/I");
    tree->Branch("m_psnm8",  &psnm8,   "m_psnm8/D");
    tree->Branch("m_psnm13",  &psnm13,   "m_psnm13/D");
    tree->Branch("m_psnm8_13",  &psnm8_13,   "m_psnm8_13/D");
    tree->Branch("m_psnm7_15",  &psnm7_15,   "m_psnm7_15/D");
    tree->Branch("m_psnm8_15",  &psnm8_15,   "m_psnm8_15/D");
    tree->Branch("m_psnm13_15",  &psnm13_15,   "m_psnm13_15/D");
    tree->Branch("m_FWD",  &FWD,   "m_FWD/D");
    tree->Branch("m_BAR",  &BAR,   "m_BAR/D");
    tree->Branch("m_BWD",  &BWD,   "m_BWD/D");
    tree->Branch("m_ALL",  &ALL,   "m_ALL/D");

    tree->Fill();
    tree->Write();
    file.Close();



    B2INFO("ECL Trigger QAM result "
           << LogVar("Run Number",  m_nRun)
           << LogVar("Total Event",  nevent)
           << LogVar("Rate of Ehigh or ICN >3 ", psnm8_13)
           << LogVar("Rate of EHigh and Bhabha",  psnm8_15)
           << LogVar("Rate of ICN>3 and Bhabha (Should be 0 %)",  psnm13_15)
           << LogVar("Rate of Track Trigger and Bhabha (Should be 0 %)",  psnm7_15)
           << LogVar("The # of Low Hit TC in Forward Endcap",  FWD)
           << LogVar("The # of Low Hit TC in Barrel",  BAR)
           << LogVar("The # of Low Hit TC in Backward Endcap",  BWD));


    B2DEBUG(100, "TRGECLQAMModule ... endRun called ");

  }
  //
  //
  //
  void  TRGECLQAMModule::terminate()
  {
    B2DEBUG(100, "TRGECLQAMModule ... terminate called ");

  }
  //
  //
  //
} // namespace Belle2
