/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/KlongDecayReconstructor/KlongMomentumUpdaterExpertModule.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

// utilities
#include <analysis/utility/ParticleCopy.h>
#include <analysis/utility/KlongCalculatorUtils.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(KlongMomentumUpdaterExpert);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

KlongMomentumUpdaterExpertModule::KlongMomentumUpdaterExpertModule() : Module()
{
  // set module description (e.g. insert text)
  setDescription("This module calculates and updates the kinematics of two body B decays including one Klong");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  addParam("listName", m_listName, "name of particle list", std::string(""));
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.",
           false);
}

void KlongMomentumUpdaterExpertModule::initialize()
{
  // Particle list with name m_listName has to exist
  m_plist.isRequired(m_listName);
}

void KlongMomentumUpdaterExpertModule::event()
{

  unsigned int n = m_plist->getListSize();
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = m_plist->getParticle(i);
    const double m_b = particle->getPDGMass();
    const std::vector<Particle*> daughters = particle->getDaughters();

    if (daughters.size() < 2)
      B2FATAL("Reconstructing particle as a daughter of a decay with less than 2 daughters!");

    if (daughters.size() > 3)
      B2FATAL("Higher multiplicity (>2) missing momentum decays not implemented yet!");

    ROOT::Math::PxPyPzEVector BMomentum;
    ROOT::Math::PxPyPzEVector KMomentum;
    int idx = 0;
    bool is_physical = KlongCalculatorUtils::calculateBtoKlongX(BMomentum, KMomentum, daughters, m_b, idx);

    if (!is_physical) {
      B2ERROR("Recalculation of two body B-decay with Klong gives unphysical results. "
              "Update of kinematics is skipped!");
      continue;
    }

    // Set 4-vector to B-meson
    particle->set4Vector(BMomentum);

    // Set 4-vector to Klong
    for (auto daughter : daughters) {
      if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
        auto copyKlong = ParticleCopy::copyParticle(daughter);
        copyKlong->set4Vector(KMomentum);
        copyKlong->writeExtraInfo("permID", idx);
        particle->replaceDaughter(daughter, copyKlong);
      }
    }

  }

}
