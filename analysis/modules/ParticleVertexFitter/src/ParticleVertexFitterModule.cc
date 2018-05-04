/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Luigi Li Gioi, Anze Zupanc Yu Hu           *
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
#include <framework/dbobjects/BeamParameters.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// utilities
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

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
             "required confidence level of fit to keep particles in the list. Note that even with confidenceLevel == 0.0, errors during the fit might discard Particles in the list. confidenceLevel = -1 if an error occurs during the fit",
             0.001);
    addParam("vertexFitter", m_vertexFitter, "kfitter or rave", string("kfitter"));
    addParam("fitType", m_fitType, "type of the kinematic fit (vertex, massvertex, mass)", string("vertex"));
    addParam("withConstraint", m_withConstraint, "additional constraint on vertex: ipprofile, iptube, mother, iptubecut", string(""));
    addParam("decayString", m_decayString, "specifies which daughter particles are included in the kinematic fit", string(""));
    addParam("updateDaughters", m_updateDaughters, "true: update the daughters after the vertex fit", false);
  }

  void ParticleVertexFitterModule::initialize()
  {

    //m_beamParams.required("", DataStore::c_Persistent);

    // magnetic field
    m_Bfield = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Unit::T;

    // RAVE setup
    if (m_vertexFitter == "rave")
      analysis::RaveSetup::initialize(1, m_Bfield);

    B2DEBUG(1, "ParticleVertexFitterModule : magnetic field = " << m_Bfield);


    if (m_decayString != "")
      m_decaydescriptor.init(m_decayString);

    B2INFO("ParticleVertexFitter: Performing " << m_fitType << " fit on " << m_listName << " using " << m_vertexFitter);
    if (m_decayString != "")
      B2INFO("ParticleVertexFitter: Using specified decay string: " << m_decayString);
    if (m_withConstraint != "")
      B2INFO("ParticleVertexFitter: Additional " << m_withConstraint << " will be applied");

  }

  void ParticleVertexFitterModule::beginRun()
  {
    //TODO: set magnetic field for each run
    //m_Bfield = BFieldMap::Instance().getBField(TVector3(0,0,0)).Z();
    //TODO: set IP spot size for each run
  }

  void ParticleVertexFitterModule::event()
  {

    StoreObjPtr<ParticleList> plist(m_listName);
    if (!plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    if (m_vertexFitter == "rave")
      analysis::RaveSetup::initialize(1, m_Bfield);

    m_BeamSpotCenter = m_beamParams->getVertex();
    m_beamSpotCov.ResizeTo(3, 3);
    TMatrixDSym beamSpotCov(3);
    if (m_withConstraint == "ipprofile") m_beamSpotCov = m_beamParams->getCovVertex();
    if (m_withConstraint == "iptube") ParticleVertexFitterModule::findConstraintBoost(2.);
    if (m_withConstraint == "iptubecut") {  // for development pourpose only
      m_BeamSpotCenter = TVector3(0.001, 0., .013);
      findConstraintBoost(0.03);
    }
    if ((m_vertexFitter == "rave") && (m_withConstraint == "ipprofile" || m_withConstraint == "iptube"
                                       || m_withConstraint == "mother" || m_withConstraint == "iptubecut"))
      analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_beamSpotCov);


    std::vector<unsigned int> toRemove;
    unsigned int n = plist->getListSize();
    for (unsigned i = 0; i < n; i++) {
      Particle* particle = plist->getParticle(i);

      if (m_updateDaughters == true) {
        if (m_decayString.empty()) ParticleCopy::copyDaughters(particle);
        else B2ERROR("Daughters update works only when all daughters are selected. Daughters will not be updated");
      }

      if (m_withConstraint == "mother") {
        m_BeamSpotCenter = particle->getVertex();
        m_beamSpotCov = particle->getVertexErrorMatrix();
      }


      bool ok = doVertexFit(particle);
      if (!ok) particle->setPValue(-1);
      if (m_confidenceLevel == 0. && particle->getPValue() == 0.) {
        toRemove.push_back(particle->getArrayIndex());
      } else {
        if (particle->getPValue() < m_confidenceLevel)toRemove.push_back(particle->getArrayIndex());
      }
    }
    plist->removeParticles(toRemove);

    //free memory allocated by rave. initialize() would be enough, except that we must clean things up before program end...
    if (m_vertexFitter == "rave")
      analysis::RaveSetup::getInstance()->reset();
  }

  bool ParticleVertexFitterModule::doVertexFit(Particle* mother)
  {
    // steering starts here

    if (m_Bfield == 0) {
      B2FATAL("ParticleVertexFitter: No magnetic field");
    }

    if (m_withConstraint != "ipprofile" &&
        m_withConstraint != "iptube" &&
        m_withConstraint != "mother" &&
        m_withConstraint != "iptubecut" &&
        m_withConstraint != "")
      B2FATAL("ParticleVertexFitter: " << m_withConstraint << " ***invalid Constraint ");

    bool ok = false;
    // fits with KFitter
    if (m_vertexFitter == "kfitter") {
      // TODO: add this functionality
      if (m_decayString != "")
        B2FATAL("ParticleVertexFitter: kfitter does not support yet selection of daughters via decay string!");
      if (m_withConstraint == "iptube" || m_withConstraint == "iptubecut")
        B2FATAL("ParticleVertexFitter: kfitter does not support yet the iptube constraint ");

      // vertex fit
      if (m_fitType == "vertex") {
        if (m_withConstraint == "ipprofile") {
          ok = doKVertexFit(mother, true, false);
        } else if (m_withConstraint == "iptube") {
          ok = doKVertexFit(mother, false, true);
        } else {
          ok = doKVertexFit(mother, false, false);
        }
      }

      // mass-constrained vertex fit
      if (m_fitType == "massvertex") {
        if (m_withConstraint == "ipprofile" || m_withConstraint == "iptube" || m_withConstraint == "iptubecut") {
          B2FATAL("ParticleVertexFitter: Invalid options - mass-constrained fit using kfitter does not work with iptube or ipprofile constraint.");
        } else {
          ok = doKMassVertexFit(mother);
        }
      }

      // mass fit
      if (m_fitType == "mass") {
        if (m_withConstraint == "ipprofile" || m_withConstraint == "iptube" || m_withConstraint == "iptubecut") {
          B2FATAL("ParticleVertexFitter: Invalid options - mass fit using kfitter does not work with iptube or ipprofile constraint.");
        } else {
          ok = doKMassFit(mother);
        }
      }

      // four C fit
      if (m_fitType == "fourC") {
        if (m_withConstraint == "ipprofile" || m_withConstraint == "iptube" || m_withConstraint == "iptubecut") {
          B2FATAL("ParticleVertexFitter: Invalid options - four C fit using kfitter does not work with iptube or ipprofile constraint.");
        } else {
          ok = doKFourCFit(mother);
        }
      }

      // invalid KFitter fit type
      if (m_fitType != "vertex"
          && m_fitType != "massvertex"
          && m_fitType != "mass"
          && m_fitType != "fourC")
        B2FATAL("ParticleVertexFitter: " << m_fitType << " ***invalid fit type for the vertex fitter ");
    }

    // fits using Rave
    if (m_vertexFitter == "rave") {
      try {
        ok = doRaveFit(mother);
      } catch (rave::CheckedFloatException) {
        B2ERROR("Invalid inputs (nan/inf)?");
        ok = false;
      }
    }

    // invalid fitter
    if (m_vertexFitter != "kfitter" && m_vertexFitter != "rave")
      B2FATAL("ParticleVertexFitter: " << m_vertexFitter << " ***invalid vertex fitter ");

    if (!ok) return false;

    // steering ends here

    //if (mother->getPValue() < m_confidenceLevel) return false;
    return true;

  }


  bool ParticleVertexFitterModule::fillFitParticles(const Particle* mother, std::vector<unsigned>& fitChildren,
                                                    std::vector<unsigned>& pi0Children)
  {
    for (unsigned ichild = 0; ichild < mother->getNDaughters(); ichild++) {
      const Particle* child = mother->getDaughter(ichild);

      if (child->getPValue() < 0) {
        B2WARNING("Daughter with PDG code " << child->getPDGCode() << " does not have a valid error matrix.");
        return false; // error matrix not valid
      }
      bool isPi0 = false;
      if (child->getPDGCode() == 111 && child->getNDaughters() == 2)
        if (child->getDaughter(0)->getPDGCode() == 22 && child->getDaughter(1)->getPDGCode() == 22)
          isPi0 = true;

      if (!isPi0)
        fitChildren.push_back(ichild);
      else
        pi0Children.push_back(ichild);
    }

    return true;
  }

  bool ParticleVertexFitterModule::redoPi0MassFit(Particle* pi0Temp, const Particle* pi0Orig, const analysis::VertexFitKFit& kv)
  {
    // TODO: something like setGammaError is necessary
    // this is just workaround for the moment

    const Particle* g1Orig = pi0Orig->getDaughter(0);
    const Particle* g2Orig = pi0Orig->getDaughter(1);
    Particle g1Temp(g1Orig->get4Vector(), 22);
    Particle g2Temp(g2Orig->get4Vector(), 22);

    TMatrixFSym g1ErrMatrix = g1Orig->getMomentumVertexErrorMatrix();
    TMatrixFSym g2ErrMatrix = g2Orig->getMomentumVertexErrorMatrix();

    TVector3 pos(kv.getVertex().x(), kv.getVertex().y(), kv.getVertex().z());
    CLHEP::HepSymMatrix posErrorMatrix = kv.getVertexError();

    TMatrixFSym errMatrix(3);
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        errMatrix(i, j) = posErrorMatrix[i][j];

    g1ErrMatrix.SetSub(4, errMatrix);
    g2ErrMatrix.SetSub(4, errMatrix);

    g1Temp.updateMomentum(g1Orig->get4Vector(), pos, g1ErrMatrix, 1.0);
    g2Temp.updateMomentum(g2Orig->get4Vector(), pos, g2ErrMatrix, 1.0);

    // perform the mass fit for pi0
    analysis::MassFitKFit km;
    km.setMagneticField(m_Bfield);

    km.addParticle(&g1Temp);
    km.addParticle(&g2Temp);

    km.setVertex(kv.getVertex());
    km.setVertexError(kv.getVertexError());
    km.setInvariantMass(pi0Orig->getPDGMass());

    int err = km.doFit();
    if (err != 0) {
      return false;
    }

    bool ok = makeKMassMother(km, pi0Temp);

    return ok;
  }

  bool ParticleVertexFitterModule::doKVertexFit(Particle* mother, bool ipProfileConstraint, bool ipTubeConstraint)
  {
    if (mother->getNDaughters() < 2) return false;

    if (ipTubeConstraint)
      B2FATAL("[ParticleVertexFitterModule::doKVertexFit] ipTubeConstraint is not supported yet!");

    std::vector<unsigned> fitChildren;
    std::vector<unsigned> pi0Children;

    bool validChildren = fillFitParticles(mother, fitChildren, pi0Children);

    if (!validChildren)
      return false;

    if (pi0Children.size() > 1) {
      B2FATAL("[ParticleVertexFitterModule::doKVertexFit] Vertex fit using KFitter does not support fit with multiple pi0s (yet).");
    }

    if (fitChildren.size() < 2) {
      B2WARNING("[ParticleVertexFitterModule::doKVertexFit] Number of particles with valid error matrix entering the vertex fit using KFitter is less than 2.");
      return false;
    }

    // Initialise the Fitter
    analysis::VertexFitKFit kv;
    kv.setMagneticField(m_Bfield);

    for (unsigned iChild = 0; iChild < fitChildren.size(); iChild++)
      kv.addParticle(mother->getDaughter(fitChildren[iChild]));

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

      const Particle* pi0 = mother->getDaughter(pi0Children[0]);
      Particle pi0Temp(pi0->get4Vector(), 111);
      ok = redoPi0MassFit(&pi0Temp, pi0, kv) ;
      if (!ok)
        return false;

      // finaly perform the fit using all daughter particles
      analysis::VertexFitKFit kv2;
      kv2.setMagneticField(m_Bfield);

      for (unsigned iChild = 0; iChild < mother->getNDaughters(); iChild++) {
        const Particle* child = mother->getDaughter(iChild);
        bool isPi0 = false;
        if (child->getPDGCode() == 111 && child->getNDaughters() == 2)
          if (child->getDaughter(0)->getPDGCode() == 22 && child->getDaughter(1)->getPDGCode() == 22)
            isPi0 = true;

        if (!isPi0)
          kv2.addParticle(child);
        else
          kv2.addParticle(&pi0Temp);
      }

      if (ipProfileConstraint)
        addIPProfileToKFitter(kv2);

      err = kv2.doFit();

      if (err != 0)
        return false;

      ok = makeKVertexMother(kv2, mother);
    }

    return ok;
  }

  bool ParticleVertexFitterModule::doKMassVertexFit(Particle* mother)
  {
    if (mother->getNDaughters() < 2) return false;

    std::vector<unsigned> fitChildren;
    std::vector<unsigned> pi0Children;

    bool validChildren = fillFitParticles(mother, fitChildren, pi0Children);

    if (!validChildren)
      return false;

    if (pi0Children.size() > 1) {
      B2FATAL("[ParticleVertexFitterModule::doKVertexFit] MassVertex fit using KFitter does not support fit with multiple pi0s (yet).");
    }

    if (fitChildren.size() < 2) {
      B2WARNING("[ParticleVertexFitterModule::doKVertexFit] Number of particles with valid error matrix entering the vertex fit using KFitter is less than 2.");
      return false;
    }

    bool ok = false;
    if (pi0Children.size() == 0) {
      // Initialise the Fitter
      analysis::MassVertexFitKFit kmv;
      kmv.setMagneticField(m_Bfield);

      for (unsigned iChild = 0; iChild < fitChildren.size(); iChild++)
        kmv.addParticle(mother->getDaughter(fitChildren[iChild]));

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
        kv.addParticle(mother->getDaughter(fitChildren[iChild]));

      // Perform vertex fit using only the particles with valid error matrices
      int err = kv.doFit();
      if (err != 0)
        return false;

      const Particle* pi0 = mother->getDaughter(pi0Children[0]);
      Particle pi0Temp(pi0->get4Vector(), 111);
      ok = redoPi0MassFit(&pi0Temp, pi0, kv) ;
      if (!ok)
        return false;

      // finaly perform the fit using all daughter particles
      analysis::MassVertexFitKFit kmv2;
      kmv2.setMagneticField(m_Bfield);

      for (unsigned iChild = 0; iChild < mother->getNDaughters(); iChild++) {
        const Particle* child = mother->getDaughter(iChild);
        bool isPi0 = false;
        if (child->getPDGCode() == 111 && child->getNDaughters() == 2)
          if (child->getDaughter(0)->getPDGCode() == 22 && child->getDaughter(1)->getPDGCode() == 22)
            isPi0 = true;

        if (!isPi0)
          kmv2.addParticle(child);
        else
          kmv2.addParticle(&pi0Temp);
      }

      kmv2.setInvariantMass(mother->getPDGMass());
      err = kmv2.doFit();

      if (err != 0)
        return false;

      ok = makeKMassVertexMother(kmv2, mother);
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

      km.addParticle(child);
    }

    // apply mass constraint
    km.setInvariantMass(mother->getPDGMass());

    int err = km.doFit();

    if (err != 0) return false;

    bool ok = makeKMassMother(km, mother);

    return ok;
  }

  bool ParticleVertexFitterModule::doKFourCFit(Particle* mother)
  {
    if (mother->getNDaughters() < 2) return false;

    analysis::FourCFitKFit kf;
    kf.setMagneticField(m_Bfield);

    for (unsigned ichild = 0; ichild < mother->getNDaughters(); ichild++) {
      const Particle* child = mother->getDaughter(ichild);

      if (child->getNDaughters() > 0) {
        bool err = addChildofParticletoKfitter(kf, child);
        if (!err) return false;
      } else {
        if (child->getPValue() < 0) return false; // error matrix not valid

        kf.addParticle(child);
      }
    }

    // apply mass constraint
    kf.setFourMomentum(m_beamParams->getHER() + m_beamParams->getLER());

    int err = kf.doFit();

    if (err != 0) return false;

    bool ok = makeKFourCMother(kf, mother);

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

    if (m_decayString.empty() && m_updateDaughters == true) {
      // update daughter momenta as well
      // the order of daughters in the *fitter is the same as in the mother Particle

      std::vector<Particle*> daughters = mother->getDaughters();

      unsigned track_count = kv.getTrackCount();
      if (daughters.size() != track_count)
        return false;

      for (unsigned iChild = 0; iChild < track_count; iChild++) {
        TLorentzVector childMom(kv.getTrackMomentum(iChild).px(),
                                kv.getTrackMomentum(iChild).py(),
                                kv.getTrackMomentum(iChild).pz(),
                                kv.getTrackMomentum(iChild).e());

        TVector3 childPos(kv.getTrackPosition(iChild).x(),
                          kv.getTrackPosition(iChild).y(),
                          kv.getTrackPosition(iChild).z());

        CLHEP::HepSymMatrix childCovMatrix = kv.getTrackError(iChild);

        TMatrixFSym childErrMatrix(7);
        for (int i = 0; i < 7; i++) {
          for (int j = 0; j < 7; j++) {
            childErrMatrix[i][j] = childCovMatrix[i][j];
          }
        }

        daughters[iChild]->set4Vector(childMom);
        daughters[iChild]->setVertex(childPos);
        daughters[iChild]->setMomentumVertexErrorMatrix(childErrMatrix);
      }
    }

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

    if (m_decayString.empty() && m_updateDaughters == true) {
      // update daughter moenta as well
      // the order of daughters in the *fitter is the same as in the mother Particle

      std::vector<Particle*> daughters = mother->getDaughters();

      unsigned track_count = kmv.getTrackCount();
      if (daughters.size() != track_count)
        return false;

      for (unsigned iChild = 0; iChild < track_count; iChild++) {
        TLorentzVector childMom(kmv.getTrackMomentum(iChild).px(),
                                kmv.getTrackMomentum(iChild).py(),
                                kmv.getTrackMomentum(iChild).pz(),
                                kmv.getTrackMomentum(iChild).e());

        TVector3 childPos(kmv.getTrackPosition(iChild).x(),
                          kmv.getTrackPosition(iChild).y(),
                          kmv.getTrackPosition(iChild).z());

        CLHEP::HepSymMatrix childCovMatrix = kmv.getTrackError(iChild);

        TMatrixFSym childErrMatrix(7);
        for (int i = 0; i < 7; i++) {
          for (int j = 0; j < 7; j++) {
            childErrMatrix[i][j] = childCovMatrix[i][j];
          }
        }

        daughters[iChild]->set4Vector(childMom);
        daughters[iChild]->setVertex(childPos);
        daughters[iChild]->setMomentumVertexErrorMatrix(childErrMatrix);
      }
    }

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

    if (m_decayString.empty() && m_updateDaughters == true) {
      // update daughter moenta as well
      // the order of daughters in the *fitter is the same as in the mother Particle

      std::vector<Particle*> daughters = mother->getDaughters();

      unsigned track_count = km.getTrackCount();
      if (daughters.size() != track_count)
        return false;

      for (unsigned iChild = 0; iChild < track_count; iChild++) {
        TLorentzVector childMom(km.getTrackMomentum(iChild).px(),
                                km.getTrackMomentum(iChild).py(),
                                km.getTrackMomentum(iChild).pz(),
                                km.getTrackMomentum(iChild).e());

        TVector3 childPos(km.getTrackPosition(iChild).x(),
                          km.getTrackPosition(iChild).y(),
                          km.getTrackPosition(iChild).z());

        CLHEP::HepSymMatrix childCovMatrix = km.getTrackError(iChild);

        TMatrixFSym childErrMatrix(7);
        for (int i = 0; i < 7; i++) {
          for (int j = 0; j < 7; j++) {
            childErrMatrix[i][j] = childCovMatrix[i][j];
          }
        }

        daughters[iChild]->set4Vector(childMom);
        daughters[iChild]->setVertex(childPos);
        daughters[iChild]->setMomentumVertexErrorMatrix(childErrMatrix);
      }
    }

    return true;
  }



  bool ParticleVertexFitterModule::makeKFourCMother(analysis::FourCFitKFit& kf, Particle* mother)
  {

    analysis::MakeMotherKFit kmm;
    kmm.setMagneticField(m_Bfield);

    unsigned n = kf.getTrackCount();
    for (unsigned i = 0; i < n; ++i) {
      kmm.addTrack(kf.getTrackMomentum(i),
                   kf.getTrackPosition(i),
                   kf.getTrackError(i),
                   kf.getTrack(i).getCharge());

      if (kf.getFlagFitWithVertex()) kmm.setTrackVertexError(kf.getTrackVertexError(i));

      for (unsigned j = i + 1; j < n; ++j) {
        kmm.setCorrelation(kf.getCorrelation(i, j));
      }
    }

    kmm.setVertex(kf.getVertex());
    if (kf.getFlagFitWithVertex())
      kmm.setVertexError(kf.getVertexError());

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

    double chi2 = kf.getCHIsq();
    int ndf = kf.getNDF();
    double prob = TMath::Prob(chi2, ndf);

    mother->updateMomentum(mom, pos, errMatrix, prob);

    if (m_decayString.empty() && m_updateDaughters == true) {
      // update daughter moenta as well
      // the order of daughters in the *fitter is the same as in the mother Particle

      std::vector<Particle*> daughters = mother->getDaughters();

      const unsigned nd = daughters.size();
      unsigned l = 0;
      std::vector<std::vector<unsigned>> u(nd);
      for (unsigned ichild = 0; ichild < nd; ichild++) {
        const Particle* daughter = mother->getDaughter(ichild);
        if (daughter->getNDaughters() > 0) {
          updateMapofTrackandDaughter(u[ichild], l, daughter);
        } else {
          u[ichild].push_back(l);
          l++;
        }
      }

      unsigned track_count = kf.getTrackCount();
      if (l != track_count)
        return false;

      for (unsigned iDaug = 0; iDaug < nd; iDaug++) {
        TLorentzVector childMoms;
        TVector3 childPoss;
        TMatrixFSym childErrMatrixs(7);
        for (unsigned iChild = 0; iChild < u[iDaug].size(); iChild++) {
          TLorentzVector childMom(kf.getTrackMomentum(u[iDaug][iChild]).px(),
                                  kf.getTrackMomentum(u[iDaug][iChild]).py(),
                                  kf.getTrackMomentum(u[iDaug][iChild]).pz(),
                                  kf.getTrackMomentum(u[iDaug][iChild]).e());
          childMoms = childMoms + childMom;

          TVector3 childPos(kf.getTrackPosition(u[iDaug][iChild]).x(),
                            kf.getTrackPosition(u[iDaug][iChild]).y(),
                            kf.getTrackPosition(u[iDaug][iChild]).z());
          childPoss = childPoss + childPos;

          CLHEP::HepSymMatrix childCovMatrix = kf.getTrackError(u[iDaug][iChild]);

          TMatrixFSym childErrMatrix(7);
          for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 7; j++) {
              childErrMatrix[i][j] = childCovMatrix[i][j];
            }
          }
          childErrMatrixs = childErrMatrixs + childErrMatrix;
        }
        daughters[iDaug]->set4Vector(childMoms);
        daughters[iDaug]->setVertex(childPoss);
        daughters[iDaug]->setMomentumVertexErrorMatrix(childErrMatrixs);
      }
    }

    return true;
  }

  void ParticleVertexFitterModule::updateMapofTrackandDaughter(std::vector<unsigned>& ui, unsigned& l, const Particle* daughter)
  {
    for (unsigned ichild = 0; ichild < daughter->getNDaughters(); ichild++) {
      const Particle* child = daughter->getDaughter(ichild);
      if (child->getNDaughters() > 0) updateMapofTrackandDaughter(ui, l, child);
      else  ui.push_back(l);
      l++;
    }
  }


  bool ParticleVertexFitterModule::doRaveFit(Particle* mother)
  {
    if ((m_decayString.empty() ||
         (m_withConstraint == "" && m_fitType != "mass")) && mother->getNDaughters() < 2) return false;

    if (m_withConstraint == "") analysis::RaveSetup::getInstance()->unsetBeamSpot();
    if (m_withConstraint == "ipprofile" || m_withConstraint == "iptube"  || m_withConstraint == "mother"
        || m_withConstraint == "iptubecut")
      analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_beamSpotCov);

    analysis::RaveKinematicVertexFitter rf;
    if (m_fitType == "mass") rf.setVertFit(false);

    if (m_decayString.empty()) {
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
            B2DEBUG(1, "ParticleVertexFitterModule: Adding particle " << tracksName[itrack] << " to vertex fit ");
            nTrk++;
          }
          if (tracksVertex[itrack] == mother) mothSel = true;
        }


        // Fit one particle constrained to originate from the beam spot
        bool mothIPfit = false;
        if (tracksVertex.size() == 1 && mothSel == true && m_withConstraint != "" && nTrk == 0) {
          rsf.addTrack(tracksVertex[0]);
          if (tracksVertex[0] != mother)
            B2FATAL("ParticleVertexFitterModule: FATAL Error in IP constrained mother fit");
          nTrk++;
          mothIPfit = true;
        }


        TVector3 pos;
        TMatrixDSym RerrMatrix(3);
        int nvert = 0;

        // one track fit is not kinematic
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
              if (mothIPfit) {
                mother->addExtraInfo("prodVertX", pos.X());
                mother->addExtraInfo("prodVertY", pos.Y());
                mother->addExtraInfo("prodVertZ", pos.Z());
                mother->addExtraInfo("prodVertSxx", RerrMatrix[0][0]);
                mother->addExtraInfo("prodVertSxy", RerrMatrix[0][1]);
                mother->addExtraInfo("prodVertSxz", RerrMatrix[0][2]);
                mother->addExtraInfo("prodVertSyx", RerrMatrix[1][0]);
                mother->addExtraInfo("prodVertSyy", RerrMatrix[1][1]);
                mother->addExtraInfo("prodVertSyz", RerrMatrix[1][2]);
                mother->addExtraInfo("prodVertSzx", RerrMatrix[2][0]);
                mother->addExtraInfo("prodVertSzy", RerrMatrix[2][1]);
                mother->addExtraInfo("prodVertSzz", RerrMatrix[2][2]);
              } else {
                mother->updateMomentum(mom, pos, errMatrix, prob);
              }
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
    if (m_fitType == "vertex") {
      okFT = true;
      nVert = rf.fit();
      rf.updateMother();
      if (m_decayString.empty() && m_updateDaughters == true) rf.updateDaughters();
      if (nVert != 1) return false;
    }
    if (m_fitType == "mass") {
      // add protection
      okFT = true;
      rf.setMassConstFit(true);
      rf.setVertFit(false);
      nVert = rf.fit();
      rf.updateMother();
      if (nVert != 1) return false;
    };
    if (m_fitType == "massvertex") {
      okFT = true;
      rf.setMassConstFit(true);
      nVert = rf.fit();
      rf.updateMother();
      if (m_decayString.empty() && m_updateDaughters == true) rf.updateDaughters();
      if (nVert != 1) return false;
    };
    if (!okFT) {
      B2FATAL("fitType : " << m_fitType << " ***invalid fit type ");
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

  bool ParticleVertexFitterModule::addChildofParticletoKfitter(analysis::FourCFitKFit& kf, const Particle* particle)
  {
    for (unsigned ichild = 0; ichild < particle->getNDaughters(); ichild++) {
      const Particle* child = particle->getDaughter(ichild);
      if (child->getNDaughters() > 0) addChildofParticletoKfitter(kf, child);
      else {
        if (child->getPValue() < 0) return false; // error matrix not valid

        kf.addParticle(child);
      }
    }
    return true;
  }


  void ParticleVertexFitterModule::addIPProfileToKFitter(analysis::VertexFitKFit& kv)
  {

    HepPoint3D pos(0.0, 0.0, 0.0);
    CLHEP::HepSymMatrix covMatrix(3, 0);

    covMatrix[0][0] = m_beamSpotCov(0, 0);
    covMatrix[0][1] = m_beamSpotCov(0, 1);
    covMatrix[0][2] = m_beamSpotCov(0, 2);
    covMatrix[1][0] = m_beamSpotCov(1, 0);
    covMatrix[1][1] = m_beamSpotCov(1, 1);
    covMatrix[1][2] = m_beamSpotCov(1, 2);
    covMatrix[2][0] = m_beamSpotCov(2, 0);
    covMatrix[2][1] = m_beamSpotCov(2, 1);
    covMatrix[2][2] = m_beamSpotCov(2, 2);

    kv.setIpProfile(pos, covMatrix);
  }


  void ParticleVertexFitterModule::findConstraintBoost(double cut)
  {

    PCmsLabTransform T;

    TVector3 boost = T.getBoostVector().BoostVector();
    TVector3 boostDir = boost.Unit();

    TMatrixDSym beamSpotCov(3);
    beamSpotCov = m_beamParams->getCovVertex();
    beamSpotCov(2, 2) = cut * cut;
    double thetab = boostDir.Theta();
    double phib = boostDir.Phi();

    double stb = TMath::Sin(thetab);
    double ctb = TMath::Cos(thetab);
    double spb = TMath::Sin(phib);
    double cpb = TMath::Cos(phib);


    TMatrix rz(3, 3);  rz(2, 2) = 1;
    rz(0, 0) = cpb; rz(0, 1) = spb;
    rz(1, 0) = -1 * spb; rz(1, 1) = cpb;

    TMatrix ry(3, 3);  ry(1, 1) = 1;
    ry(0, 0) = ctb; ry(0, 2) = -1 * stb;
    ry(2, 0) = stb; ry(2, 2) = ctb;

    TMatrix r(3, 3);  r.Mult(rz, ry);
    TMatrix rt(3, 3); rt.Transpose(r);

    TMatrix TubePart(3, 3);  TubePart.Mult(rt, beamSpotCov);
    TMatrix Tube(3, 3); Tube.Mult(TubePart, r);

    m_beamSpotCov(0, 0) = Tube(0, 0);  m_beamSpotCov(0, 1) = Tube(0, 1);  m_beamSpotCov(0, 2) = Tube(0, 2);
    m_beamSpotCov(1, 0) = Tube(1, 0);  m_beamSpotCov(1, 1) = Tube(1, 1);  m_beamSpotCov(1, 2) = Tube(1, 2);
    m_beamSpotCov(2, 0) = Tube(2, 0);  m_beamSpotCov(2, 1) = Tube(2, 1);  m_beamSpotCov(2, 2) = Tube(2, 2);

  }



} // end Belle2 namespace

