/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/NeutralHadron4MomentumCalculator/NeutralHadron4MomentumCalculatorModule.h>

#include <framework/logging/Logger.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <analysis/utility/ParticleCopy.h>
#include <framework/geometry/B2Vector3.h>
#include <Math/Vector4D.h>
#include <vector>
#include <cmath>

using namespace Belle2;

REG_MODULE(NeutralHadron4MomentumCalculator)

NeutralHadron4MomentumCalculatorModule::NeutralHadron4MomentumCalculatorModule() : Module()
{
  // Set module properties
  setDescription(
    R"DOC(Calculates 4-momentum of a neutral hadron in a given decay chain e.g. B0 -> J/Psi K_L0, or anti-B0 -> p+ K- anti-n0.)DOC");

  // Parameter definitions
  addParam("decayString", m_decayString, "Decay string for which one wants to perform the calculation", std::string(""));
  addParam("allowGamma", m_allowGamma, "Whether allow the selected particle to be gamma", false);

}

void NeutralHadron4MomentumCalculatorModule::initialize()
{
  B2DEBUG(17, "Neutralhadron4MomentumCalculator: Use particle list: " << m_decayString);
  bool valid = m_decayDescriptor.init(m_decayString);
  if (!valid)
    B2ERROR("NeutralHadron4MomentumCalculatorModule::initialize Invalid Decay Descriptor " << m_decayString);

  auto hierarchy = m_decayDescriptor.getHierarchyOfSelected();
  if (hierarchy.size() != 1)
    B2ERROR("NeutralHadron4MomentumCalculatorModule::initialize Only one particle can be selected in " << m_decayString);
  if (hierarchy[0].size() != 2)
    B2ERROR("NeutralHadron4MomentumCalculatorModule::initialize The selected particle must be a direct daughter " << m_decayString);

  std::string neutralHadronName = hierarchy[0][1].second;
  if (neutralHadronName != "n0" and neutralHadronName != "K_L0") {
    if (m_allowGamma == true and hierarchy[0][1].second == "gamma")
      B2WARNING("NeutralHadron4MomentumCalculatorModule::initialize The selected particle is gamma but you allowed so; be aware.");
    else
      B2ERROR("NeutralHadron4MomentumCalculatorModule::initialize The selected particle must be a long-lived neutral hadron "
              "i.e. (anti-)n0 or K_L0, or at least a photon (gamma), in which case you need to set allowGamma as true."
              "Input particle: " << m_decayString);
  }

  m_iNeutral = hierarchy[0][1].first;

  std::string motherFullName = m_decayDescriptor.getMother()->getFullName();
  m_plist.isRequired(motherFullName);
}

void NeutralHadron4MomentumCalculatorModule::event()
{
  const unsigned int n = m_plist->getListSize();
  std::vector<unsigned int> toRemove;
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = m_plist->getParticle(i);
    std::vector<Particle*> daughters = particle->getDaughters();
    ROOT::Math::PxPyPzEVector others4Momentum;
    for (int j = 0; j < m_decayDescriptor.getNDaughters(); j++) {
      if (j != m_iNeutral) {
        others4Momentum += daughters[j]->get4Vector();
      }
    }
    const Particle* originalNeutral = daughters[m_iNeutral];
    Particle* neutral = ParticleCopy::copyParticle(originalNeutral);
    particle->removeDaughter(originalNeutral);
    particle->appendDaughter(neutral);
    B2Vector3D neutralDirection;
    if (neutral->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster) {
      neutralDirection = neutral->getECLCluster()->getClusterPosition().Unit();
    } else if (neutral->getParticleSource() == Particle::EParticleSourceObject::c_KLMCluster) {
      neutralDirection = neutral->getKLMCluster()->getClusterPosition().Unit();
    } else {
      B2ERROR("Your neutral particle doesn't originate from ECLCluster nor KLMCluster.");
    }
    double a = others4Momentum.Vect().Dot(neutralDirection);
    double b = (std::pow(particle->getPDGMass(), 2) - std::pow(neutral->getMass(), 2) - others4Momentum.mag2()) / 2.;
    double c = others4Momentum.E();
    double d = std::pow(neutral->getMass(), 2);
    double D = (a * a - c * c) * d + b * b;
    if (D >= 0) {
      double neutralP = (-1. * a * b - c * std::sqrt(D)) / (a * a - c * c);
      double neutralPx = neutralP * neutralDirection.x();
      double neutralPy = neutralP * neutralDirection.y();
      double neutralPz = neutralP * neutralDirection.z();
      double neutralE = std::sqrt(neutralP * neutralP + d);
      const ROOT::Math::PxPyPzEVector newNeutral4Momentum(neutralPx, neutralPy, neutralPz, neutralE);

      double motherPx = newNeutral4Momentum.Px() + others4Momentum.Px();
      double motherPy = newNeutral4Momentum.Py() + others4Momentum.Py();
      double motherPz = newNeutral4Momentum.Pz() + others4Momentum.Pz();
      double motherE = newNeutral4Momentum.E() + others4Momentum.E();
      const ROOT::Math::PxPyPzEVector newMother4Momentum(motherPx, motherPy, motherPz, motherE);

      neutral->set4Vector(newNeutral4Momentum);
      particle->set4Vector(newMother4Momentum);
    } else {
      toRemove.push_back(particle->getArrayIndex());
    }
  }
  m_plist->removeParticles(toRemove);
}
