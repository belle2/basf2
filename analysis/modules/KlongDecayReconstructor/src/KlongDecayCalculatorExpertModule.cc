/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/KlongDecayReconstructor/KlongDecayCalculatorExpertModule.h>

// framework aux
#include <framework/logging/Logger.h>

#include <analysis/dataobjects/Particle.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(KlongDecayCalculatorExpert);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

KlongDecayCalculatorExpertModule::KlongDecayCalculatorExpertModule() : Module()
{
  // set module description (e.g. insert text)
  setDescription("This module is used to employ kinematic constraints to determine the momentum of Klongs for two body B decays containing a K_L0 and something else. The module creates a list of K_L0 candidates whose K_L0 momentum is reconstructed by combining the reconstructed direction (from either the ECL or KLM) of the K_L0 and kinematic constraints of the initial state.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  addParam("listName", m_listName, "name of particle list", string(""));
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
}

void KlongDecayCalculatorExpertModule::initialize()
{
  // Particle list with name m_listName has to exist
  m_plist.isRequired(m_listName);
}

void KlongDecayCalculatorExpertModule::event()
{

  unsigned int n = m_plist->getListSize();
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = m_plist->getParticle(i);

    const std::vector<Particle*> daughters = particle->getDaughters();

    if (daughters.size() < 2)
      B2FATAL("Reconstructing particle as a daughter of a decay with less than 2 daughters!");

    if (daughters.size() > 3)
      B2FATAL("Higher multiplicity (>2) missing momentum decays not implemented yet!");


    ROOT::Math::PxPyPzEVector klDaughters;
    int nKlong = 0;
    for (auto daughter : daughters) {
      if (daughter->getPDGCode() == Const::Klong.getPDGCode()) {
        klDaughters += daughter->get4Vector();
        nKlong++;
      }
    }
    if (nKlong != 1)
      B2FATAL("The number of K_L0 is required to be 1!");

    ROOT::Math::PxPyPzEVector pDaughters;
    for (auto daughter : daughters) {
      if (daughter->getPDGCode() != Const::Klong.getPDGCode()) {
        pDaughters += daughter->get4Vector();
      }
    }

    double m_b = particle->getPDGMass();
    ROOT::Math::PxPyPzEVector mom = pDaughters + klDaughters;
    mom.SetE(TMath::Sqrt(mom.P2() + m_b * m_b));
    if (!isnan(mom.P()))
      particle->set4Vector(mom);

  }

}
