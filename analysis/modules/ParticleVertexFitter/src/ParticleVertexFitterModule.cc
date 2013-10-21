/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleVertexFitter/ParticleVertexFitterModule.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <TMath.h>


using namespace std;

namespace Belle2 {


  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleVertexFitter)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleVertexFitterModule::ParticleVertexFitterModule() : Module(),
    m_Bfield(0)
  {
    // set module description (e.g. insert text)
    setDescription("Vertex fitter for modular analysis");
    setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    // Add parameters
    addParam("ListName", m_listName, "name of particle list", string(""));
    addParam("ConfidenceLevel", m_confidenceLevel,
             "required confidence level of fit to keep particles in the list", 0.001);
    addParam("VertexFitter", m_vertexFitter, "kfitter or rave", string("kfitter"));

  }

  ParticleVertexFitterModule::~ParticleVertexFitterModule()
  {
  }

  void ParticleVertexFitterModule::initialize()
  {
    m_Bfield = 1.5;
    analysis::RaveSetup::initialize(1, m_Bfield);
  }

  void ParticleVertexFitterModule::beginRun()
  {
    m_Bfield = 1.5; //TODO: get from gearbox
    std::string rfs("rave");
    if (m_vertexFitter.compare(rfs) == 0)
      B2WARNING("RAVE works only for single vertex non kinematic fit ");
  }

  void ParticleVertexFitterModule::event()
  {

    StoreObjPtr<ParticleList> plist(m_listName);
    if (!plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    StoreArray<Particle> Particles(plist->getParticleCollectionName());

    for (unsigned k = 0; k < plist->getFlavorType() + 1; k++) {
      for (unsigned i = 0; i < plist->getList(k).size(); i++) {
        unsigned index = plist->getList(k)[i];
        Particle* particle = Particles[index];
        bool ok = doVertexFit(particle);
        if (!ok) plist->markToRemove(i, k);
      }
    }
    plist->removeMarked();

    // printout with B2INFO

    int pdg = plist->getPDG();
    switch (plist->getFlavorType()) {
      case 0:
        B2INFO("ParticleVertexFitter: " << pdg << " " << m_listName
               << " size=" << plist->getList(0).size());
        break;
      case 1:
        B2INFO("ParticleVertexFitter: " << pdg << " " << m_listName
               << " size="
               << plist->getList(0).size() << "+" << plist->getList(1).size());
        break;
      default:
        B2ERROR("ParticleVertexFitter: " << pdg << " " << m_listName << " ***invalid flavor type "
                << plist->getFlavorType());
    }

  }


  void ParticleVertexFitterModule::endRun()
  {
  }

  void ParticleVertexFitterModule::terminate()
  {
  }

  void ParticleVertexFitterModule::printModuleParams() const
  {
  }


  bool ParticleVertexFitterModule::doVertexFit(Particle* mother)
  {
    // steering starts here
    bool ok = false;
    std::string kfs("kfitter");
    std::string rfs("rave");

    if (m_vertexFitter.compare(kfs) == 0) ok = doKvFit(mother);
    if (m_vertexFitter.compare(rfs) == 0) ok = doRaveFit(mother);
    if (m_vertexFitter.compare(kfs) != 0 && m_vertexFitter.compare(rfs) != 0)
      B2ERROR("ParticleVertexFitter: " << m_vertexFitter << " ***invalid vertex fitter ");

    if (!ok) return false;

    // steering ends here

    if (mother->getPValue() < m_confidenceLevel) return false;
    return true;

  }


  bool ParticleVertexFitterModule::doKvFit(Particle* mother)
  {
    if (mother->getNDaughters() < 2) return false;

    analysis::VertexFitKFit kv;
    kv.setMagneticField(m_Bfield);

    for (unsigned ichild = 0; ichild < mother->getNDaughters(); ichild++) {
      const Particle* child = mother->getDaughter(ichild);
      if (child->getPValue() < 0) return false; // error matrix not valid
      CLHEP::HepLorentzVector mom(child->getPx(),
                                  child->getPy(),
                                  child->getPz(),
                                  child->getEnergy());
      HepPoint3D pos(child->getX(), child->getY(), child->getZ());
      TMatrixFSym errMatrix = child->getMomentumVertexErrorMatrix();
      CLHEP::HepSymMatrix covMatrix(7);
      for (int i = 0; i < 7; i++) {
        for (int j = i; j < 7; j++) {
          covMatrix[i][j] = errMatrix[i][j];
        }
      }
      kv.addTrack(mom, pos, covMatrix, child->getCharge());
    }

    int err = kv.doFit();
    if (err != 0) return false;

    bool ok = makeKvMother(kv, mother);
    return ok;
  }



  bool ParticleVertexFitterModule::makeKvMother(analysis::VertexFitKFit& kv,
                                                Particle* mother)
  {

    analysis::MakeMotherKFit kmm;
    kmm.setMagneticField(m_Bfield);

    unsigned n = kv.getTrackCount();
    for (unsigned i = 0; i < n; ++i) {
      kmm.addTrack(kv.getTrackMomentum(i),
                   kv.getTrackPosition(i),
                   kv.getTrackError(i),
                   kv.getTrack(i).getCharge());
      for (unsigned j = i + 1; j < n; ++j) {
        kmm.setCorrelation(kv.getCorrelation(i, j));
      }
    }

    kmm.setVertex(kv.getVertex());
    kmm.setVertexError(kv.getVertexError());

    int err = kmm.doMake();
    if (err != 0) return false;

    TLorentzVector mom(kmm.getMotherMomentum().px(),
                       kmm.getMotherMomentum().py(),
                       kmm.getMotherMomentum().pz(),
                       kmm.getMotherMomentum().e());

    TVector3 pos(kmm.getMotherPosition().x(),
                 kmm.getMotherPosition().y(),
                 kmm.getMotherPosition().z());

    CLHEP::HepSymMatrix covMatrix = kmm.getMotherError();
    TMatrixFSym errMatrix(7);
    for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 7; j++) {
        errMatrix[i][j] = covMatrix[i][j];
      }
    }

    double chi2 = kv.getCHIsq();
    int ndf = kv.getNDF();
    double prob = TMath::Prob(chi2, ndf);

    mother->updateMomentum(mom, pos, errMatrix, prob);
    return true;

  }

  bool ParticleVertexFitterModule::doRaveFit(Particle* mother)
  {
    if (mother->getNDaughters() < 2) return false;

    analysis::RaveVertexFitter rf;
    rf.addMother(mother);

    int nVert = rf.fit("kalman");
    if (nVert != 1) return false;

    bool ok = makeRaveMother(rf, mother);
    return ok;

  }

  bool ParticleVertexFitterModule::makeRaveMother(analysis::RaveVertexFitter& rf,
                                                  Particle* p)
  {

    TVector3 pos = rf.getPos(0);
    TMatrixDSym RerrMatrix = rf.getCov(0);
    double prob = rf.getPValue(0);
    TLorentzVector mom(p->getMomentum(), p->getEnergy());
    TMatrixDSym errMatrix(7);
    for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 7; j++) {
        if (i > 3 && j > 3) {errMatrix[i][j] = RerrMatrix[i - 4][j - 4];}
        else {errMatrix[i][j] = 0;}
      }
    }

    p->updateMomentum(mom, pos, errMatrix, prob);
    return true;

  }

} // end Belle2 namespace

