/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "../include/PrintTauTauMCParticlesModule.h"
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include <TDatabasePDG.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintTauTauMCParticles)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintTauTauMCParticlesModule::PrintTauTauMCParticlesModule() : Module()
{
  B2DEBUG(100, "PrintTauTauMCParticles called:");
  //Set module properties
  setDescription("Print an MCParticle Collection");

  //Parameter definition
  addParam("collectionName", m_particleList, "Collection to print", string(""));
  addParam("onlyPrimaries", m_onlyPrimaries, "Show only primary particles", true);
  addParam("maxLevel", m_maxLevel, "Show only up to specified depth level, -1 means no limit", -1);
}


void PrintTauTauMCParticlesModule::event()
{
  StoreArray<MCParticle> MCParticles(m_particleList);
  m_seen.clear();
  m_seen.resize(MCParticles.getEntries() + 1, false);
  B2INFO("Content from MCParticle Collection '" + m_particleList + "':");
  for (int i = 0; i < MCParticles.getEntries(); i++) {
    MCParticle& p = *MCParticles[i];
    char buf[200];
    int moID = 0;
    MCParticle* q = p.getMother();
    if (q != NULL) {
      moID = q->getIndex();
    }
    sprintf(buf, "PMC: %3d %4u %8d %4d %4d %4d %9.4f %9.4f %9.4f %9.4f",
            p.getIndex(), p.getStatus(), p.getPDG(), moID,
            p.getFirstDaughter(), p.getLastDaughter(),
            p.get4Vector().Px(), p.get4Vector().Py(),
            p.get4Vector().Pz(), p.get4Vector().E());
    B2INFO(buf);
  }
  //Loop over the primary particles (no mother particle exists)
  for (int i = 0; i < MCParticles.getEntries(); i++) {
    MCParticle& mc = *MCParticles[i];
    if (!(mc.getMother() != NULL && mc.getMother()->getIndex() <= 3)) continue;
    printTree(mc, 0);
  }
}


void PrintTauTauMCParticlesModule::printTree(const MCParticle& mc, int level)
{
  if (m_onlyPrimaries && !mc.hasStatus(MCParticle::c_PrimaryParticle)) return;
  //Only show up to max level
  if (m_maxLevel >= 0 && level > m_maxLevel) return;
  ++level;
  string indent = "";
  for (int i = 0; i < 2 * level; i++) indent += " ";
  TDatabasePDG* pdb = TDatabasePDG::Instance();
  TParticlePDG* pdef = pdb->GetParticle(mc.getPDG());
  string name = pdef ? (string(" (") + pdef->GetTitle() + ")") : "";

  if (m_seen[mc.getIndex()]) {
    B2INFO(boost::format("%4d %s%10d%s*") % mc.getIndex() % indent % mc.getPDG() % name);
    return;
  }
  const TVector3& p = mc.getMomentum();
  const TVector3& v = mc.getVertex();
  B2INFO(boost::format("%3d %s%5d%s%20tp:(%10.3e, %10.3e, %10.3e) v:(%10.3e, %10.3e, %10.3e), t:%10.3e,%10.3e, s:%d, c:%d")
         % mc.getIndex() % indent % mc.getPDG() % name
         % p.X() % p.Y() % p.Z()
         % v.X() % v.Y() % v.Z()
         % mc.getProductionTime() % mc.getDecayTime()
         % mc.getStatus() % mc.getCharge()
        );

  const vector<MCParticle*> daughters = mc.getDaughters();
  BOOST_FOREACH(MCParticle * daughter, daughters) {
    printTree(*daughter, level);
  }
  m_seen[mc.getIndex()] = true;
}
