/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/FSRCorrection/FSRCorrectionModule.h>


// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/ReferenceFrame.h>


// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <math.h>
#include <algorithm>

using namespace std;

namespace Belle2 {

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

  REG_MODULE(FSRCorrection)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  FSRCorrectionModule::FSRCorrectionModule() :
    Module()

  {
    // set module description (e.g. insert text)
    setDescription("Adds radiative photons to lepton");
    // fixme: no clue here
    // setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("inputListName", m_inputListName, "The initial lepton list containing the leptons to correct, should already exists.");
    addParam("outputListName", m_outputListName, "The output lepton list containing the corrected leptons.");
    addParam("gammaListName", m_gammaListName, "The gammas list containing possibly radiative gammas, should already exist.");
    addParam("angleThreshold", m_angleThres,
             "The maximal accepted angle (in degrees) between the lepton and the (radiative) gamma to be accepted", 5.0);
    addParam("energyThreshold", m_energyThres, "The maximum energy of the (radiative) gamma to be accepted.", 1.0);
    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);

    // initializing the rest of private members
    m_pdgCode   = 0;
  }

  void FSRCorrectionModule::initialize()
  {
    m_pdgCode = 0;

    // check the validity of output ParticleList name
    bool valid = m_decaydescriptor.init(m_outputListName);
    if (!valid)
      B2ERROR("FSRCorrectionModule::initialize Invalid output ParticleList name: " << m_outputListName);

    // output particle
    const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
    m_pdgCode  = mother->getPDGCode();
    m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);

    // get exiting particle lists
    if (m_inputListName == m_outputListName) {
      B2ERROR("FSRCorrectionModule::initialize Input and output list names " << m_inputListName << " are the same.");
    } else if (!m_decaydescriptor.init(m_inputListName)) {
      B2ERROR("FSRCorrectionModule::initialize Invalid input ParticleList name: " << m_inputListName);
    } else {
      StoreObjPtr<ParticleList>::required(m_inputListName);
    }

    if (!m_decaydescriptorGamma.init(m_gammaListName)) {
      B2ERROR("FSRCorrectionModule::initialize Invalid input ParticleList name: " << m_gammaListName);
    } else {
      StoreObjPtr<ParticleList>::required(m_gammaListName);
    }

    // make output list
    StoreObjPtr<ParticleList> particleList(m_outputListName);
    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    particleList.registerInDataStore(flags);
    StoreObjPtr<ParticleList> antiParticleList(m_outputAntiListName);
    antiParticleList.registerInDataStore(flags);
  }


  void FSRCorrectionModule::event()
  {

    // clear the list
    m_particlesInTheList.clear();
    m_usedGammas.clear();

    double cos_max = -1.;
    double cos_limit = cos(m_angleThres * M_PI / 180.0);
    const StoreArray<Particle> particles;
    StoreObjPtr<ParticleList> plist(m_outputListName);
    StoreObjPtr<ParticleList> gammaList(m_gammaListName);
    bool existingList = plist.isValid();

    if (!existingList) {
      // new particle list: create it
      plist.create();
      plist->initialize(m_pdgCode, m_outputListName);

      StoreObjPtr<ParticleList> antiPlist(m_outputAntiListName);
      antiPlist.create();
      antiPlist->initialize(-1 * m_pdgCode, m_outputAntiListName);
      antiPlist->bindAntiParticleList(*(plist));

    } else  {
      // output list already contains Particles
      // fill m_particlesInTheList with unique
      // identifiers of particles already in
      for (unsigned i = 0; i < plist->getListSize(); i++) {
        const Particle* particle = plist->getParticle(i);

        std::vector<int> idSeq;
        fillUniqueIdentifier(particle, idSeq);
        m_particlesInTheList.push_back(idSeq);
      }
    }


    // copy all particles from input lists into plist and add a radiative gamma if found
    const StoreObjPtr<ParticleList> inPList(m_inputListName);
    std::vector<int> fsParticles     = inPList->getList(ParticleList::EParticleType::c_FlavorSpecificParticle, false);
    const std::vector<int>& fsAntiParticles = inPList->getList(ParticleList::EParticleType::c_FlavorSpecificParticle, true);

    fsParticles.insert(fsParticles.end(), fsAntiParticles.begin(), fsAntiParticles.end());

    // loop over lepton list
    for (unsigned i = 0; i < fsParticles.size(); i++) {
      Particle* lepton = particles[fsParticles[i]];
      TLorentzVector lepton4Vector = lepton->get4Vector();
      TLorentzVector gamma4Vector;
      bool foundGamma = false;
      int gammaMdstIndex = -999;


      // loop over gamma list
      for (unsigned j = 0; j < gammaList->getListSize(); j++) {
        Particle* gamma = gammaList->getParticle(j);

        // check if gamma energy is below threshold
        if (gamma->getEnergy() > m_energyThres) continue;

        // check if gamma was already used
        int currentGamma = gamma->getMdstArrayIndex();
        bool gammaUsed = std::find(m_usedGammas.begin(), m_usedGammas.end(), currentGamma) != m_usedGammas.end();
        if (gammaUsed) continue;

        // get angle
        // todo use a more advanced angle
        // fixme do I use the correct frame
        const auto& frame = ReferenceFrame::GetCurrent();
        TVector3 pi = frame.getMomentum(lepton).Vect();
        TVector3 pj = frame.getMomentum(gamma).Vect();
        auto cos_angle = cos(pi.Angle(pj));

        if (cos_angle > cos_max) {
          cos_max = cos_angle;
          if (cos_max > cos_limit) {
            foundGamma = true;
            gammaMdstIndex = gamma->getMdstArrayIndex();
            gamma4Vector = gamma->get4Vector();

          }
        }
      }

      // add 4-vector
      if (foundGamma) {
        B2INFO("FSRCorrectionModule::event Found a radiative gamma and added its 4-vector to the lepton");
        m_usedGammas.push_back(gammaMdstIndex);
        lepton->set4Vector(lepton4Vector + gamma4Vector);
      }

      std::vector<int> idSeq;
      fillUniqueIdentifier(lepton, idSeq);
      bool uniqueSeq = isUnique(idSeq);

      if (uniqueSeq) {
        plist->addParticle(lepton);
        m_particlesInTheList.push_back(idSeq);
      }
    }
  }


  void FSRCorrectionModule::fillUniqueIdentifier(const Particle* p, std::vector<int>& idSequence)
  {
    idSequence.push_back(p->getPDGCode());

    if (p->getNDaughters() == 0) {
      idSequence.push_back(p->getMdstArrayIndex());
    } else {
      idSequence.push_back(p->getNDaughters());
      // this is not FSP (go one level down)
      for (unsigned i = 0; i < p->getNDaughters(); i++)
        fillUniqueIdentifier(p->getDaughter(i), idSequence);
    }
  }


  bool FSRCorrectionModule::isUnique(const std::vector<int>& idSeqOUT)
  {
    for (unsigned i = 0; i < m_particlesInTheList.size(); i++) {
      std::vector<int> idSeqIN = m_particlesInTheList[i];

      bool sameSeq = (idSeqIN == idSeqOUT);
      if (sameSeq)
        return false;
    }

    return true;
  }

} // end Belle2 namespace

