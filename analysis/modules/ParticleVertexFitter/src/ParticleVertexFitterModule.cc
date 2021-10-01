/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleVertexFitter/ParticleVertexFitterModule.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/particledb/EvtGenDatabasePDG.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/Btube.h>
// utilities
#include <analysis/utility/CLHEPToROOT.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>
#include <analysis/utility/ROOTToCLHEP.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

#include <TVector.h>
#include <TRotation.h>
#include <framework/geometry/B2Vector3.h>


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
             "Confidence level to accept the fit. Particle candidates with "
             "p-value less than confidenceLevel are removed from the particle "
             "list. If set to -1, all candidates are kept; if set to 0, "
             "the candidates failing the fit are removed.",
             0.001);
    addParam("vertexFitter", m_vertexFitter, "KFit or Rave", string("KFit"));
    addParam("fitType", m_fitType, "type of the kinematic fit (vertex, massvertex, mass)", string("vertex"));
    addParam("withConstraint", m_withConstraint,
             "additional constraint on vertex: ipprofile, iptube, mother, iptubecut, pointing, btube",
             string(""));
    addParam("decayString", m_decayString, "specifies which daughter particles are included in the kinematic fit", string(""));
    addParam("updateDaughters", m_updateDaughters, "true: update the daughters after the vertex fit", false);
    addParam("smearing", m_smearing, "smear IP tube width by given length", 0.002);
    addParam("massConstraintList", m_massConstraintList,
             "Type::[int]. List of daughter particles to mass constrain with int = pdg code. (only for MassFourCKFit)", {});
    addParam("massConstraintListParticlename", m_massConstraintListParticlename,
             "Type::[string]. List of daughter particles to mass constrain with string = particle name. (only for MassFourCKFit)", {});
  }

  void ParticleVertexFitterModule::initialize()
  {
    // Particle list with name m_listName has to exist
    m_plist.isRequired(m_listName);

    // magnetic field
    m_Bfield = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Unit::T;

    // RAVE setup
    if (m_vertexFitter == "Rave")
      analysis::RaveSetup::initialize(1, m_Bfield);

    B2DEBUG(1, "ParticleVertexFitterModule : magnetic field = " << m_Bfield);


    if (m_decayString != "")
      m_decaydescriptor.init(m_decayString);

    if ((m_massConstraintList.size()) == 0 && (m_massConstraintListParticlename.size()) > 0) {
      for (auto& containedParticle : m_massConstraintListParticlename) {
        TParticlePDG* particletemp = TDatabasePDG::Instance()->GetParticle((containedParticle).c_str());
        m_massConstraintList.push_back(particletemp->PdgCode());
      }
    }

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
    if (m_vertexFitter == "Rave")
      analysis::RaveSetup::initialize(1, m_Bfield);

    m_BeamSpotCenter = m_beamSpotDB->getIPPosition();
    m_beamSpotCov.ResizeTo(3, 3);
    TMatrixDSym beamSpotCov(3);
    if (m_withConstraint == "ipprofile") m_beamSpotCov = m_beamSpotDB->getCovVertex();
    if (m_withConstraint == "iptube") {
      if (m_smearing > 0 && m_vertexFitter == "KFit") {
        ParticleVertexFitterModule::smearBeamSpot(m_smearing);
      } else {
        ParticleVertexFitterModule::findConstraintBoost(2.);
      }
    }
    if (m_withConstraint == "iptubecut") {  // for development purpose only
      m_BeamSpotCenter = TVector3(0.001, 0., .013);
      findConstraintBoost(0.03);
    }
    if ((m_vertexFitter == "Rave") && (m_withConstraint == "ipprofile" || m_withConstraint == "iptube"
                                       || m_withConstraint == "mother" || m_withConstraint == "iptubecut" || m_withConstraint == "btube"))
      analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_beamSpotCov);
    std::vector<unsigned int> toRemove;
    unsigned int n = m_plist->getListSize();
    for (unsigned i = 0; i < n; i++) {
      Particle* particle = m_plist->getParticle(i);
      m_hasCovMatrix = false;
      if (m_updateDaughters == true) {
        if (m_decayString.empty()) ParticleCopy::copyDaughters(particle);
        else B2ERROR("Daughters update works only when all daughters are selected. Daughters will not be updated");
      }

      if (m_withConstraint == "mother") {
        m_BeamSpotCenter = particle->getVertex();
        m_beamSpotCov = particle->getVertexErrorMatrix();
      }

      TMatrixFSym mother_errMatrix(7);
      mother_errMatrix = particle->getMomentumVertexErrorMatrix();
      for (int k = 0; k < 7; k++) {
        for (int j = 0; j < 7; j++) {
          if (mother_errMatrix[k][j] > 0) {
            m_hasCovMatrix = true;
          }
        }
      }
      bool hasTube = true;
      if (m_withConstraint == "btube") {
        Btube* Ver = particle->getRelatedTo<Btube>();
        if (!Ver) {
          hasTube = false;
          toRemove.push_back(particle->getArrayIndex());
        } else {
          m_BeamSpotCenter.SetXYZ(Ver->getTubeCenter()(0, 0), Ver->getTubeCenter()(1, 0), Ver->getTubeCenter()(2, 0));
          m_beamSpotCov = Ver->getTubeMatrix();
        }
      }
      bool ok = false;
      if (hasTube) {
        ok = doVertexFit(particle);
      }
      if (!ok)
        particle->setPValue(-1);
      if (particle->getPValue() < m_confidenceLevel)
        toRemove.push_back(particle->getArrayIndex());
    }
    m_plist->removeParticles(toRemove);

    //free memory allocated by rave. initialize() would be enough, except that we must clean things up before program end...
    if (m_vertexFitter == "Rave")
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
        m_withConstraint != "pointing" &&
        m_withConstraint != "btube" &&
        m_withConstraint != "")
      B2FATAL("ParticleVertexFitter: " << m_withConstraint << " ***invalid Constraint ");

    bool ok = false;
    // fits with KFit
    if (m_vertexFitter == "KFit") {

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
          B2FATAL("ParticleVertexFitter: Invalid options - mass-constrained fit using KFit does not work with iptube or ipprofile constraint.");
        } else if (m_withConstraint == "pointing") {
          ok = doKMassPointingVertexFit(mother);
        } else {
          ok = doKMassVertexFit(mother);
        }
      }

      // mass fit
      if (m_fitType == "mass") {
        if (m_withConstraint == "ipprofile" || m_withConstraint == "iptube" || m_withConstraint == "iptubecut") {
          B2FATAL("ParticleVertexFitter: Invalid options - mass fit using KFit does not work with iptube or ipprofile constraint.");
        } else {
          ok = doKMassFit(mother);
        }
      }

      // four C fit
      if (m_fitType == "fourC") {
        if (m_withConstraint == "ipprofile" || m_withConstraint == "iptube" || m_withConstraint == "iptubecut") {
          B2FATAL("ParticleVertexFitter: Invalid options - four C fit using KFit does not work with iptube or ipprofile constraint.");
        } else {
          ok = doKFourCFit(mother);
        }
      }

      // four mass C fit
      if (m_fitType == "massfourC") {
        if (m_withConstraint == "ipprofile" || m_withConstraint == "iptube" || m_withConstraint == "iptubecut") {
          B2FATAL("ParticleVertexFitter: Invalid options - four C fit using KFit does not work with iptube or ipprofile constraint.");
        } else {
          ok = doKMassFourCFit(mother);
        }
      }

      // invalid KFit fit type
      if (m_fitType != "vertex"
          && m_fitType != "massvertex"
          && m_fitType != "mass"
          && m_fitType != "fourC"
          && m_fitType != "massfourC")
        B2FATAL("ParticleVertexFitter: " << m_fitType << " ***invalid fit type for the vertex fitter ");
    }

    // fits using Rave
    if (m_vertexFitter == "Rave") {
      try {
        ok = doRaveFit(mother);
      } catch (const rave::CheckedFloatException&) {
        B2ERROR("Invalid inputs (nan/inf)?");
        ok = false;
      }
    }

    // invalid fitter
    if (m_vertexFitter != "KFit" && m_vertexFitter != "Rave")
      B2FATAL("ParticleVertexFitter: " << m_vertexFitter << " ***invalid vertex fitter ");

    if (!ok) return false;

    // steering ends here

    //if (mother->getPValue() < m_confidenceLevel) return false;
    return true;

  }

  bool ParticleVertexFitterModule::fillFitParticles(const Particle* mother, std::vector<const Particle*>& fitChildren,
                                                    std::vector<const Particle*>& twoPhotonChildren)
  {
    if (m_decayString.empty()) {
      // if decayString is empty, just use all primary daughters
      for (unsigned ichild = 0; ichild < mother->getNDaughters(); ichild++) {
        const Particle* child = mother->getDaughter(ichild);
        // This if allows to skip the daughters, which cannot be used in the fits, particularly K_L0 from KLM.
        // Useful for fully-inclusive particles.
        if (mother->getProperty() == Particle::PropertyFlags::c_IsUnspecified and child->getPValue() < 0) {
          continue;
        }
        fitChildren.push_back(child);
      }
    } else {
      fitChildren = m_decaydescriptor.getSelectionParticles(mother);
    }

    auto itr = fitChildren.begin();
    while (itr != fitChildren.end()) {
      const Particle* child = *itr;

      if (child->getPValue() < 0) {
        B2WARNING("Daughter with PDG code " << child->getPDGCode() << " does not have a valid error matrix.");
        return false; // error matrix not valid
      }
      bool isTwoPhotonParticle = false;
      if (m_hasCovMatrix == false) {
        if (child->getPDGCode() == Const::pi0.getPDGCode() or child->getPDGCode() == 221) { // pi0 or eta
          if (child->getNDaughters() == 2) {
            if (child->getDaughter(0)->getPDGCode() == Const::photon.getPDGCode()
                && child->getDaughter(1)->getPDGCode() == Const::photon.getPDGCode()) {
              isTwoPhotonParticle = true;
            }
          }
        }
      }
      if (isTwoPhotonParticle) {
        // move children from fitChildren to twoPhotonChildren
        twoPhotonChildren.push_back(child);
        itr = fitChildren.erase(itr);
      } else {
        itr++;
      }
    }

    return true;
  }

  bool ParticleVertexFitterModule::redoTwoPhotonDaughterMassFit(Particle* postFit, const Particle* preFit,
      const analysis::VertexFitKFit& kv)
  {
    // TODO: something like setGammaError is necessary
    // this is just workaround for the moment

    const Particle* g1Orig = preFit->getDaughter(0);
    const Particle* g2Orig = preFit->getDaughter(1);
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

    // perform the mass fit for the two-photon particle
    analysis::MassFitKFit km;
    km.setMagneticField(m_Bfield);

    km.addParticle(&g1Temp);
    km.addParticle(&g2Temp);

    km.setVertex(kv.getVertex());
    km.setVertexError(kv.getVertexError());
    km.setInvariantMass(preFit->getPDGMass());

    int err = km.doFit();
    if (err != 0) {
      return false;
    }

    // The update of the daughters is disabled for this mass fit.
    bool updateDaughters = m_updateDaughters;
    m_updateDaughters = false;
    bool ok = makeKMassMother(km, postFit);
    m_updateDaughters = updateDaughters;

    return ok;
  }

  bool ParticleVertexFitterModule::doKVertexFit(Particle* mother, bool ipProfileConstraint, bool ipTubeConstraint)
  {
    if ((mother->getNDaughters() < 2 && !ipTubeConstraint) || mother->getNDaughters() < 1) return false;

    std::vector<const Particle*> fitChildren;
    std::vector<const Particle*> twoPhotonChildren;
    bool validChildren = fillFitParticles(mother, fitChildren, twoPhotonChildren);

    if (!validChildren)
      return false;

    if (twoPhotonChildren.size() > 1) {
      B2FATAL("[ParticleVertexFitterModule::doKVertexFit] Vertex fit using KFit does not support fit with multiple particles decaying to two photons like pi0 (yet).");
    }

    if ((fitChildren.size() < 2 && !ipTubeConstraint) || fitChildren.size() < 1) {
      B2WARNING("[ParticleVertexFitterModule::doKVertexFit] Number of particles with valid error matrix entering the vertex fit using KFit is too low.");
      return false;
    }

    // Initialise the Fitter
    analysis::VertexFitKFit kv;
    kv.setMagneticField(m_Bfield);

    for (auto& child : fitChildren)
      kv.addParticle(child);

    if (ipProfileConstraint)
      addIPProfileToKFit(kv);

    if (ipTubeConstraint)
      addIPTubeToKFit(kv);

    // Perform vertex fit using only the particles with valid error matrices
    int err = kv.doFit();
    if (err != 0)
      return false;

    double chi2_track = getChi2TracksLBoost(kv);
    unsigned track_count = kv.getTrackCount();
    bool haschi2_track = mother->hasExtraInfo("chiSquared_trackL");
    if (haschi2_track) {
      mother->writeExtraInfo("chiSquared_trackL", chi2_track);
      mother->writeExtraInfo("n_track", track_count);
    }

    bool ok = false;
    if (twoPhotonChildren.size() == 0)
      // in the case daughters do not include pi0 - this is it (fit done)
      ok = makeKVertexMother(kv, mother);
    else if (twoPhotonChildren.size() == 1) {
      // there is a daughter reconstructed from two photons so without position information
      // 1. determine vertex based on all other valid daughters
      // 2. set position and error matrix of two-photon daughter to previously determined vertex
      // 3. redo the fit using all particles (including two-photon particle this time)

      const Particle* twoPhotonDaughter = twoPhotonChildren[0];
      Particle fixedTwoPhotonDaughter(twoPhotonDaughter->get4Vector(), twoPhotonDaughter->getPDGCode());
      ok = redoTwoPhotonDaughterMassFit(&fixedTwoPhotonDaughter, twoPhotonDaughter, kv);
      if (!ok)
        return false;

      // finally perform the fit using all daughter particles
      analysis::VertexFitKFit kv2;
      kv2.setMagneticField(m_Bfield);

      for (auto& child : fitChildren)
        kv2.addParticle(child);

      kv2.addParticle(&fixedTwoPhotonDaughter);

      if (ipProfileConstraint)
        addIPProfileToKFit(kv2);

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

    std::vector<const Particle*> fitChildren;
    std::vector<const Particle*> twoPhotonChildren;
    bool validChildren = fillFitParticles(mother, fitChildren, twoPhotonChildren);

    if (!validChildren)
      return false;

    if (twoPhotonChildren.size() > 1) {
      B2FATAL("[ParticleVertexFitterModule::doKVertexFit] MassVertex fit using KFit does not support fit with multiple particles decaying to two photons like pi0 (yet).");
    }

    if (fitChildren.size() < 2) {
      B2WARNING("[ParticleVertexFitterModule::doKVertexFit] Number of particles with valid error matrix entering the vertex fit using KFit is less than 2.");
      return false;
    }

    bool ok = false;
    if (twoPhotonChildren.size() == 0) {
      // Initialise the Fitter
      analysis::MassVertexFitKFit kmv;
      kmv.setMagneticField(m_Bfield);

      for (auto child : fitChildren)
        kmv.addParticle(child);

      kmv.setInvariantMass(mother->getPDGMass());
      int err = kmv.doFit();
      if (err != 0)
        return false;

      // in the case daughters do not include particles with two photon daughters like pi0 - this is it (fit done)
      ok = makeKMassVertexMother(kmv, mother);
    } else if (twoPhotonChildren.size() == 1) {
      // there is a daughter reconstructed from two photons so without position information
      // 1. determine vertex based on all other valid daughters
      // 2. set position and error matrix of two-photon daughter to previously determined vertex
      // 3. redo the fit using all particles (including two-photon particle this time)

      analysis::VertexFitKFit kv;
      kv.setMagneticField(m_Bfield);

      for (auto child : fitChildren)
        kv.addParticle(child);

      // Perform vertex fit using only the particles with valid error matrices
      int err = kv.doFit();
      if (err != 0)
        return false;

      const Particle* twoPhotonDaughter = twoPhotonChildren[0];
      Particle fixedTwoPhotonDaughter(twoPhotonDaughter->get4Vector(), twoPhotonDaughter->getPDGCode());
      ok = redoTwoPhotonDaughterMassFit(&fixedTwoPhotonDaughter, twoPhotonDaughter, kv);
      if (!ok)
        return false;

      // finally perform the fit using all daughter particles
      analysis::MassVertexFitKFit kmv2;
      kmv2.setMagneticField(m_Bfield);

      for (auto child : fitChildren)
        kmv2.addParticle(child);
      kmv2.addParticle(&fixedTwoPhotonDaughter);

      kmv2.setInvariantMass(mother->getPDGMass());
      err = kmv2.doFit();

      if (err != 0)
        return false;

      ok = makeKMassVertexMother(kmv2, mother);
    }

    return ok;

  }

  bool ParticleVertexFitterModule::doKMassPointingVertexFit(Particle* mother)
  {
    if (!(mother->hasExtraInfo("prodVertX") && mother->hasExtraInfo("prodVertY") && mother->hasExtraInfo("prodVertZ"))) {
      return false;
    }

    if (mother->getNDaughters() < 2) return false;

    std::vector<const Particle*> fitChildren;
    std::vector<const Particle*> twoPhotonChildren;
    bool validChildren = fillFitParticles(mother, fitChildren, twoPhotonChildren);

    if (!validChildren)
      return false;

    if (twoPhotonChildren.size() > 0) {
      B2FATAL("[ParticleVertexFitterModule::doKMassPointingVertexFit] MassPointingVertex fit using KFit does not support fit with two-photon daughters (yet).");
    }

    if (fitChildren.size() < 2) {
      B2WARNING("[ParticleVertexFitterModule::doKMassPointingVertexFit] Number of particles with valid error matrix entering the vertex fit using KFit is less than 2.");
      return false;
    }

    bool ok = false;
    // Initialise the Fitter
    analysis::MassPointingVertexFitKFit kmpv;
    kmpv.setMagneticField(m_Bfield);

    for (auto child : fitChildren)
      kmpv.addParticle(child);

    kmpv.setInvariantMass(mother->getPDGMass());
    HepPoint3D productionVertex(mother->getExtraInfo("prodVertX"),
                                mother->getExtraInfo("prodVertY"),
                                mother->getExtraInfo("prodVertZ"));
    kmpv.setProductionVertex(productionVertex);
    int err = kmpv.doFit();
    if (err != 0) return false;

    ok = makeKMassPointingVertexMother(kmpv, mother);

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
        bool err = addChildofParticletoKFit(kf, child);
        if (!err) return false;
      } else {
        if (child->getPValue() < 0) return false; // error matrix not valid

        kf.addParticle(child);
      }
    }

    // apply four momentum constraint
    PCmsLabTransform T;
    kf.setFourMomentum(T.getBeamFourMomentum());

    int err = kf.doFit();

    if (err != 0) return false;

    bool ok = makeKFourCMother(kf, mother);

    return ok;
  }

  bool ParticleVertexFitterModule::doKMassFourCFit(Particle* mother)
  {
    if (mother->getNDaughters() < 2) return false;

    analysis::MassFourCFitKFit kf;
    kf.setMagneticField(m_Bfield);

    for (unsigned ichild = 0; ichild < mother->getNDaughters(); ichild++) {
      const Particle* child = mother->getDaughter(ichild);

      if (child->getNDaughters() > 0) {
        bool massconstraint = std::find(m_massConstraintList.begin(), m_massConstraintList.end(),
                                        std::abs(child->getPDGCode())) != m_massConstraintList.end();
        std::vector<unsigned> childId;
        bool err = addChildofParticletoMassKFit(kf, child, childId);
        if (massconstraint) kf.addMassConstraint(child->getPDGMass(), childId);
        if (!err) return false;
      } else {
        if (child->getPValue() < 0) return false; // error matrix not valid
        kf.addParticle(child);
      }
    }

    // apply four momentum constraint
    PCmsLabTransform T;
    kf.setFourMomentum(T.getBeamFourMomentum());

    int err = kf.doFit();

    if (err != 0) return false;

    bool ok = makeMassKFourCMother(kf, mother);

    return ok;
  }

  bool ParticleVertexFitterModule::makeKVertexMother(analysis::VertexFitKFit& kv,
                                                     Particle* mother)
  {
    enum analysis::KFitError::ECode fitError;
    fitError = kv.updateMother(mother);
    if (fitError != analysis::KFitError::kNoError)
      return false;
    if (m_decayString.empty() && m_updateDaughters == true) {
      // update daughter momenta as well
      // the order of daughters in the *fitter is the same as in the mother Particle

      std::vector<Particle*> daughters = mother->getDaughters();

      unsigned track_count = kv.getTrackCount();
      if (daughters.size() != track_count)
        return false;

      for (unsigned iChild = 0; iChild < track_count; iChild++) {
        daughters[iChild]->set4Vector(
          CLHEPToROOT::getTLorentzVector(kv.getTrackMomentum(iChild)));
        daughters[iChild]->setVertex(
          CLHEPToROOT::getTVector3(kv.getTrackPosition(iChild)));
        daughters[iChild]->setMomentumVertexErrorMatrix(
          CLHEPToROOT::getTMatrixFSym(kv.getTrackError(iChild)));
      }
    }

    return true;
  }

  bool ParticleVertexFitterModule::makeKMassVertexMother(analysis::MassVertexFitKFit& kmv,
                                                         Particle* mother)
  {
    enum analysis::KFitError::ECode fitError;
    fitError = kmv.updateMother(mother);
    if (fitError != analysis::KFitError::kNoError)
      return false;
    if (m_decayString.empty() && m_updateDaughters == true) {
      // update daughter momenta as well
      // the order of daughters in the *fitter is the same as in the mother Particle

      std::vector<Particle*> daughters = mother->getDaughters();

      unsigned track_count = kmv.getTrackCount();
      if (daughters.size() != track_count)
        return false;

      for (unsigned iChild = 0; iChild < track_count; iChild++) {
        daughters[iChild]->set4Vector(
          CLHEPToROOT::getTLorentzVector(kmv.getTrackMomentum(iChild)));
        daughters[iChild]->setVertex(
          CLHEPToROOT::getTVector3(kmv.getTrackPosition(iChild)));
        daughters[iChild]->setMomentumVertexErrorMatrix(
          CLHEPToROOT::getTMatrixFSym(kmv.getTrackError(iChild)));
      }
    }

    return true;
  }

  bool ParticleVertexFitterModule::makeKMassPointingVertexMother(analysis::MassPointingVertexFitKFit& kmpv,
      Particle* mother)
  {
    enum analysis::KFitError::ECode fitError;
    fitError = kmpv.updateMother(mother);
    if (fitError != analysis::KFitError::kNoError) {
      return false;
    }

    if (m_decayString.empty() && m_updateDaughters == true) {
      // update daughter momenta as well
      // the order of daughters in the *fitter is the same as in the mother Particle

      std::vector<Particle*> daughters = mother->getDaughters();

      unsigned track_count = kmpv.getTrackCount();
      if (daughters.size() != track_count)
        return false;

      for (unsigned iChild = 0; iChild < track_count; iChild++) {
        daughters[iChild]->set4Vector(
          CLHEPToROOT::getTLorentzVector(kmpv.getTrackMomentum(iChild)));
        daughters[iChild]->setVertex(
          CLHEPToROOT::getTVector3(kmpv.getTrackPosition(iChild)));
        daughters[iChild]->setMomentumVertexErrorMatrix(
          CLHEPToROOT::getTMatrixFSym(kmpv.getTrackError(iChild)));
      }
    }

    return true;
  }


  bool ParticleVertexFitterModule::makeKMassMother(analysis::MassFitKFit& km,
                                                   Particle* mother)
  {
    enum analysis::KFitError::ECode fitError;
    fitError = km.updateMother(mother);
    if (fitError != analysis::KFitError::kNoError)
      return false;
    if (m_decayString.empty() && m_updateDaughters == true) {
      // update daughter momenta as well
      // the order of daughters in the *fitter is the same as in the mother Particle

      std::vector<Particle*> daughters = mother->getDaughters();

      unsigned track_count = km.getTrackCount();
      if (daughters.size() != track_count)
        return false;

      for (unsigned iChild = 0; iChild < track_count; iChild++) {
        daughters[iChild]->set4Vector(
          CLHEPToROOT::getTLorentzVector(km.getTrackMomentum(iChild)));
        daughters[iChild]->setVertex(
          CLHEPToROOT::getTVector3(km.getTrackPosition(iChild)));
        daughters[iChild]->setMomentumVertexErrorMatrix(
          CLHEPToROOT::getTMatrixFSym(km.getTrackError(iChild)));
      }
    }

    return true;
  }



  bool ParticleVertexFitterModule::makeKFourCMother(analysis::FourCFitKFit& kf, Particle* mother)
  {
    enum analysis::KFitError::ECode fitError;
    fitError = kf.updateMother(mother);
    if (fitError != analysis::KFitError::kNoError)
      return false;
    mother->addExtraInfo("FourCFitProb", kf.getCHIsq());
    mother->addExtraInfo("FourCFitChi2", kf.getNDF());
    if (m_decayString.empty() && m_updateDaughters == true) {
      // update daughter momenta as well
      // the order of daughters in the *fitter is the same as in the mother Particle

      std::vector<Particle*> daughters = mother->getDaughters();

      const unsigned nd = daughters.size();
      unsigned l = 0;
      std::vector<std::vector<unsigned>> pars;
      std::vector<Particle*> allparticles;
      for (unsigned ichild = 0; ichild < nd; ichild++) {
        const Particle* daughter = mother->getDaughter(ichild);
        std::vector<unsigned> pard;
        if (daughter->getNDaughters() > 0) {
          updateMapOfTrackAndDaughter(l, pars, pard, allparticles, daughter);
          pars.push_back(pard);
          allparticles.push_back(daughters[ichild]);
        } else {
          pard.push_back(l);
          pars.push_back(pard);
          allparticles.push_back(daughters[ichild]);
          l++;
        }
      }

      unsigned track_count = kf.getTrackCount();
      if (l != track_count)
        return false;

      for (unsigned iDaug = 0; iDaug < allparticles.size(); iDaug++) {
        TLorentzVector childMoms;
        TVector3 childPoss;
        TMatrixFSym childErrMatrixs(7);
        for (unsigned int iChild : pars[iDaug]) {
          childMoms = childMoms +
                      CLHEPToROOT::getTLorentzVector(
                        kf.getTrackMomentum(iChild));
          childPoss = childPoss +
                      CLHEPToROOT::getTVector3(
                        kf.getTrackPosition(iChild));
          TMatrixFSym childErrMatrix =
            CLHEPToROOT::getTMatrixFSym(kf.getTrackError(iChild));
          childErrMatrixs = childErrMatrixs + childErrMatrix;
        }
        allparticles[iDaug]->set4Vector(childMoms);
        allparticles[iDaug]->setVertex(childPoss);
        allparticles[iDaug]->setMomentumVertexErrorMatrix(childErrMatrixs);
      }
    }

    return true;
  }

  bool ParticleVertexFitterModule::makeMassKFourCMother(analysis::MassFourCFitKFit& kf, Particle* mother)
  {
    enum analysis::KFitError::ECode fitError;
    fitError = kf.updateMother(mother);
    if (fitError != analysis::KFitError::kNoError)
      return false;
    mother->addExtraInfo("MassFourCFitProb", TMath::Prob(kf.getCHIsq(), kf.getNDF()));
    mother->addExtraInfo("MassFourCFitChi2", kf.getCHIsq());
    mother->addExtraInfo("MassFourCFitNDF", kf.getNDF());
    if (m_decayString.empty() && m_updateDaughters == true) {
      // update daughter momenta as well
      // the order of daughters in the *fitter is the same as in the mother Particle

      std::vector<Particle*> daughters = mother->getDaughters();

      const unsigned nd = daughters.size();
      unsigned l = 0;
      std::vector<std::vector<unsigned>> pars;
      std::vector<Particle*> allparticles;
      for (unsigned ichild = 0; ichild < nd; ichild++) {
        const Particle* daughter = mother->getDaughter(ichild);
        std::vector<unsigned> pard;
        if (daughter->getNDaughters() > 0) {
          updateMapOfTrackAndDaughter(l, pars, pard, allparticles, daughter);
          pars.push_back(pard);
          allparticles.push_back(daughters[ichild]);
        } else {
          pard.push_back(l);
          pars.push_back(pard);
          allparticles.push_back(daughters[ichild]);
          l++;
        }
      }

      unsigned track_count = kf.getTrackCount();
      if (l != track_count)
        return false;

      for (unsigned iDaug = 0; iDaug < allparticles.size(); iDaug++) {
        TLorentzVector childMoms;
        TVector3 childPoss;
        TMatrixFSym childErrMatrixs(7);
        for (unsigned int iChild : pars[iDaug]) {
          childMoms = childMoms +
                      CLHEPToROOT::getTLorentzVector(
                        kf.getTrackMomentum(iChild));
          childPoss = childPoss +
                      CLHEPToROOT::getTVector3(
                        kf.getTrackPosition(iChild));
          TMatrixFSym childErrMatrix =
            CLHEPToROOT::getTMatrixFSym(kf.getTrackError(iChild));
          childErrMatrixs = childErrMatrixs + childErrMatrix;
        }
        allparticles[iDaug]->set4Vector(childMoms);
        allparticles[iDaug]->setVertex(childPoss);
        allparticles[iDaug]->setMomentumVertexErrorMatrix(childErrMatrixs);
      }
    }

    return true;
  }


  void ParticleVertexFitterModule::updateMapOfTrackAndDaughter(unsigned& l,  std::vector<std::vector<unsigned>>& pars,
      std::vector<unsigned>& parm, std::vector<Particle*>&  allparticles, const Particle* daughter)
  {
    std::vector <Belle2::Particle*> childs = daughter->getDaughters();
    for (unsigned ichild = 0; ichild < daughter->getNDaughters(); ichild++) {
      const Particle* child = daughter->getDaughter(ichild);
      std::vector<unsigned> pard;
      if (child->getNDaughters() > 0) {
        updateMapOfTrackAndDaughter(l, pars, pard, allparticles, child);
        parm.insert(parm.end(), pard.begin(), pard.end());
        pars.push_back(pard);
        allparticles.push_back(childs[ichild]);
      } else  {
        pard.push_back(l);
        parm.push_back(l);
        pars.push_back(pard);
        allparticles.push_back(childs[ichild]);
        l++;
      }
    }
  }


  bool ParticleVertexFitterModule::doRaveFit(Particle* mother)
  {
    if ((m_decayString.empty() ||
         (m_withConstraint == "" && m_fitType != "mass")) && mother->getNDaughters() < 2) return false;
    if (m_withConstraint == "") analysis::RaveSetup::getInstance()->unsetBeamSpot();
    if (m_withConstraint == "ipprofile" || m_withConstraint == "iptube"  || m_withConstraint == "mother"
        || m_withConstraint == "iptubecut" || m_withConstraint == "btube")
      analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_beamSpotCov);

    analysis::RaveKinematicVertexFitter rf;
    if (m_fitType == "mass") rf.setVertFit(false);

    if (m_decayString.empty()) {
      rf.addMother(mother);
    } else {
      std::vector<const Particle*> tracksVertex = m_decaydescriptor.getSelectionParticles(mother);
      std::vector<std::string> tracksName = m_decaydescriptor.getSelectionNames();

      if (allSelectedDaughters(mother, tracksVertex)) {
        for (auto& itrack : tracksVertex) {
          if (itrack != mother) rf.addTrack(itrack);
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
          for (auto& itrack : tracksVertex) {
            rsg.addTrack(itrack);
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
                mother->writeExtraInfo("prodVertX", pos.X());
                mother->writeExtraInfo("prodVertY", pos.Y());
                mother->writeExtraInfo("prodVertZ", pos.Z());
                mother->writeExtraInfo("prodVertSxx", RerrMatrix[0][0]);
                mother->writeExtraInfo("prodVertSxy", RerrMatrix[0][1]);
                mother->writeExtraInfo("prodVertSxz", RerrMatrix[0][2]);
                mother->writeExtraInfo("prodVertSyx", RerrMatrix[1][0]);
                mother->writeExtraInfo("prodVertSyy", RerrMatrix[1][1]);
                mother->writeExtraInfo("prodVertSyz", RerrMatrix[1][2]);
                mother->writeExtraInfo("prodVertSzx", RerrMatrix[2][0]);
                mother->writeExtraInfo("prodVertSzy", RerrMatrix[2][1]);
                mother->writeExtraInfo("prodVertSzz", RerrMatrix[2][2]);
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

    bool okFT = false;
    if (m_fitType == "vertex") {
      okFT = true;
      int nVert = rf.fit();
      rf.updateMother();
      if (m_decayString.empty() && m_updateDaughters == true) rf.updateDaughters();
      if (nVert != 1) return false;
    }
    if (m_fitType == "mass") {
      // add protection
      okFT = true;
      rf.setMassConstFit(true);
      rf.setVertFit(false);
      int nVert = rf.fit();
      rf.updateMother();
      if (nVert != 1) return false;
    };
    if (m_fitType == "massvertex") {
      okFT = true;
      rf.setMassConstFit(true);
      int nVert = rf.fit();
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
                                                        const std::vector<const Particle*>& tracksVertex)
  {
    bool isAll = false;
    if (mother->getNDaughters() == 0) return false;

    int nNotIncluded = mother->getNDaughters();

    for (unsigned i = 0; i < mother->getNDaughters(); i++) {
      bool dauOk = false;
      for (auto& vi : tracksVertex) {
        if (vi == mother->getDaughter(i)) {
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

  bool ParticleVertexFitterModule::addChildofParticletoKFit(analysis::FourCFitKFit& kf, const Particle* particle)
  {
    for (unsigned ichild = 0; ichild < particle->getNDaughters(); ichild++) {
      const Particle* child = particle->getDaughter(ichild);
      if (child->getNDaughters() > 0) addChildofParticletoKFit(kf, child);
      else {
        if (child->getPValue() < 0) return false; // error matrix not valid

        kf.addParticle(child);
      }
    }
    return true;
  }

  bool ParticleVertexFitterModule::addChildofParticletoMassKFit(analysis::MassFourCFitKFit& kf, const Particle* particle,
      std::vector<unsigned>& particleId)
  {
    for (unsigned ichild = 0; ichild < particle->getNDaughters(); ichild++) {
      const Particle* child = particle->getDaughter(ichild);
      if (child->getNDaughters() > 0) {
        bool massconstraint = std::find(m_massConstraintList.begin(), m_massConstraintList.end(),
                                        std::abs(child->getPDGCode())) != m_massConstraintList.end();
        std::vector<unsigned> childId;
        addChildofParticletoMassKFit(kf, child, childId);
        if (massconstraint) kf.addMassConstraint(child->getPDGMass(), childId);
        particleId.insert(particleId.end(), childId.begin(), childId.end());
      } else {
        if (child->getPValue() < 0) return false; // error matrix not valid
        kf.addParticle(child);
        particleId.push_back(kf.getTrackCount() - 1);
      }
    }
    return true;
  }

  void ParticleVertexFitterModule::addIPProfileToKFit(analysis::VertexFitKFit& kv)
  {
    HepPoint3D pos(0.0, 0.0, 0.0);
    CLHEP::HepSymMatrix covMatrix(3, 0);

    for (int i = 0; i < 3; i++) {
      pos[i] = m_BeamSpotCenter(i);
      for (int j = 0; j < 3; j++) {
        covMatrix[i][j] = m_beamSpotCov(i, j);
      }
    }

    kv.setIpProfile(pos, covMatrix);
  }

  void ParticleVertexFitterModule::addIPTubeToKFit(analysis::VertexFitKFit& kv)
  {
    CLHEP::HepSymMatrix err(7, 0);

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        err[i + 4][j + 4] = m_beamSpotCov(i, j);
      }
    }

    PCmsLabTransform T;
    TLorentzVector iptube_mom = T.getBeamFourMomentum();

    kv.setIpTubeProfile(
      ROOTToCLHEP::getHepLorentzVector(iptube_mom),
      ROOTToCLHEP::getPoint3D(m_BeamSpotCenter),
      err,
      0.);
  }

  void ParticleVertexFitterModule::findConstraintBoost(double cut)
  {
    PCmsLabTransform T;

    TVector3 boost = T.getBoostVector();
    TVector3 boostDir = boost.Unit();

    TMatrixDSym beamSpotCov = m_beamSpotDB->getCovVertex();
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

  void ParticleVertexFitterModule::smearBeamSpot(double width)
  {
    TMatrixDSym beamSpotCov = m_beamSpotDB->getCovVertex();
    for (int i = 0; i < 3; i++)
      beamSpotCov(i, i) += width * width;

    m_beamSpotCov = beamSpotCov;
  }

  double ParticleVertexFitterModule::getChi2TracksLBoost(analysis::VertexFitKFit& kv)
  {
    double chi2TrackL = 0;

    for (int iTrack = 0; iTrack < kv.getTrackCount(); iTrack++) {

      analysis::KFitTrack trk_i = kv.getTrack(iTrack); // KFitTrack contains parameters before/after fit.

      TMatrixFSym err = CLHEPToROOT::getTMatrixFSym(trk_i.getError(analysis::KFitConst::kBeforeFit)); // px, py, pz, E, x, y, z

      B2Vector3D x_before = CLHEPToROOT::getTVector3(trk_i.getPosition(analysis::KFitConst::kBeforeFit));
      B2Vector3D x_after = CLHEPToROOT::getTVector3(trk_i.getPosition());
      B2Vector3D dPos = x_after - x_before;

      PCmsLabTransform T;
      B2Vector3D boost3 = T.getBoostVector().Unit();
      TVectorD boostD(0, 6, 0., 0., 0., 0., boost3.X(), boost3.Y(), boost3.Z(), "END");

      double dLBoost = dPos.Dot(boost3);

      chi2TrackL += TMath::Power(dLBoost, 2) / err.Similarity(boostD);
    }
    return chi2TrackL;
  }

} // end Belle2 namespace
