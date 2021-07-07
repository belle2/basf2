/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleCombiner/ParticleCombinerModule.h>

// framework aux
#include <framework/logging/Logger.h>

// decay descriptor
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/EvtPDLUtil.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <memory>

using namespace std;

namespace Belle2 {

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

  REG_MODULE(ParticleCombiner)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  ParticleCombinerModule::ParticleCombinerModule() :
    Module()

  {
    // set module description (e.g. insert text)
    setDescription("Makes particle combinations");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("decayString", m_decayString,
             "Input DecayDescriptor string (see :ref:`DecayString`).");
    addParam("cut", m_cutParameter, "Selection criteria to be applied", std::string(""));
    addParam("maximumNumberOfCandidates", m_maximumNumberOfCandidates,
             "Max. number of candidates reconstructed. By default, if the limit is reached no candidates will be produced.\n"
             "This behaviour can be changed by \'ignoreIfTooManyCandidates\' flag.", 10000);

    addParam("ignoreIfTooManyCandidates", m_ignoreIfTooManyCandidates,
             "Don't reconstruct channel if more candidates than given by \'maximumNumberOfCandidates\' are produced.", true);

    addParam("decayMode", m_decayModeID, "User-specified decay mode identifier (saved in 'decayModeID' extra-info for each Particle)",
             0);
    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
    addParam("recoilParticleType", m_recoilParticleType,
             "If not equal 0, the mother Particle is reconstructed in the recoil against the daughter particles.\n"
             "In the case of the following decay chain M -> D1 D2 ... Dn and\n\n"
             ""
             "  a) recoilParticleType = 1: \n\n"
             "    - the mother momentum is given by: p(M) = p(e+e-) - p(D1) - p(D2) - ... - p(DN)\n"
             "    - D1, D2, ..., DN are attached as daughters of M\n\n"
             "  b) recoilParticleType = 2: \n\n"
             "    - the mother momentum is given by: p(M) = p(D1) - p(D2) - ... - p(DN)\n"
             "    - D1, D2, ..., DN are attached as daughters of M\n\n" , 0);
    addParam("chargeConjugation", m_chargeConjugation,
             "If true, the charge-conjugated mode will be reconstructed as well", true);
    addParam("allowChargeViolation", m_allowChargeViolation,
             "If true the decay string does not have to conserve electric charge", false);

    // initializing the rest of private members
    m_pdgCode   = 0;
    m_isSelfConjugatedParticle = false;
    m_generator = nullptr;
  }

  void ParticleCombinerModule::initialize()
  {
    // clear everything
    m_pdgCode = 0;
    m_listName = "";

    // obtain the input and output particle lists from the decay string
    bool valid = m_decaydescriptor.init(m_decayString);
    if (!valid)
      B2ERROR("Invalid input DecayString: " << m_decayString);

    // Mother particle
    const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

    m_pdgCode = mother->getPDGCode();
    m_listName = mother->getFullName();

    m_antiListName = ParticleListName::antiParticleListName(m_listName);
    m_isSelfConjugatedParticle = (m_listName == m_antiListName);

    // Daughters
    int nProducts = m_decaydescriptor.getNDaughters();
    int daughtersNetCharge = 0;
    for (int i = 0; i < nProducts; ++i) {
      const DecayDescriptorParticle* daughter =
        m_decaydescriptor.getDaughter(i)->getMother();
      StoreObjPtr<ParticleList>().isRequired(daughter->getFullName());
      int daughterPDGCode = daughter->getPDGCode();
      if (m_recoilParticleType == 2 && i == 0) {
        daughtersNetCharge -= EvtPDLUtil::charge(daughterPDGCode);
      } else {
        daughtersNetCharge += EvtPDLUtil::charge(daughterPDGCode);
      }
    }

    if (daughtersNetCharge != EvtPDLUtil::charge(m_pdgCode)) {
      if (!m_allowChargeViolation) {
        B2FATAL("Your decay string " << m_decayString << " violates electric charge conservation!\n"
                "If you want to allow this you can set the argument 'allowChargeViolation' to True. Something like:\n"
                "modularAnalysis.reconstructDecay(" << m_decayString << ", your_cuts, allowChargeViolation=True, path=mypath)");
      }
      B2WARNING("Your decay string " << m_decayString << " violates electric charge conservation!\n"
                "Processing is continued assuming that you allowed this deliberately, e.g. for systematic studies etc.");
    }

    m_generator = std::make_unique<ParticleGenerator>(m_decayString, m_cutParameter);

    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    m_outputList.registerInDataStore(m_listName, flags);
    if (!m_isSelfConjugatedParticle && m_chargeConjugation) {
      m_outputAntiList.registerInDataStore(m_antiListName, flags);
    }

    if (m_recoilParticleType != 0 && m_recoilParticleType != 1 && m_recoilParticleType != 2)
      B2FATAL("Invalid recoil particle type = " << m_recoilParticleType <<
              "! Valid values are 0 (not a recoil), 1 (recoiling against e+e- and daughters), 2 (daughter of a recoil)");
  }

  void ParticleCombinerModule::event()
  {
    m_outputList.create();
    m_outputList->initialize(m_pdgCode, m_listName);

    if (!m_isSelfConjugatedParticle && m_chargeConjugation) {
      m_outputAntiList.create();
      m_outputAntiList->initialize(-1 * m_pdgCode, m_antiListName);

      m_outputList->bindAntiParticleList(*(m_outputAntiList));
    }

    m_generator->init();

    int numberOfCandidates = 0;
    while (m_generator->loadNext(m_chargeConjugation)) {

      Particle&& particle = m_generator->getCurrentParticle();

      // if particle is reconstructed in the recoil,
      // its 4-momentum vector needs to be fixed
      // at this stage its 4 momentum is:
      // p(mother) = Sum_i p(daughter_i)
      // but it needs to be
      //  a) in the case of recoilParticleType == 1
      //    - p(mother) = p(e-) + p(e+) - Sum_i p(daughter_i)
      //  b) in the case of recoilParticleType == 2
      //    - p(mother) = p(daughter_0) - Sum_i p(daughter_i) (where i > 0)
      if (m_recoilParticleType == 1) {
        PCmsLabTransform T;
        TLorentzVector recoilMomentum = T.getBeamFourMomentum() - particle.get4Vector();
        particle.set4Vector(recoilMomentum);
      } else if (m_recoilParticleType == 2) {
        const std::vector<Particle*> daughters = particle.getDaughters();

        if (daughters.size() < 2)
          B2FATAL("Reconstructing particle as a daughter of a recoil with less then 2 daughters!");

        TLorentzVector pDaughters;
        for (unsigned i = 1; i < daughters.size(); i++) {
          pDaughters += daughters[i]->get4Vector();
        }

        TLorentzVector mom = daughters[0]->get4Vector() - pDaughters;
        particle.set4Vector(mom);
      }

      numberOfCandidates++;

      if (m_maximumNumberOfCandidates > 0 and numberOfCandidates > m_maximumNumberOfCandidates) {
        if (m_ignoreIfTooManyCandidates) {
          B2WARNING("Maximum number of " << m_maximumNumberOfCandidates << " candidates reached, skipping event");
          m_outputList->clear();
        } else {
          B2WARNING("Maximum number of " << m_maximumNumberOfCandidates << " candidates reached. Ignoring others");
        }
        break;
      }

      Particle* newParticle = m_particles.appendNew(particle);

      m_outputList->addParticle(newParticle);

      // append to the created particle the user specified decay mode ID
      newParticle->addExtraInfo("decayModeID", m_decayModeID);
    }
  }

} // end Belle2 namespace

