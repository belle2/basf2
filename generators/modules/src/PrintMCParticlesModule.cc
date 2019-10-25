/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/PrintMCParticlesModule.h>

#include <mdst/dataobjects/MCParticle.h>

#include <boost/format.hpp>

#include <TDatabasePDG.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintMCParticles)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintMCParticlesModule::PrintMCParticlesModule() : Module()
{
  //Set module properties
  setDescription("Print an MCParticle List");

  //Parameter definition
  addParam("storeName", m_particleList, "Name of the StoreArray to print", string(""));
  addParam("onlyPrimaries", m_onlyPrimaries, "Show only primary particles", true);
  addParam("maxLevel", m_maxLevel, "Show only up to specified depth level, -1 means no limit", -1);
}

void PrintMCParticlesModule::initialize()
{
  m_mcparticles.isRequired(m_particleList);
}

void PrintMCParticlesModule::event()
{
  m_seen.clear();
  m_seen.resize(m_mcparticles.getEntries() + 1, false);
  B2INFO("Content from MCParticle list '" + m_mcparticles.getName() + "':");

  //Loop over the top level particles (no mother particle exists)
  for (int i = 0; i < m_mcparticles.getEntries(); i++) {
    MCParticle& mc = *m_mcparticles[i];
    if (mc.getMother() != NULL) continue;
    printTree(mc, 0);
  }
}


void PrintMCParticlesModule::printTree(const MCParticle& mc, int level)
{
  if (m_onlyPrimaries && !mc.hasStatus(MCParticle::c_PrimaryParticle)) return;

  //Only show up to max level
  if (m_maxLevel >= 0 && level > m_maxLevel) return;
  ++level;
  string indent = "";
  for (int i = 0; i < level; i++) indent += "  ";
  TDatabasePDG* pdb = TDatabasePDG::Instance();
  TParticlePDG* pdef = pdb->GetParticle(mc.getPDG());
  string name = pdef ? (string(" (") + pdef->GetTitle() + ")") : "";

  if (m_seen[mc.getIndex()]) {
    B2INFO(boost::format("%4d %s%10d%s*") % mc.getIndex() % indent % mc.getPDG() % name);
    return;
  }
  const TVector3& p = mc.getMomentum();
  const TVector3& v = mc.getVertex();

  B2INFO(boost::format("%3d %s%5d%s%30tE:%10.3e m:%10.3e p:(%10.3e,%10.3e,%10.3e) v:(%10.3e,%10.3e,%10.3e), t:%10.3e,%10.3e, s:%d, c:%d")
         % mc.getIndex() % indent % mc.getPDG() % name
         % mc.getEnergy() % mc.getMass()
         % p.X() % p.Y() % p.Z()
         % v.X() % v.Y() % v.Z()
         % mc.getProductionTime() % mc.getDecayTime()
         % mc.getStatus() % mc.getCharge()
        );

  const vector<MCParticle*> daughters = mc.getDaughters();
  for (MCParticle* daughter : daughters) {
    printTree(*daughter, level);
  }
  m_seen[mc.getIndex()] = true;
}
