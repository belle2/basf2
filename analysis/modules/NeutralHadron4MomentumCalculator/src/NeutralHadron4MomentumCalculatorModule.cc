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
#include <mdst/dataobjects/KLMCluster.h>
#include <TVector3.h>
#include <TLorentzVector.h>
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
  addParam("listName", m_listName, "Name of the ParticleList for which one wants to perform the calculation", std::string(""));

}

void NeutralHadron4MomentumCalculatorModule::initialize()
{
  B2DEBUG(1, "Neutralhadron4MomentumCalculator: Use particle list: " << m_listName);
  m_plist.isRequired(m_listName);
}

void NeutralHadron4MomentumCalculatorModule::event()
{
  const unsigned int n = m_plist->getListSize();
  std::vector<unsigned int> toRemove;
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = m_plist->getParticle(i);
    std::vector<Particle*> daughters = particle->getDaughters();
    Particle* charged = daughters[0];
    Particle* neutral = daughters[1];
    TVector3 neutralDirection;
    if (neutral->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster) {
      neutralDirection = neutral->getECLCluster()->getClusterPosition().Unit();
    } else if (neutral->getParticleSource() == Particle::EParticleSourceObject::c_KLMCluster) {
      neutralDirection = neutral->getKLMCluster()->getClusterPosition().Unit();
    } else {
      B2ERROR("Your neutral particle doens't originate from ECLCluster nor KLMCluster.");
    }
    double a = charged->getMomentum() * neutralDirection;
    double b = (std::pow(particle->getPDGMass(), 2) - std::pow(neutral->getMass(), 2) - charged->get4Vector().Mag2()) / 2.;
    double c = charged->getEnergy();
    double d = std::pow(neutral->getMass(), 2);
    double D = (a * a - c * c) * d + b * b;
    if (D >= 0) {
      double neutralP = (-1. * a * b - c * std::sqrt(D)) / (a * a - c * c);
      double neutralPx = neutralP * neutralDirection.x();
      double neutralPy = neutralP * neutralDirection.y();
      double neutralPz = neutralP * neutralDirection.z();
      double neutralE = std::sqrt(neutralP * neutralP + d);
      const TLorentzVector& newNeutral4Momentum = TLorentzVector(neutralPx, neutralPy, neutralPz, neutralE);

      double motherPx = newNeutral4Momentum.Px() + charged->getPx();
      double motherPy = newNeutral4Momentum.Py() + charged->getPy();
      double motherPz = newNeutral4Momentum.Pz() + charged->getPz();
      double motherE = newNeutral4Momentum.E() + charged->getEnergy();
      const TLorentzVector& newMother4Momentum = TLorentzVector(motherPx, motherPy, motherPz, motherE);

      neutral->set4Vector(newNeutral4Momentum);
      particle->set4Vector(newMother4Momentum);
    } else {
      toRemove.push_back(particle->getArrayIndex());
    }
  }
  m_plist->removeParticles(toRemove);
}
