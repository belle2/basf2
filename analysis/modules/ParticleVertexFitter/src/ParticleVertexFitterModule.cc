/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Luigi Li Gioi, Anze Zupanc                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleVertexFitter/ParticleVertexFitterModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// Magnetic field
#include <geometry/bfieldmap/BFieldMap.h>

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
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("listName", m_listName, "name of particle list", string(""));
    addParam("confidenceLevel", m_confidenceLevel,
             "required confidence level of fit to keep particles in the list", 0.001);
    addParam("vertexFitter", m_vertexFitter, "kfitter or rave", string("kfitter"));
    addParam("fitType", m_fitType, "type of the kinematic fit", string("vertex"));
    addParam("withConstraint", m_withConstraint, "additional constraint on vertex", string(""));
    addParam("decayString", m_decayString, "specifies which daughter particles are included in the kinematic fit", string(""));
  }

  ParticleVertexFitterModule::~ParticleVertexFitterModule()
  {
  }

  void ParticleVertexFitterModule::initialize()
  {
    m_Bfield = BFieldMap::Instance().getBField(TVector3(0, 0, 0)).Z();
    analysis::RaveSetup::initialize(1, m_Bfield);
    B2INFO("ParticleVertexFitterModule : magnetic field = " << m_Bfield);

    TVector3 beamSpot(0, 0, 0);
    TMatrixDSym beamSpotCov(3);
    beamSpotCov(0, 0) = 1e-03; beamSpotCov(1, 1) = 5.9e-06;
    if (m_withConstraint.compare(std::string("ipprofile")) == 0) beamSpotCov(2, 2) = 1.9e-02; // from TDR
    if (m_withConstraint.compare(std::string("iptube")) == 0) beamSpotCov(2, 2) = 1000;

    if (m_withConstraint.compare(std::string("ipprofile")) == 0 || m_withConstraint.compare(std::string("iptube")) == 0)
      analysis::RaveSetup::getInstance()->setBeamSpot(beamSpot, beamSpotCov);

    if (m_decayString.compare(std::string("")) != 0)
      m_decaydescriptor.init(m_decayString);

    B2INFO("ParticleVertexFitter: Performing " << m_fitType << " fit on " << m_listName << " using " << m_vertexFitter);
    if (m_decayString.compare(std::string("")) != 0)
      B2INFO("ParticleVertexFitter: Using specified decay string: " << m_decayString);
    if (m_withConstraint.compare(std::string("")) != 0)
      B2INFO("ParticleVertexFitter: Additional " << m_withConstraint << " will be applied");

  }

  void ParticleVertexFitterModule::beginRun()
  {
    //TODO: set magnetic field for each run
    //m_Bfield = BFieldMap::Instance().getBField(TVector3(0,0,0)).Z();
  }

  void ParticleVertexFitterModule::event()
  {

    StoreObjPtr<ParticleList> plist(m_listName);
    if (!plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    std::vector<unsigned int> toRemove;
    for (unsigned i = 0; i < plist->getListSize(); i++) {
      Particle* particle = plist->getParticle(i);
      bool ok = doVertexFit(particle);
      if (!ok) toRemove.push_back(particle->getArrayIndex());
    }
    plist->removeParticles(toRemove);
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

    if (m_Bfield == 0) {
      B2ERROR("ParticleVertexFitter: No magnetic field");
      return false;
    }

    if (m_withConstraint.compare(std::string("ipprofile")) != 0 &&
        m_withConstraint.compare(std::string("iptube")) != 0 && m_withConstraint.compare(std::string("")) != 0) {
      B2ERROR("ParticleVertexFitter: " << m_withConstraint << " ***invalid Constraint ");
      return false;
    }


    bool ok = false;
    // fits with KFitter
    if (m_vertexFitter.compare(std::string("kfitter")) == 0) {
      // TODO: add this functionality
      if (m_decayString.compare(std::string("")) != 0)
        B2ERROR("ParticleVertexFitter: kfitter does not support yet selection of daughters via decay string!") ;
      if (m_withConstraint.compare(std::string("iptube")) == 0)
        B2ERROR("ParticleVertexFitter: kfitter does not support yet the iptube constraint ");

      // vertex fit
      if (m_fitType.compare(std::string("vertex")) == 0) {
        if (m_withConstraint.compare(std::string("ipprofile")) == 0) {
          ok = doKVertexFit(mother, true, false);
        } else if (m_withConstraint.compare(std::string("iptube")) == 0) {
          ok = doKVertexFit(mother, false, true);
        } else {
          ok = doKVertexFit(mother, false, false);
        }
      }

      // mass-constrained vertex fit
      if (m_fitType.compare(std::string("massvertex")) == 0) {
        if (m_withConstraint.compare(std::string("ipprofile")) == 0 || m_withConstraint.compare(std::string("iptube")) == 0) {
          B2ERROR("ParticleVertexFitter: Invalid options - mass-constrained fit using kfitter does not work with iptube or ipprofile constraint.");
        } else {
          ok = doKMassVertexFit(mother);
        }
      }

      // mass fit
      if (m_fitType.compare(std::string("mass")) == 0) {
        if (m_withConstraint.compare(std::string("ipprofile")) == 0 || m_withConstraint.compare(std::string("iptube")) == 0) {
          B2ERROR("ParticleVertexFitter: Invalid options - mass fit using kfitter does not work with iptube or ipprofile constraint.");
        } else {
          ok = doKMassFit(mother);
        }
      }

      // invalid KFitter fit type
      if (m_fitType.compare(std::string("vertex")) != 0
          && m_fitType.compare(std::string("massvertex")) != 0
          && m_fitType.compare(std::string("mass")) != 0)
        B2ERROR("ParticleVertexFitter: " << m_fitType << " ***invalid fit type for the Kfitter ");
    }

    // fits using Rave
    if (m_vertexFitter.compare(std::string("rave")) == 0) ok = doRaveFit(mother);

    // invalid fitter
    if (m_vertexFitter.compare(std::string("kfitter")) != 0 && m_vertexFitter.compare(std::string("rave")) != 0)
      B2ERROR("ParticleVertexFitter: " << m_vertexFitter << " ***invalid vertex fitter ");

    if (!ok) return false;

    // steering ends here

    if (mother->getPValue() < m_confidenceLevel) return false;
    return true;

  }


  bool ParticleVertexFitterModule::fillFitParticles(const Particle* mother, std::vector<const Particle*>& fitChildren, std::vector<const Particle*>& pi0Children)
  {
    for (unsigned ichild = 0; ichild < mother->getNDaughters(); ichild++) {
      const Particle* child = mother->getDaughter(ichild);

      if (child->getPValue() < 0)
        return false; // error matrix not valid
      bool isPi0 = false;
      if (child->getPDGCode() == 111 && child->getNDaughters() == 2)
        if (child->getDaughter(0)->getPDGCode() == 22 && child->getDaughter(1)->getPDGCode() == 22)
          isPi0 = true;

      if (!isPi0)
        fitChildren.push_back(child);
      else
        pi0Children.push_back(child);
    }

    return true;
  }

  bool ParticleVertexFitterModule::redoPi0MassFit(Particle* pi0Temp, const Particle* pi0Orig, const analysis::VertexFitKFit kv)
  {
    // TODO: something like setGammaError is necessary
    // this is just workaround for the moment

    const Particle* g1Orig = pi0Orig->getDaughter(0);
    const Particle* g2Orig = pi0Orig->getDaughter(1);
    Particle* g1Temp = new Particle(g1Orig->get4Vector(), 22);
    Particle* g2Temp = new Particle(g2Orig->get4Vector(), 22);

    TMatrixFSym g1ErrMatrix = g1Orig->getMomentumVertexErrorMatrix();
    TMatrixFSym g2ErrMatrix = g2Orig->getMomentumVertexErrorMatrix();

    TVector3 pos(kv.getVertex().x(), kv.getVertex().y(), kv.getVertex().z());
    CLHEP::HepSymMatrix posErrorMatrix = kv.getVertexError();

    TMatrixFSym errMatrix(3);
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        errMatrix[i][j] = posErrorMatrix[i][j];

    g1ErrMatrix.SetSub(4, errMatrix);
    g2ErrMatrix.SetSub(4, errMatrix);

    g1Temp->updateMomentum(g1Orig->get4Vector(), pos, g1ErrMatrix, 1.0);
    g2Temp->updateMomentum(g2Orig->get4Vector(), pos, g2ErrMatrix, 1.0);

    // perform the mass fit for pi0
    analysis::MassFitKFit km;
    km.setMagneticField(m_Bfield);

    addParticleToKfitter(km, g1Temp);
    addParticleToKfitter(km, g2Temp);

    km.setVertex(kv.getVertex());
    km.setVertexError(kv.getVertexError());
    km.setInvariantMass(pi0Orig->getPDGMass());

    int err = km.doFit();
    if (err != 0) {
      delete g1Temp;
      delete g2Temp;

      return false;
    }

    bool ok = makeKMassMother(km, pi0Temp);

    delete g1Temp;
    delete g2Temp;
    return ok;
  }

  bool ParticleVertexFitterModule::doKVertexFit(Particle* mother, bool ipProfileConstraint, bool ipTubeConstraint)
  {
    if (mother->getNDaughters() < 2) return false;

    if (ipTubeConstraint)
      B2ERROR("[ParticleVertexFitterModule::doKVertexFit] ipTubeConstraint is not supported yet!");

    std::vector<const Particle*> fitChildren;
    std::vector<const Particle*> pi0Children;

    bool validChildren = fillFitParticles(mother, fitChildren, pi0Children);

    if (!validChildren)
      return false;

    if (pi0Children.size() > 1)
      B2ERROR("[ParticleVertexFitterModule::doKVertexFit] Vertex fit using KFitter does not support fit with multiple pi0s (yet).");

    if (fitChildren.size() < 2)
      B2ERROR("[ParticleVertexFitterModule::doKVertexFit] Number of particles with valid error matrix entering the vertex fit using KFitter is less than 2.");

    // Initialise the Fitter
    analysis::VertexFitKFit kv;
    kv.setMagneticField(m_Bfield);

    for (unsigned iChild = 0; iChild < fitChildren.size(); iChild++)
      addParticleToKfitter(kv, fitChildren[iChild]);

    if (ipProfileConstraint)
      addIPProfileToKFitter(kv);

    // Perform vertex fit using only the particles with valid error matrices
    int err = kv.doFit();
    if (err != 0)
      return false;

    bool ok = false;
    if (pi0Children.size() == 0)
      // in the case daughters do not inlude pi0 - this is it (fit done)
      ok = makeKVertexMother(kv, mother);
    else if (pi0Children.size() == 1) {
      // the daughters contain pi0:
      // 1. refit pi0 to previously determined vertex
      // 2. redo the fit using all particles (including pi0 this time)

      Particle* pi0Temp = new Particle(pi0Children[0]->get4Vector(), 111);
      ok = redoPi0MassFit(pi0Temp, pi0Children[0], kv) ;
      if (!ok)
        return false;

      // finaly perform the fit using all daughter particles
      analysis::VertexFitKFit kv2;
      kv2.setMagneticField(m_Bfield);

      for (unsigned iChild = 0; iChild < fitChildren.size(); iChild++)
        addParticleToKfitter(kv2, fitChildren[iChild]);
      addParticleToKfitter(kv2, pi0Temp);

      if (ipProfileConstraint)
        addIPProfileToKFitter(kv2);

      err = kv2.doFit();

      if (err != 0)
        return false;

      ok = makeKVertexMother(kv2, mother);
      delete pi0Temp;
    }

    return ok;
  }

  bool ParticleVertexFitterModule::doKMassVertexFit(Particle* mother)
  {
    if (mother->getNDaughters() < 2) return false;

    std::vector<const Particle*> fitChildren;
    std::vector<const Particle*> pi0Children;

    bool validChildren = fillFitParticles(mother, fitChildren, pi0Children);

    if (!validChildren)
      return false;

    if (pi0Children.size() > 1)
      B2ERROR("[ParticleVertexFitterModule::doKVertexFit] MassVertex fit using KFitter does not support fit with multiple pi0s (yet).");

    if (fitChildren.size() < 2)
      B2ERROR("[ParticleVertexFitterModule::doKVertexFit] Number of particles with valid error matrix entering the vertex fit using KFitter is less than 2.");

    bool ok = false;
    if (pi0Children.size() == 0) {
      // Initialise the Fitter
      analysis::MassVertexFitKFit kmv;
      kmv.setMagneticField(m_Bfield);

      for (unsigned iChild = 0; iChild < fitChildren.size(); iChild++)
        addParticleToKfitter(kmv, fitChildren[iChild]);

      kmv.setInvariantMass(mother->getPDGMass());
      int err = kmv.doFit();
      if (err != 0)
        return false;

      // in the case daughters do not inlude pi0 - this is it (fit done)
      ok = makeKMassVertexMother(kmv, mother);
    } else if (pi0Children.size() == 1) {
      // the daughters contain pi0:
      // 1. refit pi0 to previously determined vertex
      // 2. redo the fit using all particles (including pi0 this time)

      analysis::VertexFitKFit kv;
      kv.setMagneticField(m_Bfield);

      for (unsigned iChild = 0; iChild < fitChildren.size(); iChild++)
        addParticleToKfitter(kv, fitChildren[iChild]);

      // Perform vertex fit using only the particles with valid error matrices
      int err = kv.doFit();
      if (err != 0)
        return false;

      Particle* pi0Temp = new Particle(pi0Children[0]->get4Vector(), 111);
      ok = redoPi0MassFit(pi0Temp, pi0Children[0], kv) ;
      if (!ok)
        return false;

      // finaly perform the fit using all daughter particles
      analysis::MassVertexFitKFit kmv2;
      kmv2.setMagneticField(m_Bfield);

      for (unsigned iChild = 0; iChild < fitChildren.size(); iChild++)
        addParticleToKfitter(kmv2, fitChildren[iChild]);
      addParticleToKfitter(kmv2, pi0Temp);

      kmv2.setInvariantMass(mother->getPDGMass());
      err = kmv2.doFit();

      if (err != 0)
        return false;

      ok = makeKMassVertexMother(kmv2, mother);
      delete pi0Temp;
    }

    return ok;

  }

  bool ParticleVertexFitterModule::doKMassFit(Particle* mother)
  {
    if (mother->getNDaughters() < 2) return false;

    analysis::MassFitKFit km;
    km.setMagneticField(m_Bfield);

    for (unsigned ichild = 0; ichild < mother->getNDaughters(); ichild++) {
      const Particle* child = mother->getDaughter(ichild);

      if (child->getPValue() < 0) return false; // error matrix not valid

      addParticleToKfitter(km, child);
    }

    // apply mass constraint
    km.setInvariantMass(mother->getPDGMass());

    int err = km.doFit();

    if (err != 0) return false;

    bool ok = makeKMassMother(km, mother);

    return ok;
  }


  bool ParticleVertexFitterModule::makeKVertexMother(analysis::VertexFitKFit& kv,
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

      kmm.setTrackVertexError(kv.getTrackVertexError(i));

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


  bool ParticleVertexFitterModule::makeKMassVertexMother(analysis::MassVertexFitKFit& kmv,
                                                         Particle* mother)
  {

    analysis::MakeMotherKFit kmm;
    kmm.setMagneticField(m_Bfield);

    unsigned n = kmv.getTrackCount();
    for (unsigned i = 0; i < n; ++i) {
      kmm.addTrack(kmv.getTrackMomentum(i),
                   kmv.getTrackPosition(i),
                   kmv.getTrackError(i),
                   kmv.getTrack(i).getCharge());

      kmm.setTrackVertexError(kmv.getTrackVertexError(i));

      for (unsigned j = i + 1; j < n; ++j) {
        kmm.setCorrelation(kmv.getCorrelation(i, j));
      }
    }

    kmm.setVertex(kmv.getVertex());
    kmm.setVertexError(kmv.getVertexError());

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

    double chi2 = kmv.getCHIsq();
    int ndf = kmv.getNDF();
    double prob = TMath::Prob(chi2, ndf);

    mother->updateMomentum(mom, pos, errMatrix, prob);

    return true;
  }


  bool ParticleVertexFitterModule::makeKMassMother(analysis::MassFitKFit& km,
                                                   Particle* mother)
  {

    analysis::MakeMotherKFit kmm;
    kmm.setMagneticField(m_Bfield);

    unsigned n = km.getTrackCount();
    for (unsigned i = 0; i < n; ++i) {
      kmm.addTrack(km.getTrackMomentum(i),
                   km.getTrackPosition(i),
                   km.getTrackError(i),
                   km.getTrack(i).getCharge());

      if (km.getFlagFitWithVertex()) kmm.setTrackVertexError(km.getTrackVertexError(i));

      for (unsigned j = i + 1; j < n; ++j) {
        kmm.setCorrelation(km.getCorrelation(i, j));
      }
    }

    kmm.setVertex(km.getVertex());
    if (km.getFlagFitWithVertex())
      kmm.setVertexError(km.getVertexError());

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

    double chi2 = km.getCHIsq();
    int ndf = km.getNDF();
    double prob = TMath::Prob(chi2, ndf);

    mother->updateMomentum(mom, pos, errMatrix, prob);

    return true;
  }



  bool ParticleVertexFitterModule::doRaveFit(Particle* mother)
  {
    if (mother->getNDaughters() < 2) return false;

    analysis::RaveKinematicVertexFitter rf;
    if (m_fitType.compare(std::string("mass")) == 0) rf.setVertFit(false);


    if (m_decayString.compare(std::string("")) == 0) {
      rf.addMother(mother);
    } else {
      std::vector<const Particle*> tracksVertex = m_decaydescriptor.getSelectionParticles(mother);
      std::vector<std::string> tracksName = m_decaydescriptor.getSelectionNames();

      if (allSelectedDaughters(mother, tracksVertex)) {
        for (unsigned itrack = 0; itrack < tracksVertex.size(); itrack++) {
          if (tracksVertex[itrack] != mother) rf.addTrack(tracksVertex[itrack]);
        }
        rf.setMother(mother);
      } else {

        analysis::RaveKinematicVertexFitter rsf;
        bool mothSel = false;
        int nTrk = 0;
        for (unsigned itrack = 0; itrack < tracksVertex.size(); itrack++) {
          if (tracksVertex[itrack] != mother) {
            rsf.addTrack(tracksVertex[itrack]);
            B2INFO("ParticleVertexFitterModule: Adding particle " << tracksName[itrack] << " to vertex fit ");
            nTrk++;
          }
          if (tracksVertex[itrack] == mother) mothSel = true;
        }

        TVector3 pos;
        TMatrixDSym RerrMatrix(3);
        int nvert = 0;

        if (nTrk == 1) {
          analysis::RaveVertexFitter rsg;
          for (unsigned itrack = 0; itrack < tracksVertex.size(); itrack++) {
            rsg.addTrack(tracksVertex[itrack]);
            nvert = rsg.fit("kalman");
            if (nvert > 0) {
              pos = rsg.getPos(0);
              RerrMatrix = rsg.getCov(0);
              double prob = rsg.getPValue(0);
              TLorentzVector mom(mother->getMomentum(), mother->getEnergy());
              TMatrixDSym errMatrix(7);
              for (int i = 0; i < 7; i++) {
                for (int j = 0; j < 7; j++) {
                  if (i > 3 && j > 3) {errMatrix[i][j] = RerrMatrix[i - 4][j - 4];}
                  else {errMatrix[i][j] = 0;}
                }
              }
              mother->updateMomentum(mom, pos, errMatrix, prob);
              return true;
            } else {return false;}
          }
        } else {
          nvert = rsf.fit();
        }

        if (nvert > 0) {
          pos = rsf.getPos();
          RerrMatrix = rsf.getVertexErrorMatrix();
          double prob = rsf.getPValue();
          TLorentzVector mom(mother->getMomentum(), mother->getEnergy());
          TMatrixDSym errMatrix(7);
          for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 7; j++) {
              if (i > 3 && j > 3) {errMatrix[i][j] = RerrMatrix[i - 4][j - 4];}
              else {errMatrix[i][j] = 0;}
            }
          }
          mother->updateMomentum(mom, pos, errMatrix, prob);
        } else {return false;}


        if (mothSel && nTrk > 1) {
          analysis::RaveSetup::getInstance()->setBeamSpot(pos, RerrMatrix);
          rf.addMother(mother);
          int nKfit = rf.fit();
          rf.updateMother();
          analysis::RaveSetup::getInstance()->unsetBeamSpot();

          if (nKfit > 0) {return true;}
          else return false;
        } else return true;
      }

    }


    int nVert = 0;
    bool okFT = false;
    if (m_fitType.compare(std::string("vertex")) == 0) {
      okFT = true;
      nVert = rf.fit();
      rf.updateMother();
      if (nVert != 1) return false;
    }
    if (m_fitType.compare(std::string("mass")) == 0) {
      // add protection
      okFT = true;
      rf.setMassConstFit(true);
      rf.setVertFit(false);
      nVert = rf.fit();
      rf.updateMother();
      if (nVert != 1) return false;
    };
    if (m_fitType.compare(std::string("massvertex")) == 0) {
      okFT = true;
      rf.setMassConstFit(true);
      nVert = rf.fit();
      rf.updateMother();
      if (nVert != 1) return false;
    };
    if (!okFT) {
      B2ERROR("fitType : " << m_fitType << " ***invalid fit type ");
      return false;
    }


    return true;
  }


  bool ParticleVertexFitterModule::allSelectedDaughters(const Particle* mother,
                                                        std::vector<const Particle*> tracksVertex)
  {

    bool isAll = false;
    if (mother->getNDaughters() == 0) return false;

    int nNotIncluded = mother->getNDaughters();

    for (unsigned i = 0; i < mother->getNDaughters(); i++) {
      bool dauOk = false;
      for (unsigned vi = 0; vi < tracksVertex.size(); vi++) {
        if (tracksVertex[vi] == mother->getDaughter(i)) {
          nNotIncluded = nNotIncluded - 1;
          dauOk = true;
        }
      }
      if (!dauOk) {
        if (allSelectedDaughters(mother->getDaughter(i), tracksVertex)) nNotIncluded--;
      }
    }
    if (nNotIncluded == 0) isAll = true;
    return isAll;

  }

  void ParticleVertexFitterModule::addParticleToKfitter(analysis::VertexFitKFit& kv, const Particle* particle)
  {
    CLHEP::HepLorentzVector mom = getCLHEPLorentzVector(particle);
    HepPoint3D              pos = getCLHEPPoint3D(particle);
    CLHEP::HepSymMatrix     err = getCLHEPSymMatrix(particle);

    kv.addTrack(mom, pos, err, particle->getCharge());
  }

  void ParticleVertexFitterModule::addParticleToKfitter(analysis::MassVertexFitKFit& kmv, const Particle* particle)
  {
    CLHEP::HepLorentzVector mom = getCLHEPLorentzVector(particle);
    HepPoint3D              pos = getCLHEPPoint3D(particle);
    CLHEP::HepSymMatrix     err = getCLHEPSymMatrix(particle);

    kmv.addTrack(mom, pos, err, particle->getCharge());
  }

  void ParticleVertexFitterModule::addParticleToKfitter(analysis::MassFitKFit& km, const Particle* particle)
  {
    CLHEP::HepLorentzVector mom = getCLHEPLorentzVector(particle);
    HepPoint3D              pos = getCLHEPPoint3D(particle);
    CLHEP::HepSymMatrix     err = getCLHEPSymMatrix(particle);

    km.addTrack(mom, pos, err, particle->getCharge());
  }

  CLHEP::HepLorentzVector ParticleVertexFitterModule::getCLHEPLorentzVector(const Particle* particle)
  {
    CLHEP::HepLorentzVector mom(particle->getPx(),
                                particle->getPy(),
                                particle->getPz(),
                                particle->getEnergy());

    return mom;
  }

  HepPoint3D ParticleVertexFitterModule::getCLHEPPoint3D(const Particle* particle)
  {
    HepPoint3D pos(particle->getX(), particle->getY(), particle->getZ());

    return pos;
  }

  CLHEP::HepSymMatrix ParticleVertexFitterModule::getCLHEPSymMatrix(const Particle* particle)
  {
    CLHEP::HepSymMatrix covMatrix(7);
    TMatrixFSym errMatrix = particle->getMomentumVertexErrorMatrix();

    for (int i = 0; i < 7; i++) {
      for (int j = i; j < 7; j++) {
        covMatrix[i][j] = errMatrix[i][j];
      }
    }

    return covMatrix;
  }

  void ParticleVertexFitterModule::addIPProfileToKFitter(analysis::VertexFitKFit& kv)
  {
    //TODO: fix this once IPProfile is available from somewhere
    HepPoint3D pos(0.0, 0.0, 0.0);
    CLHEP::HepSymMatrix covMatrix(3, 0);

    covMatrix[0][0] = 1e-03;
    covMatrix[1][1] = 5.9e-06;
    covMatrix[2][2] = 1.9e-02;

    kv.setIpProfile(pos, covMatrix);
  }

} // end Belle2 namespace

