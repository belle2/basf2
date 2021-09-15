/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dataobjects/RestOfEvent.h>

#include <framework/datastore/StoreArray.h>

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <analysis/ClusterUtility/ClusterUtils.h>

#include <TLorentzVector.h>

using namespace Belle2;
// New methods:
void RestOfEvent::addParticles(const std::vector<const Particle*>& particlesToAdd)
{
  StoreArray<Particle> allParticles;
  for (auto* particleToAdd : particlesToAdd) {
    std::vector<const Particle*> daughters = particleToAdd->getFinalStateDaughters();
    for (auto* daughter : daughters) {
      bool toAdd = true;
      for (auto& myIndex : m_particleIndices) {
        if (allParticles[myIndex]->isCopyOf(daughter, true)) {
          toAdd = false;
          break;
        }
      }
      if (toAdd) {
        B2DEBUG(10, "\t\tAdding particle with PDG " << daughter->getPDGCode());
        m_particleIndices.insert(daughter->getArrayIndex());
      }
    }
  }
}

std::vector<const Particle*> RestOfEvent::getParticles(const std::string& maskName, bool unpackComposite) const
{
  std::vector<const Particle*> result;
  StoreArray<Particle> allParticles;
  std::set<int> source;
  if (m_particleIndices.size() == 0) {
    B2DEBUG(10, "ROE contains no particles, masks are empty too");
    return result;
  }
  if (maskName == RestOfEvent::c_defaultMaskName) {
    // if no mask provided work with internal source
    source = m_particleIndices;
  } else {
    bool maskFound = false;
    for (auto& mask : m_masks) {
      if (mask.getName() == maskName) {
        maskFound = true;
        source = mask.getParticles();
        break;
      }
    }
    if (!maskFound) {
      B2FATAL("No '" << maskName << "' mask defined in current ROE!");
    }
  }
  for (const int index : source) {
    if ((allParticles[index]->getParticleSource() == Particle::EParticleSourceObject::c_Composite or
         allParticles[index]->getParticleSource() == Particle::EParticleSourceObject::c_V0) && unpackComposite) {
      auto fsdaughters = allParticles[index]->getFinalStateDaughters();
      for (auto* daughter : fsdaughters) {
        result.push_back(daughter);
      }
      continue;
    }
    result.push_back(allParticles[index]);
  }
  return result;
}

std::vector<const Particle*> RestOfEvent::getPhotons(const std::string& maskName, bool unpackComposite) const
{
  auto particles = getParticles(maskName, unpackComposite);
  std::vector<const Particle*> photons;
  for (auto* particle : particles) {
    if (particle->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster) {
      photons.push_back(particle);
    }
  }
  return photons;
}

std::vector<const Particle*> RestOfEvent::getHadrons(const std::string& maskName, bool unpackComposite) const
{
  auto particles = getParticles(maskName, unpackComposite);
  std::vector<const Particle*> hadrons;
  for (auto* particle : particles) {
    if (particle->getParticleSource() == Particle::EParticleSourceObject::c_KLMCluster) {
      hadrons.push_back(particle);
    }
  }
  return hadrons;
}

std::vector<const Particle*> RestOfEvent::getChargedParticles(const std::string& maskName, unsigned int pdg,
    bool unpackComposite) const
{
  auto particles = getParticles(maskName, unpackComposite);
  std::vector<const Particle*> charged;
  for (auto* particle : particles) {
    if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Track) {
      if (pdg == 0 || pdg == abs(particle->getPDGCode())) {
        charged.push_back(particle);
      }
    }
  }
  return charged;
}


bool RestOfEvent::hasParticle(const Particle* particle, const std::string& maskName) const
{
  if (maskName != RestOfEvent::c_defaultMaskName && !hasMask(maskName)) {
    B2FATAL("No '" << maskName << "' mask defined in current ROE!");
  }

  std::vector<const Particle*> particlesROE = getParticles(maskName);
  return isInParticleList(particle, particlesROE);
}

void RestOfEvent::initializeMask(const std::string& name, const std::string& origin)
{
  if (name == "") {
    B2FATAL("Creation of ROE Mask with an empty name is not allowed!");
  }
  if (name == RestOfEvent::c_defaultMaskName) {
    B2FATAL("Creation of ROE Mask with a name " << RestOfEvent::c_defaultMaskName << " is not allowed!");
  }
  if (findMask(name)) {
    B2FATAL("ROE Mask already exists!");
  }
  Mask elon(name, origin);
  m_masks.push_back(elon);
}

void RestOfEvent::excludeParticlesFromMask(const std::string& maskName, const std::vector<const Particle*>& particlesToUpdate,
                                           Particle::EParticleSourceObject listType, bool discard)
{
  Mask* mask = findMask(maskName);
  if (!mask) {
    B2FATAL("No '" << maskName << "' mask defined in current ROE!");
  }
  std::string maskNameToGetParticles = maskName;
  if (!mask->isValid()) {
    maskNameToGetParticles = RestOfEvent::c_defaultMaskName;
  }
  std::vector<const Particle*> allROEParticles =  getParticles(maskNameToGetParticles);
  std::vector<const Particle*> toKeepinROE;
  for (auto* roeParticle : allROEParticles) {
    if (isInParticleList(roeParticle, particlesToUpdate)) {
      if (!discard) {
        // If keep particles option is on, take the equal particles
        toKeepinROE.push_back(roeParticle);
      }
    } else {
      // Keep all particles which has different type than provided list
      if (listType != roeParticle->getParticleSource()) {
        toKeepinROE.push_back(roeParticle);
      } else if (discard) {
        // If keep particles option is off, take not equal particles
        toKeepinROE.push_back(roeParticle);
      }
    }
  }
  mask->clearParticles();
  mask->addParticles(toKeepinROE);
}

void RestOfEvent::updateMaskWithCuts(const std::string& maskName, const std::shared_ptr<Variable::Cut>& trackCut,
                                     const std::shared_ptr<Variable::Cut>& eclCut, const std::shared_ptr<Variable::Cut>& klmCut, bool updateExisting)
{
  Mask* mask = findMask(maskName);
  if (!mask) {
    B2FATAL("ROE Mask does not exist!");
  }
  std::string sourceName = RestOfEvent::c_defaultMaskName;
  if (updateExisting) {
    // if mask already exists, take its particles to update
    sourceName = maskName;
  }
  // get all initial ROE particles, don't touch the possible V0s, otherwise, some daughters may be excluded, and some not... This may be revisited if needed
  std::vector<const Particle*> allROEParticles = getParticles(sourceName, false);
  std::vector<const Particle*> maskedParticles;
  // First check particle type, then check cuts, if no cuts provided, take all particles of this type
  for (auto* particle : allROEParticles) {
    if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Track && (!trackCut || trackCut->check(particle))) {
      maskedParticles.push_back(particle);
    }
    if (particle->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster && (!eclCut || eclCut->check(particle))) {
      maskedParticles.push_back(particle);
    }
    if (particle->getParticleSource() == Particle::EParticleSourceObject::c_KLMCluster && (!klmCut || klmCut->check(particle))) {
      maskedParticles.push_back(particle);
    }
    // don't lose a possible V0 particle
    if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Composite or
        particle->getParticleSource() == Particle::EParticleSourceObject::c_V0) {
      maskedParticles.push_back(particle);
    }
  }
  mask->clearParticles();
  mask->addParticles(maskedParticles);
}

void RestOfEvent::updateMaskWithV0(const std::string& name, const Particle* particleV0)
{
  Mask* mask = findMask(name);
  if (!mask) {
    B2FATAL("ROE Mask does not exist!");
  }
  std::vector<const Particle*> allROEParticles = getParticles(name, false);
  std::vector<int> indicesToErase;
  std::vector<const Particle*> daughtersV0 =  particleV0->getFinalStateDaughters();
  for (auto* maskParticle : allROEParticles) {
    bool toKeep = true;
    for (auto* daughterV0 : daughtersV0) {
      if (daughterV0->isCopyOf(maskParticle, true)) {
        toKeep = false;
      }
    }
    if (!toKeep) {
      indicesToErase.push_back(maskParticle->getArrayIndex());
    }
  }
  if (daughtersV0.size() != indicesToErase.size()) {
    B2DEBUG(10, "Only " << indicesToErase.size() << " daughters are excluded from mask particles. Abort");
    return;
  }
  std::string toprint = "We will erase next indices from " + name + " mask: ";
  for (auto& i : indicesToErase) {
    toprint += std::to_string(i) + " ";
  }
  B2DEBUG(10, toprint);
  // If everything is good, we add
  mask->addV0(particleV0, indicesToErase);
}

bool RestOfEvent::checkCompatibilityOfMaskAndV0(const std::string& name, const Particle* particleV0)
{
  Mask* mask = findMask(name);
  if (!mask) {
    B2FATAL("ROE Mask does not exist!");
  }
  if (!mask->isValid()) {
    return false; //We should have particles here!
  }
  if (particleV0->getParticleSource() != Particle::EParticleSourceObject::c_Composite and
      particleV0->getParticleSource() != Particle::EParticleSourceObject::c_V0) {
    return false;
  }
  std::vector<const Particle*> daughtersV0 =  particleV0->getFinalStateDaughters();
  for (auto* daughter : daughtersV0) {
    if (daughter->getParticleSource() != Particle::EParticleSourceObject::c_Track) {
      return false; // Non tracks are not supported yet
    }
  }
  if (mask->hasV0(particleV0)) {
    return false; // We are not going to add another one
  }
  return true;
}

bool RestOfEvent::hasMask(const std::string& name) const
{
  for (auto& mask : m_masks) {
    if (mask.getName() == name) {
      return true;
    }
  }
  return false;
}
TLorentzVector RestOfEvent::get4Vector(const std::string& maskName) const
{
  TLorentzVector roe4Vector;
  std::vector<const Particle*> myParticles = RestOfEvent::getParticles(maskName);
  for (const Particle* particle : myParticles) {
    // KLMClusters are discarded, because KLM energy estimation is based on hit numbers, therefore it is unreliable
    // also, enable it as an experimental option:
    if (particle->getParticleSource() == Particle::EParticleSourceObject::c_KLMCluster and !m_useKLMEnergy) {
      continue;
    }
    roe4Vector += particle->get4Vector();
  }
  return roe4Vector;
}


RestOfEvent::Mask* RestOfEvent::findMask(const std::string& name)
{
  for (auto& mask : m_masks) {
    if (mask.getName() == name) {
      return &mask;
    }
  }
  return nullptr;

}

int RestOfEvent::getNTracks(const std::string& maskName) const
{
  int nTracks = getChargedParticles(maskName).size();
  return nTracks;
}

int RestOfEvent::getNECLClusters(const std::string& maskName) const
{
  int nROEneutralECLClusters = getPhotons(maskName).size();
  int nROEchargedECLClusters = 0;
  for (auto& roeParticle : getChargedParticles(maskName)) {
    if (roeParticle->getECLCluster()) ++nROEchargedECLClusters;
  }

  return nROEneutralECLClusters + nROEchargedECLClusters;
}

int RestOfEvent::getNKLMClusters(const std::string& maskName) const
{
  int nROEKLMClusters = getHadrons(maskName).size();
  return nROEKLMClusters;
}

TLorentzVector RestOfEvent::get4VectorNeutralECLClusters(const std::string& maskName) const
{
  auto roeClusters = getPhotons(maskName);
  TLorentzVector roe4VectorECLClusters;

  // Add all momenta from neutral ECLClusters which have the nPhotons hypothesis
  for (auto& roeCluster : roeClusters) {
    if (roeCluster->getECLClusterEHypothesisBit() == ECLCluster::EHypothesisBit::c_nPhotons)
      roe4VectorECLClusters += roeCluster->get4Vector();
  }

  return roe4VectorECLClusters;
}

bool RestOfEvent::isInParticleList(const Particle* roeParticle, const std::vector<const Particle*>& particlesToUpdate) const
{
  for (auto* listParticle : particlesToUpdate) {
    if (roeParticle->isCopyOf(listParticle, true)) {
      return true;
    }
  }
  return false;
}

std::vector<std::string> RestOfEvent::getMaskNames() const
{
  std::vector<std::string> maskNames;

  for (auto& mask : m_masks) {
    maskNames.push_back(mask.getName());
  }

  return maskNames;
}

void RestOfEvent::print(const std::string& maskName, bool unpackComposite) const
{
  std::string tab = " - ";
  if (maskName != RestOfEvent::c_defaultMaskName) {
    // Disable possible B2FATAL in printing method, might be useful for tests
    if (!hasMask(maskName)) {
      B2WARNING("No mask with the name '" << maskName << "' exists in this ROE! Nothing else to print");
      return;
    }
    tab = " - - ";
  } else {
    if (m_isNested) {
      B2INFO(tab << "ROE is nested");
    }
    if (m_isFromMC) {
      B2INFO(tab << "ROE is build from generated particles");
    }
  }
  if (m_useKLMEnergy) {
    B2WARNING("This ROE has KLM energy included into its 4-vector!");
  }
  if (!m_isFromMC) {
    unsigned int nCharged = getChargedParticles(maskName, 0, unpackComposite).size();
    unsigned int nPhotons = getPhotons(maskName, unpackComposite).size();
    unsigned int nNeutralHadrons = getHadrons(maskName, unpackComposite).size();
    B2INFO(tab << "No. of Charged particles in ROE: " << nCharged);
    B2INFO(tab << "No. of Photons           in ROE: " << nPhotons);
    B2INFO(tab << "No. of K_L0 and neutrons in ROE: " << nNeutralHadrons);
  } else {
    unsigned int nParticles = getParticles(maskName, unpackComposite).size();
    B2INFO(tab << "No. of generated particles in ROE: " << nParticles);
  }
  printIndices(maskName, unpackComposite, tab);
}

void RestOfEvent::printIndices(const std::string& maskName, bool unpackComposite, const std::string& tab) const
{
  auto particles = getParticles(maskName, unpackComposite);
  if (particles.size() == 0) {
    B2INFO(tab << "No indices to print");
    return;
  }
  std::string printoutIndex =  tab + "|";
  std::string printoutPDG =  tab + "|";
  for (const auto particle : particles) {
    printoutIndex += std::to_string(particle->getArrayIndex()) +  " |  ";
    printoutPDG   += std::to_string(particle->getPDGCode()) +  " | ";
  }
  B2INFO(printoutPDG);
  B2INFO(printoutIndex);
}

Particle* RestOfEvent::convertToParticle(const std::string& maskName, int pdgCode, bool isSelfConjugated)
{
  StoreArray<Particle> particles;
  std::set<int> source;
  if (maskName == RestOfEvent::c_defaultMaskName) {
    // if no mask provided work with internal source
    source = m_particleIndices;
  } else {
    bool maskFound = false;
    for (auto& mask : m_masks) {
      if (mask.getName() == maskName) {
        maskFound = true;
        source = mask.getParticles();
        break;
      }
    }
    if (!maskFound) {
      B2FATAL("No '" << maskName << "' mask defined in current ROE!");
    }
  }
  int particlePDG = (pdgCode == 0) ? getPDGCode() : pdgCode;
  auto isFlavored = (isSelfConjugated) ? Particle::EFlavorType::c_Unflavored : Particle::EFlavorType::c_Flavored;
  return particles.appendNew(get4Vector(maskName), particlePDG, isFlavored, std::vector(source.begin(),
                             source.end()), Particle::PropertyFlags::c_IsUnspecified);
}

