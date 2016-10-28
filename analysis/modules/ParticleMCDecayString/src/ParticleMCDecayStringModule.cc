/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/ParticleMCDecayString/ParticleMCDecayStringModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/DecayHashMap.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/utility/MCMatching.h>

#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  REG_MODULE(ParticleMCDecayString)

  ParticleMCDecayStringModule::ParticleMCDecayStringModule() : Module()
  {
    setDescription("Adds the Monte Carlo decay string to a Particle. "
                   "This is done in the form of a hash, which is currently just an int. "
                   "The hash can later on be used to get the actual decay string using the DecayHashMap, or using the 'MCDecayString' ntuple tool."
                   "IMPORTANT: We have to convert the int to a float, this does is done BITWISE. So you also must convert the float in your ntuple back to an integer BITWISE!");
    setPropertyFlags(c_ParallelProcessingCertified);
    addParam("listName", m_listName, "Particles from these ParticleList are used as input.");
    addParam("removeFSR", m_removeFSR, "If true, final state radiation (FSR) photons are removed from the decay string.", true);
    addParam("printHashes", m_printHashes, "If true, each new hash will be printed on stdout", false);
  }

  void ParticleMCDecayStringModule::initialize()
  {
    StoreObjPtr<ParticleList>::required(m_listName);

    StoreObjPtr<DecayHashMap> dMap("", DataStore::c_Persistent);
    if (not dMap.isValid()) {
      dMap.registerInDataStore();
      dMap.create();
    }

  }

  void ParticleMCDecayStringModule::event()
  {
    StoreObjPtr<DecayHashMap> dMap("", DataStore::c_Persistent);

    StoreObjPtr<ParticleList> pList(m_listName);

    for (unsigned iParticle = 0; iParticle < pList->getListSize(); ++iParticle) {
      Particle* particle = pList->getParticle(iParticle);

      const std::string decayString = getDecayString(*particle);

      // Convert unsigned int decay hash into a float keeping the same bit pattern
      assert(sizeof(float) == sizeof(unsigned int));
      union convert {
        unsigned int i;
        float f;
      };

      convert hash;
      hash.i = dMap->addDecayHash(decayString, m_printHashes);
      particle->addExtraInfo(c_ExtraInfoName, hash.f);
    }
  }


  const MCParticle* ParticleMCDecayStringModule::getInitialParticle(const MCParticle* mcP)
  {
    const MCParticle* mcPMother = mcP->getMother();
    if (mcPMother == nullptr) {
      return mcP;
    } else {
      return getInitialParticle(mcPMother);
    }
  }

  /** defines what is a final state particle for this purpose. */
  bool isFSP(int pdg)
  {
    switch (abs(pdg)) {
      case 211:   //pi^+
      case 321:   //K^+
      case 11:    //e
      case 12:    //nu_e
      case 13:    //mu
      case 14:    //nu_mu
      case 16:    //nu_tau
      case 22:    //gamma
      case 310:   //K_S
      case 130:   //K_L
      case 2112:  //n
      case 2212:  //p
        return 1;
      default:
        return 0;
    }
  }

  std::string ParticleMCDecayStringModule::getDecayString(const Particle& p)
  {

    std::string output;
    output += getDecayStringFromParticle(&p) + " | ";
    output += getMCDecayStringFromParticle(&p);
    return output;

  }

  std::string ParticleMCDecayStringModule::getDecayStringFromParticle(const Particle* p)
  {

    std::string output = " ";

    output += std::to_string(p->getPDGCode());

    if (not isFSP(p->getPDGCode())) {
      output += " (-->";
      for (auto daughter : p->getDaughters()) {
        output += getDecayStringFromParticle(daughter);
      }
      output += ")";
    }

    return output;

  }

  std::string ParticleMCDecayStringModule::getMCDecayStringFromParticle(const Particle* p)
  {

    std::string output;

    output = getMCDecayStringFromMCParticle(p->getRelatedTo<MCParticle>());
    for (auto& daughter : p->getDaughters()) {
      output += " | " + getMCDecayStringFromParticle(daughter);
    }

    return output;

  }

  std::string ParticleMCDecayStringModule::getMCDecayStringFromMCParticle(const MCParticle* mcPMatched)
  {

    if (mcPMatched == nullptr)
      return "(No match)";

    // TODO Performance can be optimized, this mcPMother does not change during the construction
    const MCParticle* mcPMother = getInitialParticle(mcPMatched);

    std::string decayString = buildMCDecayString(mcPMother, mcPMatched);

    if (mcPMatched->getPDG() == 10022)
      return decayString + " (Virtual gamma match)";
    return decayString;
  }


  std::string ParticleMCDecayStringModule::buildMCDecayString(const MCParticle* mcPMother, const MCParticle* mcPMatched)
  {
    if (m_removeFSR and mcPMother->getPDG() == 22 and MCMatching::isFSR(mcPMother))
      return "";

    std::stringstream ss;
    ss << " ";
    if (mcPMother->getArrayIndex() == mcPMatched->getArrayIndex()) {
      ss << "^";
    }

    ss << mcPMother->getPDG();

    if (not isFSP(mcPMother->getPDG())) {
      ss << " (-->";
      for (auto daughter : mcPMother->getDaughters()) {
        ss << buildMCDecayString(daughter, mcPMatched);
      }
      ss << ")";
    }

    return ss.str();
  }

} // Belle2 namespace

