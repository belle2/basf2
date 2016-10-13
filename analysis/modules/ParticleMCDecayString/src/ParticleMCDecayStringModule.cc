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
                   "The hash can later on be used to get the actual decay string using the DecayHashMap, or using the 'MCDecayString' ntuple tool.");
    setPropertyFlags(c_ParallelProcessingCertified);
    addParam("listName", m_listName, "Particles from these ParticleList are used as input.");
    addParam("motherPDGs", m_motherPDGs, "PDG codes of potential mother particles (absolute values)", std::vector<int>({511, 521, 531}));
    addParam("removeFSR", m_removeFSR, "If true, final state radiation (FSR) photons are removed from the decay string.", true);
    addParam("printHashes", m_printHashes, "If true, each new hash will be printed on stdout", false);
  }

  void ParticleMCDecayStringModule::initialize()
  {
    StoreObjPtr<ParticleList>::required(m_listName);

    StoreObjPtr<DecayHashMap> dMap("", DataStore::c_Persistent);
    dMap.registerInDataStore();
    dMap.create();

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


  const MCParticle* ParticleMCDecayStringModule::getMother(const MCParticle& mcP)
  {
    const int pdg = mcP.getPDG();

    if (std::find(m_motherPDGs.begin(), m_motherPDGs.end(), abs(pdg)) != m_motherPDGs.end()) {
      return &mcP;
    } else {
      const MCParticle* mcPMother = mcP.getMother();
      if (mcPMother == nullptr) {
        return nullptr;
      } else {
        return getMother(*mcPMother);
      }
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
      case 111:   //pi^0
      case 310:   //K_S
      case 130:   //K_L
      case 2112:  //n
      case 2212:  //p
        return 1;
      default:
        return 0;
    }
  }


  std::string ParticleMCDecayStringModule::buildDecayString(const MCParticle& mcPMother, const MCParticle& mcPMatched)
  {
    if (m_removeFSR and mcPMother.getPDG() == 22 and MCMatching::isFSR(&mcPMother))
      return "";

    std::stringstream ss;
    ss << " ";
    if (mcPMother.getArrayIndex() == mcPMatched.getArrayIndex()) {
      ss << "^";
    }

    if (isFSP(mcPMother.getPDG())) {
      ss << mcPMother.getPDG();
    } else {
      ss << mcPMother.getPDG() << " (-->";
      for (auto daughter : mcPMother.getDaughters()) {
        ss << buildDecayString(*daughter, mcPMatched);
      }
      ss << ")";
    }

    return ss.str();
  }

  std::string ParticleMCDecayStringModule::getDecayString(const Particle& p)
  {
    const MCParticle* mcPMatched = p.getRelatedTo<MCParticle>();
    if (mcPMatched == nullptr)
      return ("No MC match");
    else if (mcPMatched->getPDG() == 300553)
      return ("Y(4S) match");
    else if (mcPMatched->getPDG() == 10022)
      return ("Virtual gamma match");

    const MCParticle* mcPMother = getMother(*mcPMatched);

    if (mcPMother == nullptr)
      return ("No mother found");

    return buildDecayString(*mcPMother, *mcPMatched);
  }

} // Belle2 namespace

