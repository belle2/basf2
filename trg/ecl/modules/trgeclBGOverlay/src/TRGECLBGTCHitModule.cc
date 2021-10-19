/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
      m_TCEnergyCut(0.005),
      m_TCTimingCutLow(-1500),
      m_TCTimingCutHigh(1500),
      m_debugLevel(0)
  {
    string desc = "TRGECLBGTCHitModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("TCEnergyCut",
             m_TCEnergyCut,
             "TC energy cut: TC(E)<X (GeV), X=",
             m_TCEnergyCut);
    addParam("TCTimingCutLow",
             m_TCTimingCutLow,
             "TC Timing cut low: TC(T)>X (ns), X=",
             m_TCTimingCutLow);
    addParam("TCTimingCutHigh",
             m_TCTimingCutHigh,
             "TC Timing cut high: TC(T)<X (ns), X=",
             m_TCTimingCutHigh);
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

    B2INFO("[TRGECLBGTCHitModule] TCEnergyCut     : TC(E) GeV = "
           << m_TCEnergyCut);
    B2INFO("[TRGECLBGTCHitModule] TCTimingCutLow  : TC(T) ns  = "
           << m_TCTimingCutLow);
    B2INFO("[TRGECLBGTCHitModule] TCTimingCutHigh : TC(T) ns  = "
           << m_TCTimingCutHigh);

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
      if (h.e < m_TCEnergyCut) { continue; }
      // TC timing cut to reduce object size
      if (h.t < m_TCTimingCutLow || h.t > m_TCTimingCutHigh) { continue; }
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
      double tcTime   = (double) ttt.getTCTime();
      if (tcID <= 0) {continue;}
      // ADC to Energy(MeV) conversion
      double adc2energy = 5.231;
      // TC energy (GeV)
      double tcEnergy = tcEnergyADC * (adc2energy / 1000);
      // TC energy cut to reduce object size
      if (tcEnergy < m_TCEnergyCut) { continue; }
      // TC timing cut to reduce object size
      if (tcTime < m_TCTimingCutLow || tcTime > m_TCTimingCutHigh) { continue; }
      // store data
      m_trgeclBGTCHits.appendNew(tcID,
                                 tcEnergy,
                                 tcTime);
    }
  }

} // namespace Belle2
