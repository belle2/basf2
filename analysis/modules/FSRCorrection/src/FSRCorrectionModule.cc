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
//    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("leptonListName", m_leptonListName, "The initial lepton list which should already exists.");
    addParam("outputListName", m_outputListName, "The corrected lepton list which should already exists.");
    addParam("gammaListName", m_gammaListName, "The list containing the gammas which are considered as possibly radiative gammas");
    addParam("thetaThreshold", m_thetaThres,
             "The maximal accepted angle between the lepton and the gamma to be accepted to be radiative", 5.0);
    // fixme is unit of energy really gev?
    addParam("energyThreshold", m_energyThres, "The maximum energy of the gamma to be accepted.", 1.0);
    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);

    // initializing the rest of private memebers
    m_pdgCode   = 0;

    // original code can be found here: /sw/belle/belle/b20090127_0910/src/anal/eid/eid/eid.h /sw/belle/belle/b20090127_0910/src/anal/eid/src/eid.cc
  }

  void FSRCorrectionModule::initialize()
  {
    // 1) fixme clear everything
    m_pdgCode = 0;

    // check the validity of output ParticleList name
    bool valid = m_decaydescriptor.init(m_outputListName);
    if (!valid)
      B2ERROR("ParticleListManipulatorModule::initialize Invalid output ParticleList name: " << m_outputListName);

    //Output particle
    const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
    m_pdgCode  = mother->getPDGCode();
    m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);

    B2INFO(" pdg code in initilize " << m_pdgCode);

    // 2) get exiting particle lists
    if (m_leptonListName == m_outputListName) {
      B2ERROR("FSRCorrection: input and output list names " << m_leptonListName << " are the same.");
    } else if (!m_decaydescriptor.init(m_leptonListName)) {
      B2ERROR("Invalid input ParticleList name: " << m_leptonListName);
    } else {
      StoreObjPtr<ParticleList>::required(m_leptonListName);
    }

    // fixme maybe I should check more (e.g. pdgoflist == gamma, no input nor outputlist, etc)
    if (!m_decaydescriptorGamma.init(m_gammaListName)) {
      B2ERROR("Invalid input ParticleList name: " << m_gammaListName);
    } else {
      StoreObjPtr<ParticleList>::required(m_gammaListName);
    }

    // 3) make a new list
    StoreObjPtr<ParticleList> particleList(m_outputListName);
    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    particleList.registerInDataStore(flags);
    StoreObjPtr<ParticleList> antiParticleList(m_outputAntiListName);
    antiParticleList.registerInDataStore(flags);
  }


  void FSRCorrectionModule::event()
  {

    // clear the list
    // fixme more to clear?
    m_particlesInTheList.clear();
    m_usedGammas.clear();

    double cos_max = -1.;
    double cos_limit = cos(m_thetaThres * M_PI / 180.0);
    B2INFO("COS LIM " << cos_limit);
    const StoreArray<Particle> particles;
    StoreObjPtr<ParticleList> plist(m_outputListName);
    StoreObjPtr<ParticleList> gammaList(m_gammaListName);
    bool existingList = plist.isValid();

    if (!existingList) {
      // new particle list: create it
      plist.create();
      B2INFO(" plist init pdg " << m_pdgCode);
      plist->initialize(m_pdgCode, m_outputListName);
      B2INFO(" plist after init  pdg " << plist->getPDGCode());

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


    // copy all particles from input lists into plist

    const StoreObjPtr<ParticleList> inPList(m_leptonListName);

    std::vector<int> fsParticles     = inPList->getList(ParticleList::EParticleType::c_FlavorSpecificParticle, false);
    const std::vector<int>& fsAntiParticles = inPList->getList(ParticleList::EParticleType::c_FlavorSpecificParticle, true);

    fsParticles.insert(fsParticles.end(), fsAntiParticles.begin(), fsAntiParticles.end());

    B2INFO("electon list size " <<  fsParticles.size());
    for (unsigned i = 0; i < fsParticles.size(); i++) {
      B2INFO("at electecton number " << i);
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
        // fixme get the angles in the right planes and make proper checks
        // fixme warum das mit dem frame
        const auto& frame = ReferenceFrame::GetCurrent();
        TVector3 pi = frame.getMomentum(lepton).Vect();
        TVector3 pj = frame.getMomentum(gamma).Vect();
        // fixme gibt das den cosine zurueck?
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

      // add 4vector
      if (foundGamma) {

        B2INFO("COS LIM " << cos_limit);
        B2INFO("COSmax of this gamma " << cos_max);
        B2INFO("Found a radiative gamma and add the 4 Vector to the lepton");
        B2INFO("pdg before " << lepton->getPDGCode());
        B2INFO("4vector before " << lepton->get4Vector().Px() << " " <<  lepton->get4Vector().Py() << " " <<  lepton->get4Vector().Pz() <<
               " " <<  lepton->get4Vector().Energy());
        m_usedGammas.push_back(gammaMdstIndex);
        lepton->set4Vector(lepton4Vector + gamma4Vector);
        B2INFO("4vector after " << lepton->get4Vector().Px() << " " <<  lepton->get4Vector().Py() << " " <<  lepton->get4Vector().Pz() <<
               " " <<  lepton->get4Vector().Energy());
        B2INFO("pdg after " << lepton->getPDGCode());
      }

      B2INFO("still at electecton number " << i);
      std::vector<int> idSeq;
      fillUniqueIdentifier(lepton, idSeq);
      bool uniqueSeq = isUnique(idSeq);

      if (uniqueSeq) {
        B2INFO("addiding electontr " << i);
        B2INFO("pdg lepton " << lepton->getPDGCode());
        B2INFO("pdg list " << plist->getPDGCode());
        B2INFO("4vector when adding " << lepton->get4Vector().Px() << " " <<  lepton->get4Vector().Py() << " " <<  lepton->get4Vector().Pz()
               << " " <<  lepton->get4Vector().Energy());
        plist->addParticle(lepton);

        B2INFO("currecnt list size " <<  plist->getListSize());
        m_particlesInTheList.push_back(idSeq);
      }
    }
    B2INFO("AD|FTER electon list size " <<  plist->getListSize());
  }

// fixme klaeren mit den funcktionene (wann kann einen list schon exitieren?)
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

