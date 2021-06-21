//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGECLBGTCHitModule.cc
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A trigger module for TRG ECL
//
//---------------------------------------------------------------
// 1.00 : 2012/06/xx : First version
//---------------------------------------------------------------

#define TRGECLBGTCHit_SHORT_NAMES

//framework headers
#include <framework/logging/Logger.h>

//trg package headers
#include "trg/ecl/modules/trgeclBGOverlay/TRGECLBGTCHitModule.h"

#include <iostream>

using namespace std;

namespace Belle2 {
//
//
//! Register module name
  REG_MODULE(TRGECLBGTCHit);
//
//
//
  string
  TRGECLBGTCHitModule::version() const
  {
    return string("TRGECLBGTCHitModule 1.00");
  }
//
//
//
  TRGECLBGTCHitModule::TRGECLBGTCHitModule()
    : Module::Module(),
      m_simTCEnergyCut(0.00001),
      m_debugLevel(0)
  {
    string desc = "TRGECLBGTCHitModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("simTCEnergyCut",
             m_simTCEnergyCut,
             "simulation TC energy cut: TC(E)<X (GeV), X=",
             m_simTCEnergyCut);
    addParam("DebugLevel",
             m_debugLevel,
             "TRGECL debug level",
             m_debugLevel);

    m_TCMap = new TrgEclMapping();

    B2DEBUG(100, "TRGECLBGTCHitModule ... created");
  }
//
//
//
  TRGECLBGTCHitModule::~TRGECLBGTCHitModule()
  {
    B2DEBUG(100, "TRGECLBGTCHitModule ... destructed ");
  }
//
//
//
  void
  TRGECLBGTCHitModule::initialize()
  {
    B2DEBUG(100, "TRGECLBGTCHitModule::initialize>");

    B2INFO("[TRGECLBGTCHitModule] simTCEnergyCut(TC(E)<X(GeV)) = "
           << m_simTCEnergyCut);

    m_eclHits.registerInDataStore();
    m_trgeclUnpackerStores.registerInDataStore();
    m_trgeclBGTCHits.registerInDataStore();
  }
//
//
//
  void
  TRGECLBGTCHitModule::beginRun()
  {
    B2DEBUG(200, "TRGECLBGTCHitModule ... beginRun called ");
  }
//
//
//
  void
  TRGECLBGTCHitModule::event()
  {
    B2DEBUG(200, "TRGECLBGTCHitMoudle ... event called");

    if (m_eclHits.getEntries() > 0) {
      genSimulationObj();
    } else if (m_trgeclUnpackerStores.getEntries() > 0) {
      genRandomTrgObj();
    }
  }
//
//
//
  void
  TRGECLBGTCHitModule::endRun()
  {
    B2DEBUG(200, "TRGECLBGTCHitModule ... endRun called ");
  }
//
//
//
  void
  TRGECLBGTCHitModule::terminate()
  {
    B2DEBUG(100, "TRGECLBGTCHitModule ... terminate called ");
  }
//
//
//
  void
  TRGECLBGTCHitModule::genSimulationObj()
  {

    struct hit_t { double e, t; };
    map<int, hit_t> a;

    struct thit_t { int tid, hid; };
    vector<thit_t> tr;

    for (const ECLHit& t : m_eclHits) {
      double txtal = t.getTimeAve();
      // timing cut, +-8ns, (almost) same as ECL in SensitiveDetector.cc
      if (txtal >= 8000.0 || txtal < -8000.0) continue;
      double edep = t.getEnergyDep();
      int TimeIndex = (txtal + 8000.0) * (1. / 100);
      int cellId = t.getCellId() - 1;
      int iTCIdm = m_TCMap->getTCIdFromXtalId(cellId + 1) - 1;

      int key = iTCIdm * 160 + TimeIndex;
      hit_t& h = a[key];
      double old_edep = h.e, old_txtal = h.t;
      double new_edep = old_edep + edep;
      h.e = new_edep;
      h.t = (old_edep * old_txtal + edep * txtal) / new_edep;
    }
    // save TC data to dataobject
    for (pair<int, hit_t> t : a) {
      int key  = t.first;
      int tcid = key / 160;
      const hit_t& h = t.second;
      // TC energy cut to reduce object size
      if (h.e < m_simTCEnergyCut) { continue; }
      // store data
      m_trgeclBGTCHits.appendNew(tcid + 1, h.e, h.t);
    }

  }
  //
  //
  //
  void
  TRGECLBGTCHitModule::genRandomTrgObj()
  {

    for (const TRGECLUnpackerStore& ttt : m_trgeclUnpackerStores) {
      int tcID = ttt.getTCId();
      // TC energy(ADC)
      double tcEnergyADC = (double) ttt.getTCEnergy();
      // TC timing(ns)
      double tcCalTime   = (double) ttt.getTCCALTime();
      if (tcID <= 0) {continue;}
      // ADC to Energy(MeV) conversion
      double adc2energy = 5.231;
      // TC energy (GeV)
      double tcEnergy = tcEnergyADC * (adc2energy / 1000);
      // store data
      m_trgeclBGTCHits.appendNew(tcID,
                                 tcEnergy,
                                 tcCalTime);
    }
  }

} // namespace Belle2
