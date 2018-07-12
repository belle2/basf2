/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/InclusiveParticleCheckerModule.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/particledb/EvtGenDatabasePDG.h>
#include <iomanip>

using namespace Belle2;

namespace {
  /** boost::variant visitor to convert particle names into pdg codes and
   * return integers as they are */
  class GetPDGCode: public boost::static_visitor<int> {
  public:
    /** int: already pdg code */
    int operator()(int i) const
    {
      return i;
    }
    /** std::string: particle name, lookup its pdg code */
    int operator()(const std::string& i) const
    {
      auto* p = Belle2::EvtGenDatabasePDG::Instance()->GetParticle(i.c_str());
      if (!p) {
        B2ERROR("Unknown particle name: " << i);
        return 0;
      }
      return p->PdgCode();
    }
  };
}

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(InclusiveParticleChecker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

InclusiveParticleCheckerModule::InclusiveParticleCheckerModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(
Check for the existence of one or more inclusive particles in the list of
generated particles. If any of the particles is found the event is accepted and
the return value is set to True. Otherwise the return value is set to false.

The particles to look for can either be given as numerical pdg codes or as
particle names or even as a mix of both:

>>> checker = path.add_module("InclusiveParticleChecker", particles = [11, "pi+"])

Valid names are defined in the ``evt.pdl`` of evtgen and can be inspected using
either the `pdg` module or ``b2help-particles``.

This module is intended for inclusive signal generation where we run a
generator which produces generic events and we require a certain particle to be
present in the resulting list of particles.
)DOC");

  // Parameter definitions
  addParam("particles", m_particles, "Name or PDG code of the particles to look for. "
           "If any of those is found in the event the event is accepted");
  addParam("includeConjugates", m_includeConjugates, "If true look for either the "
           "particles or their charge conjugates", true);
}

void InclusiveParticleCheckerModule::initialize()
{
  m_mcParticles.isRequired();
  for(const auto &p: m_particles) {
    int pdg = boost::apply_visitor(GetPDGCode(), p);
    if(pdg==0) continue;
    m_particleCodes.insert(pdg);
    if(m_includeConjugates) {
      const auto* anti = EvtGenDatabasePDG::Instance()->GetParticle(-pdg);
      if(anti) {
        m_particleCodes.insert(-pdg);
      }
    }
  }
  B2INFO("Accepting events which contain one of the following particles:");
  for(int i: m_particleCodes){
    B2INFO(" - Particle " << std::quoted(EvtGenDatabasePDG::Instance()->GetParticle(i)->GetName())
           << " (" << i << ")");
  }
}

void InclusiveParticleCheckerModule::event()
{
  bool found{false};
  for(const MCParticle& p: m_mcParticles){
    if(m_particleCodes.count(p.getPDG())>0) {
      found = true;
      break;
    }
  }
  setReturnValue(found);
}
