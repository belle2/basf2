/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Matic Lubej                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
#include <iostream>

using namespace Belle2;
// New methods:
void RestOfEvent::addParticle(const Particle* particle)
{
  m_particleIndices.insert(particle->getArrayIndex());
}

std::vector<const Particle*> RestOfEvent::getParticles(std::string maskName, bool unpackComposite) const
{
  std::vector<const Particle*> result;
  StoreArray<Particle> allParticles;
  std::set<int> source;
  if (maskName == "") {
    // if no mask provided work with internal source
    source = m_particleIndices;
  } else {
    for (auto& mask : m_masks) {
      if (mask.getName() == maskName) {
        source = mask.getParticles();
        break;
      }
    }
    if (source.size() == 0) {
      B2FATAL("No " << maskName << " mask defined in current ROE!");
    }
  }
  for (const int index : source) {
    if (allParticles[index]->getParticleType() == Particle::EParticleType::c_Composite && unpackComposite) {
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

bool RestOfEvent::hasParticle(const Particle* particle, std::string maskName) const
{
  if (maskName != "" && !hasMask(maskName)) {
    B2FATAL("No " << maskName << " mask defined in current ROE!");
  }

  std::vector<const Particle*> particlesROE = getParticles(maskName);
  return isInParticleList(particle, particlesROE);
}

void RestOfEvent::initializeMask(std::string name, std::string origin)
{
  if (name == "") {
    B2FATAL("Creation of ROE Mask with an empty name is not allowed!");
  }
  if (findMask(name)) {
    B2FATAL("ROE Mask already exists!");
  }
  Mask elon(name, origin);
  m_masks.push_back(elon);
}

void RestOfEvent::excludeParticlesFromMask(std::string maskName, std::vector<const Particle*>& particlesToUpdate,
                                           Particle::EParticleType listType, bool discard)
{
  Mask* mask = findMask(maskName);
  if (!mask) {
    B2FATAL("No " << maskName << " mask defined in current ROE!");
  }
  std::string maskNameToGetParticles = maskName;
  if (!mask->isValid()) {
    maskNameToGetParticles = "";
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
      if (listType != roeParticle->getParticleType()) {
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

void RestOfEvent::updateMaskWithCuts(std::string maskName, std::shared_ptr<Variable::Cut> trackCut,
                                     std::shared_ptr<Variable::Cut> eclCut, std::shared_ptr<Variable::Cut> klmCut, bool updateExisting)
{
  Mask* mask = findMask(maskName);
  if (!mask) {
    B2FATAL("ROE Mask does not exist!");
  }
  std::string sourceName = "";
  if (updateExisting) {
    // if mask already exists, take its particles to update
    sourceName = maskName;
  }
  // get all initial ROE particles, don't touch the possible V0s, otherwise, some daughters may be excluded, and some not... This may be revisited if needed
  std::vector<const Particle*> allROEParticles = getParticles(sourceName, false);
  std::vector<const Particle*> maskedParticles;
  // First check particle type, then check cuts, if no cuts provided, take all particles of this type
  for (auto* particle : allROEParticles) {
    if (particle->getParticleType() == Particle::EParticleType::c_Track && (!trackCut || trackCut->check(particle))) {
      maskedParticles.push_back(particle);
    }
    if (particle->getParticleType() == Particle::EParticleType::c_ECLCluster && (!eclCut || eclCut->check(particle))) {
      maskedParticles.push_back(particle);
    }
    if (particle->getParticleType() == Particle::EParticleType::c_KLMCluster && (!klmCut || klmCut->check(particle))) {
      maskedParticles.push_back(particle);
    }
    // don't lose a possible V0 particle
    if (particle->getParticleType() == Particle::EParticleType::c_Composite) {
      maskedParticles.push_back(particle);
    }
  }
  mask->clearParticles();
  mask->addParticles(maskedParticles);
}

void RestOfEvent::updateMaskWithV0(std::string name, const Particle* particleV0)
{
  Mask* mask = findMask(name);
  if (!mask) {
    B2FATAL("ROE Mask does not exist!");
  }
  std::vector<const Particle*> allROEParticles = getParticles(name);
  std::vector<int> indicesToErase;
  std::vector<const Particle*> daughtersV0 =  particleV0->getFinalStateDaughters();
  for (auto* maskParticle : allROEParticles) {
    bool toKeep = true;
    for (auto* daughterV0 : daughtersV0) {
      if (daughterV0->isCopyOf(maskParticle)) {
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

bool RestOfEvent::checkCompatibilityOfMaskAndV0(std::string name, const Particle* particleV0)
{
  Mask* mask = findMask(name);
  if (!mask) {
    B2FATAL("ROE Mask does not exist!");
  }
  if (!mask->isValid()) {
    return false; //We should have particles here!
  }
  if (particleV0->getParticleType() != Particle::EParticleType::c_Composite) {
    return false;
  }
  std::vector<const Particle*> daughtersV0 =  particleV0->getFinalStateDaughters();
  for (auto* daughter : daughtersV0) {
    if (daughter->getParticleType() != Particle::EParticleType::c_Track) {
      return false; // Non tracks are not supported yet
    }
  }
  if (mask->hasV0(particleV0)) {
    return false; // We are not going to add another one
  }
  return true;
}

bool RestOfEvent::hasMask(std::string name) const
{
  for (auto& mask : m_masks) {
    if (mask.getName() == name) {
      return true;
    }
  }
  return false;
}
TLorentzVector RestOfEvent::get4Vector(std::string maskName) const
{
  TLorentzVector roe4Vector;
  std::vector<const Particle*> myParticles = RestOfEvent::getParticles(maskName);
  for (const Particle* particle : myParticles) {
    // TODO: Why it was excluded in ROE 1.0? It should be allowed
    if (particle->getParticleType() == Particle::EParticleType::c_KLMCluster) {
      continue;
    }
    roe4Vector += particle->get4Vector();
  }
  return roe4Vector;
}


RestOfEvent::Mask* RestOfEvent::findMask(std::string& name)
{
  for (auto& mask : m_masks) {
    if (mask.getName() == name) {
      return &mask;
    }
  }
  return nullptr;

}
std::vector<const Track*> RestOfEvent::getTracks(std::string maskName) const
{
  std::vector<const Track*> result;
  std::vector<const Particle*> allParticles = getParticles(maskName);
  for (auto* particle : allParticles) {
    if (particle->getParticleType() == Particle::EParticleType::c_Track) {
      result.push_back(particle->getTrack());
    }
  }
  return result;
}
std::vector<const ECLCluster*> RestOfEvent::getECLClusters(std::string maskName) const
{
  std::vector<const ECLCluster*> result;
  std::vector<const Particle*> allParticles = getParticles(maskName);
  for (auto* particle : allParticles) {
    if (particle->getParticleType() == Particle::EParticleType::c_ECLCluster) {
      result.push_back(particle->getECLCluster());
    }
  }
  return result;
}
std::vector<const KLMCluster*> RestOfEvent::getKLMClusters(std::string maskName) const
{
  std::vector<const KLMCluster*> result;
  std::vector<const Particle*> allParticles = getParticles(maskName);
  for (auto* particle : allParticles) {
    if (particle->getParticleType() == Particle::EParticleType::c_KLMCluster) {
      result.push_back(particle->getKLMCluster());
    }
  }
  return result;
}
int RestOfEvent::getNTracks(std::string maskName) const
{
  int nTracks = getTracks(maskName).size();
  return nTracks;
}
int RestOfEvent::getNECLClusters(std::string maskName) const
{
  int nROEECLClusters = getECLClusters(maskName).size();
  return nROEECLClusters;
}
int RestOfEvent::getNKLMClusters(std::string maskName) const
{
  int nROEKLMClusters = getKLMClusters(maskName).size();
  return nROEKLMClusters;
}
void RestOfEvent::updateChargedStableFractions(std::string maskName, std::vector<double>& fractions)
{
  Mask* mask = findMask(maskName);
  if (!mask) {
    B2FATAL("No " << maskName << " mask defined in current ROE!");
  }
  if (fractions.size() != Const::ChargedStable::c_SetSize) {
    B2WARNING("Charged stable fractions have incorrect size of array!");
  }
  mask->setChargedStableFractions(fractions);
}
std::vector<double> RestOfEvent::getChargedStableFractions(std::string maskName) const
{
  std::vector<double> maskFractions = {0, 0, 1, 0, 0, 0};
  for (auto& mask : m_masks) {
    if (mask.getName() == maskName && maskFractions.size() == Const::ChargedStable::c_SetSize) {
      maskFractions = mask.getChargedStableFractions();
    }
  }
  return maskFractions;
}

//
// Old methods:
//
TLorentzVector RestOfEvent::get4VectorTracks(std::string maskName) const
{
  StoreArray<Particle> particles;
  std::vector<const Track*> roeTracks = RestOfEvent::getTracks(maskName);

  // Collect V0 momenta
  TLorentzVector roe4VectorTracks;
  //TODO: untested: test and move this method to variables!
  //std::vector<unsigned int> v0List = RestOfEvent::getV0IDList(maskName);
  //for (unsigned int iV0 = 0; iV0 < v0List.size(); iV0++)
  //  roe4VectorTracks += particles[v0List[iV0]]->get4Vector();
  //const unsigned int n = Const::ChargedStable::c_SetSize;
  auto fractions = getChargedStableFractions(maskName);
  //double fractions[n] = {0,0,1,0,0,0};
  //fillFractions(fractions, maskName);

  // Add momenta from other tracks
  for (unsigned int iTrack = 0; iTrack < roeTracks.size(); iTrack++) {

    const Track* track = roeTracks[iTrack];
    const PIDLikelihood* pid = track->getRelatedTo<PIDLikelihood>();
    const MCParticle* mcp = roeTracks[iTrack]->getRelatedTo<MCParticle>();

    if (!pid) {
      B2ERROR("Track with no PID information!");
      continue;
    }

    double particlePDG;

    // MC, if available
    /////////////////////////////////////////
    double mcChoice = Const::pion.getPDGCode();
    if (mcp) {
      int mcpdg = abs(mcp->getPDG());
      if (Const::chargedStableSet.contains(Const::ParticleType(mcpdg))) {
        mcChoice = mcpdg;
      }
    }

    // PID for Belle
    //////////////////////////////////////////
    double pidChoice = Const::pion.getPDGCode();
    // Set variables
    Const::PIDDetectorSet set = Const::ECL;
    double eIDBelle = pid->getProbability(Const::electron, Const::pion, set);
    double muIDBelle = 0.5;
    if (pid->isAvailable(Const::KLM))
      muIDBelle = exp(pid->getLogL(Const::muon, Const::KLM));
    double atcPIDBelle_Kpi = atcPIDBelleKpiFromPID(pid);

    // Check for leptons, else kaons or pions
    if (eIDBelle > 0.9 and eIDBelle > muIDBelle)
      pidChoice = Const::electron.getPDGCode();
    else if (muIDBelle > 0.9 and eIDBelle < muIDBelle)
      pidChoice = Const::muon.getPDGCode();
    // Check for kaons, else pions
    else if (atcPIDBelle_Kpi > 0.6)
      pidChoice = Const::kaon.getPDGCode();
    // Assume pions
    else
      pidChoice = Const::pion.getPDGCode();

    // Most likely
    //////////////////////////////////////////////
    // TODO: SET TO PION UNTILL MOST LIKELY FUNCTION IS RELIABLE
    double fracChoice = Const::pion.getPDGCode();

    // Use MC mass hypothesis
    if (fractions[0] == -1)
      particlePDG = mcChoice;
    // Use Belle case
    else if (fractions[0] == -2)
      particlePDG = pidChoice;
    // Use fractions
    else
      particlePDG = fracChoice;

    Const::ChargedStable trackParticle = Const::ChargedStable(particlePDG);
    const TrackFitResult* tfr = roeTracks[iTrack]->getTrackFitResultWithClosestMass(trackParticle);

    // Set energy of track
    double tempMass = trackParticle.getMass();
    TVector3 tempMom = tfr->getMomentum();
    TLorentzVector temp4Vector;
    temp4Vector.SetVect(tempMom);
    temp4Vector.SetE(TMath::Sqrt(tempMom.Mag2() + tempMass * tempMass));

    roe4VectorTracks += temp4Vector;
  }

  return roe4VectorTracks;
}

TLorentzVector RestOfEvent::get4VectorNeutralECLClusters(std::string maskName) const
{
  std::vector<const ECLCluster*> roeClusters = RestOfEvent::getECLClusters(maskName);
  TLorentzVector roe4VectorECLClusters;

  // Add all momenta from neutral ECLClusters
  ClusterUtils C;
  for (unsigned int iEcl = 0; iEcl < roeClusters.size(); iEcl++) {
    if (roeClusters[iEcl]->isNeutral())
      roe4VectorECLClusters += C.Get4MomentumFromCluster(roeClusters[iEcl]);
  }

  return roe4VectorECLClusters;
}

bool RestOfEvent::isInParticleList(const Particle* roeParticle, std::vector<const Particle*>& particlesToUpdate) const
{
  for (auto* listParticle : particlesToUpdate) {
    if (roeParticle->isCopyOf(listParticle)) {
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

void RestOfEvent::print() const
{
  B2INFO(" - Particles[" << m_particleIndices.size() << "] : ");
  printIndices(m_particleIndices);
  for (auto mask : m_masks) {
    mask.print();
  }
}

void RestOfEvent::printIndices(std::set<int> indices) const
{
  if (indices.empty())
    return;

  std::string printout =  "     -> ";
  for (const int index : indices) {
    printout += std::to_string(index) +  ", ";
  }
  B2INFO(printout);
}

double RestOfEvent::atcPIDBelleKpiFromPID(const PIDLikelihood* pid) const
{
  // ACC = ARICH
  Const::PIDDetectorSet set = Const::ARICH;
  double acc_sig = exp(pid->getLogL(Const::kaon, set));
  double acc_bkg = exp(pid->getLogL(Const::pion, set));
  double acc = 0.5; // Belle standard
  if (acc_sig + acc_bkg  > 0.0)
    acc = acc_sig / (acc_sig + acc_bkg);

  // TOF = TOP
  set = Const::TOP;
  double tof_sig = exp(pid->getLogL(Const::kaon, set));
  double tof_bkg = exp(pid->getLogL(Const::pion, set));
  double tof = 0.5; // Belle standard
  double tof_all = tof_sig + tof_bkg;
  if (tof_all != 0) {
    tof = tof_sig / tof_all;
    if (tof < 0.001) tof = 0.001;
    if (tof > 0.999) tof = 0.999;
  }

  // dE/dx = CDC
  set = Const::CDC;
  double cdc_sig = exp(pid->getLogL(Const::kaon, set));
  double cdc_bkg = exp(pid->getLogL(Const::pion, set));
  double cdc = 0.5; // Belle standard
  double cdc_all = cdc_sig + cdc_bkg;
  if (cdc_all != 0) {
    cdc = cdc_sig / cdc_all;
    if (cdc < 0.001) cdc = 0.001;
    if (cdc > 0.999) cdc = 0.999;
  }

  // Combined
  double pid_sig = acc * tof * cdc;
  double pid_bkg = (1. - acc) * (1. - tof) * (1. - cdc);

  return pid_sig / (pid_sig + pid_bkg);
}
