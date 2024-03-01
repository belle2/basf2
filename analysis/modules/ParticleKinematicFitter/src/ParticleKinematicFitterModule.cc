/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <sstream>
#define SSTR( x ) dynamic_cast< std::ostringstream && >(( std::ostringstream() << std::dec << x ) ).str()

// kinfitter
#include <analysis/modules/ParticleKinematicFitter/ParticleKinematicFitterModule.h>
#include <analysis/OrcaKinFit/BaseFitObject.h>
#include <analysis/OrcaKinFit/OPALFitterGSL.h>
#include <analysis/OrcaKinFit/JetFitObject.h>
#include <analysis/OrcaKinFit/NewtonFitterGSL.h>
#include <analysis/OrcaKinFit/NewFitterGSL.h>
#include <analysis/OrcaKinFit/PxPyPzMFitObject.h>

#include <mdst/dataobjects/ECLCluster.h>

// framework utilities
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// analysis dataobjects
#include <analysis/dataobjects/Particle.h>

// analysis utilities
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>
#include <analysis/ClusterUtility/ClusterUtils.h>

#include <cmath>

using namespace CLHEP;
using namespace std;
using namespace Belle2;
using namespace Belle2::OrcaKinFit;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(ParticleKinematicFitter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ParticleKinematicFitterModule::ParticleKinematicFitterModule() : Module(), m_textTracer(nullptr), m_eventextrainfo("",
      DataStore::c_Event)
{
  setDescription("Kinematic fitter for modular analysis");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  addParam("listName", m_listName, "Name of particle list.", string(""));
  addParam("kinematicFitter", m_kinematicFitter, "Available: OrcaKinFit.", string("OrcaKinFit"));
  addParam("orcaFitterEngine", m_orcaFitterEngine, "OrcaKinFit engine: NewFitterGSL, NewtonFitterGSL, OPALFitterGSL.",
           string("OPALFitterGSL"));
  addParam("orcaTracer", m_orcaTracer, "OrcaKinFit tracer: None, Text.", string("None"));
  addParam("orcaConstraint", m_orcaConstraint, "OrcaKinFit constraint: HardBeam, RecoilMass.", string("HardBeam"));
  addParam("debugFitter", m_debugFitter, "Switch on/off internal debugging output if available.", false);
  addParam("debugFitterLevel", m_debugFitterLevel, "Internal debugging output level if available.", 10);
  addParam("addUnmeasuredPhoton", m_addUnmeasuredPhoton, "Add one unmeasured photon (-3C).", false);
  addParam("fixUnmeasuredToHER", m_fixUnmeasuredPhotonToHER, "fix the momentum of the unmeasured photon to HER (+2C).", false);
  addParam("fixUnmeasuredToLER", m_fixUnmeasuredPhotonToLER, "fix the momentum of the unmeasured photon to LER (+2C).", false);
  addParam("add3CPhoton", m_add3CPhoton, "Add one photon with unmeasured energy (-1C).", false);
  addParam("liftPhotonTheta", m_liftPhotonTheta, "Lift theta constraint of 3CPhoton. Valid when add3CPhoton is true.", false);
  addParam("decayString", m_decayString, "Specifies which daughter particles are included in the kinematic fit.", string(""));
  addParam("updateMother", m_updateMother, "Update the mother kinematics.", true);
  addParam("updateDaughters", m_updateDaughters, "Update the daughter kinematics.", false);
  addParam("recoilMass", m_recoilMass, "Recoil mass in GeV. RecoilMass constraint only.", 0.0);
  addParam("invMass", m_invMass, "Invariant mass in GeV. Mass constraint only.", 0.0);

}

void ParticleKinematicFitterModule::initialize()
{
  m_eventextrainfo.registerInDataStore();

  if (m_decayString != "") {
    m_decaydescriptor.init(m_decayString);
    B2INFO("ParticleKinematicFitter: Using specified decay string: " << m_decayString);
  }

  m_plist.isRequired(m_listName);
}

void ParticleKinematicFitterModule::terminate()
{
  B2INFO("ParticleKinematicFitterModule::terminate");
}

void ParticleKinematicFitterModule::event()
{
  B2DEBUG(17, "ParticleKinematicFitterModule::event");

  unsigned int n = m_plist->getListSize();

  for (unsigned i = 0; i < n; i++) {
    Particle* particle = m_plist->getParticle(i);

    if (m_updateDaughters == true) {
      if (m_decayString.empty()) ParticleCopy::copyDaughters(particle);
      else B2ERROR("Daughters update works only when all daughters are selected. Daughters will not be updated");
    }

    bool ok = doKinematicFit(particle);

    if (!ok) particle->setPValue(-1.);
  }
}

bool ParticleKinematicFitterModule::doKinematicFit(Particle* mother)
{
  B2DEBUG(17, "ParticleKinematicFitterModule::doKinematicFit");

  bool ok = false;

  // fitting with OrcaKinFit
  if (m_kinematicFitter == "OrcaKinFit") {

    // select subset of particles for the fit
    if (m_decayString != "") {
      B2FATAL("ParticleKinematicFitterModule: OrcaKinFit does not support yet selection of daughters via decay string!") ;
    }

    // check requested fit engine
    if (!(m_orcaFitterEngine == "OPALFitterGSL" or
          m_orcaFitterEngine == "NewtonFitterGSL" or
          m_orcaFitterEngine == "NewFitterGSL")) {
      B2FATAL("ParticleKinematicFitterModule:  " << m_orcaFitterEngine << " is an invalid OrcaKinFit fitter engine!");
    }

    // check requested constraint
    if (!(m_orcaConstraint == "HardBeam" or
          m_orcaConstraint == "RecoilMass" or
          m_orcaConstraint == "Mass")) {
      B2FATAL("ParticleKinematicFitterModule:  " << m_orcaConstraint << " is an invalid OrcaKinFit constraint!");
    }

    // basic  check is good, go to fitting routine
    ok = doOrcaKinFitFit(mother);
  } else { // invalid fitter
    B2FATAL("ParticleKinematicFitter: " << m_kinematicFitter << " is an invalid kinematic fitter!");
  }

  if (!ok) return false;

  return true;

}

bool ParticleKinematicFitterModule::doOrcaKinFitFit(Particle* mother)
{
  if (mother->getNDaughters() <= 1) {
    B2WARNING("ParticleKinematicFitterModule: Cannot fit with " << mother->getNDaughters() << " daughters.");
    return false;
  }

  // fill particles
  std::vector<Particle*> particleChildren;
  bool validChildren = fillFitParticles(mother, particleChildren);

  if (!validChildren) {
    B2WARNING("ParticleKinematicFitterModule: Cannot find valid children for the fit.");
    return false;
  }

  // set fit engine
  BaseFitter* pfitter;

  // internal debugger
  int debugfitter = 0;
  if (m_debugFitter) debugfitter = m_debugFitterLevel;

  // choose minimization
  if (m_orcaFitterEngine == "OPALFitterGSL") {
    pfitter = new OPALFitterGSL(); // OPAL fitter has no debugger
  } else if (m_orcaFitterEngine == "NewtonFitterGSL") {
    pfitter = new NewtonFitterGSL();
    (dynamic_cast<NewtonFitterGSL*>(pfitter))->setDebug(debugfitter);
  } else if (m_orcaFitterEngine == "NewFitterGSL") {
    pfitter = new NewFitterGSL();
    (dynamic_cast<NewFitterGSL*>(pfitter))->setDebug(debugfitter);
  } else {
    B2FATAL("ParticleKinematicFitterModule:  " << m_orcaFitterEngine << " is an invalid OrcaKinFit fitter engine!");
    return false;
  }

  if (!pfitter) return false;
  BaseFitter& fitter(*pfitter);

  // reset fitter
  resetFitter(fitter);

  // set constraints (not connected to a fitter or particles at this point!)
  setConstraints();

  // add fit particles from particle list to the fitter and to all constraints
  for (unsigned iChild = 0; iChild < particleChildren.size(); iChild++) {
    addParticleToOrcaKinFit(fitter, particleChildren[iChild], iChild);
  }

  // add unmeasured photon to the fitter and to all constraints
  if (m_addUnmeasuredPhoton) addUnmeasuredGammaToOrcaKinFit(fitter);

  // add constraints to the fitter
  addConstraintsToFitter(fitter);

  // add tracers to the fitter
  addTracerToFitter(fitter);

  //store information before the fit
  storeOrcaKinFitParticles("Measured", fitter, particleChildren, mother);

  double prob   = fitter.fit();
  double chi2   = fitter.getChi2();
  int niter     = fitter.getIterations();
  int ndof      = fitter.getDoF();
  int errorcode = fitter.getError();

  B2DEBUG(17, "ParticleKinematicFitterModule: -------------------------------------------");
  B2DEBUG(17, "ParticleKinematicFitterModule: Fit result of OrcaKinFit using " << m_orcaFitterEngine);
  B2DEBUG(17, "ParticleKinematicFitterModule:   prob              " << prob);
  B2DEBUG(17, "ParticleKinematicFitterModule:   chi2              " << chi2);
  B2DEBUG(17, "ParticleKinematicFitterModule:   iterations        " << niter);
  B2DEBUG(17, "ParticleKinematicFitterModule:   ndf               " << ndof);
  B2DEBUG(17, "ParticleKinematicFitterModule:   errorcode         " << errorcode);
  B2DEBUG(17, "ParticleKinematicFitterModule: -------------------------------------------");

  // default update mother information
  if (m_updateMother) updateOrcaKinFitMother(fitter, particleChildren, mother);

  // update daughter information if that is requested
  if (m_updateDaughters) updateOrcaKinFitDaughters(fitter, mother);

  // store information after the fit
  storeOrcaKinFitParticles("Fitted", fitter, particleChildren, mother);

  //store general fit results
  mother->addExtraInfo("OrcaKinFitProb", prob);
  mother->setPValue(prob);
  mother->addExtraInfo("OrcaKinFitChi2", chi2);
  mother->addExtraInfo("OrcaKinFitErrorCode", errorcode);

  // if we added an unmeasured photon, add the kinematics to the mother - at some point we may want to create a particle list from this?
  std::vector <BaseFitObject*>* fitObjectContainer = fitter.getFitObjects();
  for (auto fo : *fitObjectContainer) {
    if (m_addUnmeasuredPhoton) {
      const std::string name = fo->getName();
      if (name.find("Unmeasured") != std::string::npos) {
        auto* fitobject = static_cast<ParticleFitObject*>(fo);
        ROOT::Math::PxPyPzEVector tlv = getLorentzVector(fitobject);
        mother->addExtraInfo("OrcaKinFit" + name + "Theta", tlv.Theta());
        mother->addExtraInfo("OrcaKinFit" + name + "Phi", tlv.Phi());
        mother->addExtraInfo("OrcaKinFit" + name + "E", tlv.E());

        // Uncertainty
        // const double err0 = getFitObjectError(fitobject, 0);
        mother->addExtraInfo("OrcaKinFit" + name + "ErrorTheta", getFitObjectError(fitobject, 1));
        mother->addExtraInfo("OrcaKinFit" + name + "ErrorPhi", getFitObjectError(fitobject, 2));
        mother->addExtraInfo("OrcaKinFit" + name + "ErrorE", getFitObjectError(fitobject, 0));
      }
    }
    delete fo;
  }

  delete pfitter;
  delete m_textTracer;
  return true;
}

bool ParticleKinematicFitterModule::fillFitParticles(Particle* mother, std::vector<Particle*>& particleChildren)
{
  for (unsigned ichild = 0; ichild < mother->getNDaughters(); ichild++) {
    auto* child = const_cast<Particle*>(mother->getDaughter(ichild));

    if (child->getNDaughters() > 0) {
      bool err = fillFitParticles(child, particleChildren);
      if (!err) {
        B2WARNING("ParticleKinematicFitterModule: Cannot find valid children for the fit.");
        return false;
      }
    } else if (child->getPValue() > 0) {
      particleChildren.push_back(child);
    } else {
      B2ERROR("Daughter with PDG code " << child->getPDGCode() << " does not have a valid p-value: p=" << child->getPValue() << ", E=" <<
              child->getEnergy() << " GeV");
      return false; // error matrix not valid
    }
  }
  return true;
}

bool ParticleKinematicFitterModule::AddFour(Particle* mother)
{
  TMatrixFSym MomentumVertexErrorMatrix(7);
  for (unsigned ichild = 0; ichild < mother->getNDaughters(); ichild++) {
    auto* child = const_cast<Particle*>(mother->getDaughter(ichild));

    if (child->getPValue() > 0) {
      MomentumVertexErrorMatrix += child->getMomentumVertexErrorMatrix();
    } else if (child->getNDaughters() > 0) {
      AddFour(child);
      MomentumVertexErrorMatrix += child->getMomentumVertexErrorMatrix();
    } else {
      B2ERROR("Daughter with PDG code " << child->getPDGCode() << " does not have a valid p-value: p=" << child->getPValue() << ", E=" <<
              child->getEnergy() << " GeV");
      return false; // error matrix not valid
    }
  }
  mother->setMomentumVertexErrorMatrix(MomentumVertexErrorMatrix);
  return true;
}

void ParticleKinematicFitterModule::addParticleToOrcaKinFit(BaseFitter& fitter, Particle* particle, const int index)
{
  B2DEBUG(17, "ParticleKinematicFitterModule: adding a particle to the fitter!");

  if (m_add3CPhoton && index == 0) {
    if (particle -> getPDGCode() != Const::photon.getPDGCode()) {
      B2ERROR("In 3C Kinematic fit, the first daughter should be the Unmeasured Photon!");
    }

    const ECLCluster* cluster = particle->getECLCluster();
    double startingE = cluster->getEnergy(particle->getECLClusterEHypothesisBit());
    double startingPhi = cluster->getPhi();
    double startingTheta = cluster->getTheta();

    ClusterUtils clutls;
    const auto EPhiThetaCov = clutls.GetCovarianceMatrix3x3FromCluster(cluster);
    double startingeE = sqrt(fabs(EPhiThetaCov[0][0]));
    double startingePhi = sqrt(fabs(EPhiThetaCov[1][1]));
    double startingeTheta = sqrt(fabs(EPhiThetaCov[2][2]));

    B2DEBUG(17, startingPhi << " " << startingTheta << " " <<  startingePhi << " " << startingeTheta);
    // create a fit object
    ParticleFitObject* pfitobject;
    // memory allocated: it will be deallocated via "delete fo" in doOrcaKinFitFit
    pfitobject  = new JetFitObject(startingE, startingTheta, startingPhi, startingeE, startingeTheta, startingePhi, 0.);
    pfitobject->setParam(0, startingE, false, false);
    if (m_liftPhotonTheta)
      pfitobject->setParam(1, startingTheta, false, false);
    else
      pfitobject->setParam(1, startingTheta, true, false);
    pfitobject->setParam(2, startingPhi, true, false);

    std::string fitObjectName = "Unmeasured3C";
    pfitobject->setName(fitObjectName.c_str());
    ParticleFitObject& fitobject = *pfitobject;

    // add this fit object (=particle) to the constraints
    addFitObjectToConstraints(fitobject);

    // add fit particle to the fitter
    fitter.addFitObject(fitobject);

  } else {
    // four vector
    CLHEP::HepLorentzVector clheplorentzvector = getCLHEPLorentzVector(particle);

    // error matrix
    CLHEP::HepSymMatrix clhepmomentumerrormatrix = getCLHEPMomentumErrorMatrix(particle);

    // create the fit object (ParticleFitObject is the base class)
    ParticleFitObject* pfitobject;
    // memory allocated: it will be deallocated via "delete fo" in doOrcaKinFitFit
    pfitobject  = new PxPyPzMFitObject(clheplorentzvector, clhepmomentumerrormatrix);
    std::string fitObjectName = "particle_" + SSTR(index);
    pfitobject->setName(fitObjectName.c_str());
    ParticleFitObject& fitobject = *pfitobject;

    // add this fit object (=particle) to the constraints
    addFitObjectToConstraints(fitobject);

    // add fit particle to the fitter
    fitter.addFitObject(fitobject);
  }

  return;
}

CLHEP::HepSymMatrix ParticleKinematicFitterModule::getCLHEPMomentumErrorMatrix(Particle* particle)
{
  CLHEP::HepSymMatrix covMatrix(4);
  TMatrixFSym errMatrix = particle->getMomentumErrorMatrix();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      covMatrix[i][j] = errMatrix[i][j];
    }
  }

  return covMatrix;
}

CLHEP::HepSymMatrix ParticleKinematicFitterModule::getCLHEPMomentumVertexErrorMatrix(Particle* particle)
{
  CLHEP::HepSymMatrix covMatrix(7);
  TMatrixFSym errMatrix = particle->getMomentumVertexErrorMatrix();

  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 7; j++) {
      covMatrix[i][j] = errMatrix[i][j];
    }
  }

  return covMatrix;
}

CLHEP::HepLorentzVector ParticleKinematicFitterModule::getCLHEPLorentzVector(Particle* particle)
{
  CLHEP::HepLorentzVector mom(particle->getPx(), particle->getPy(), particle->getPz(), particle->get4Vector().E());
  return mom;
}

ROOT::Math::PxPyPzEVector ParticleKinematicFitterModule::getLorentzVector(ParticleFitObject* fitobject)
{
  ROOT::Math::PxPyPzEVector mom(fitobject->getPx(), fitobject->getPy(), fitobject->getPz(), fitobject->getE());
  return mom;
}

//  TMatrixFSym ParticleKinematicFitterModule::getTMatrixFSymMomentumErrorMatrix(ParticleFitObject* fitobject)  //fix the warning
TMatrixFSym ParticleKinematicFitterModule::getTMatrixFSymMomentumErrorMatrix()
{
  TMatrixFSym errMatrix(4);

  for (int i = 0; i < 4; i++) {
    for (int j = i; j < 4; j++) {
      errMatrix[i][j] = 0.0;
    }
  }

  return errMatrix;
}

//  TMatrixFSym ParticleKinematicFitterModule::getTMatrixFSymMomentumVertexErrorMatrix(ParticleFitObject* fitobject)  //fix the warning
TMatrixFSym ParticleKinematicFitterModule::getTMatrixFSymMomentumVertexErrorMatrix()
{
  TMatrixFSym errMatrix(7);

  for (int i = 0; i < 7; i++) {
    for (int j = i; j < 7; j++) {
      errMatrix[i][j] = 0.0;
    }
  }

  return errMatrix;
}

ROOT::Math::PxPyPzEVector ParticleKinematicFitterModule::getLorentzVectorConstraints()
{
  if (m_orcaConstraint == "HardBeam") {
    ROOT::Math::PxPyPzEVector constraints4vector(m_hardConstraintPx.getValue(),
                                                 m_hardConstraintPy.getValue(),
                                                 m_hardConstraintPz.getValue(),
                                                 m_hardConstraintE.getValue());
    return constraints4vector;
  } else {
    B2FATAL("ParticleKinematicFitterModule:  " << m_orcaConstraint << " is an invalid OrcaKinFit constraint!");
  }

  // should not reach this point...
  return ROOT::Math::PxPyPzEVector(0., 0., 0., 0.);
}

void ParticleKinematicFitterModule::setConstraints()
{
  if (m_orcaConstraint == "HardBeam") {
    PCmsLabTransform T;
    const ROOT::Math::PxPyPzEVector boost = T.getBeamFourMomentum();

    m_hardConstraintPx = MomentumConstraint(0, 1, 0, 0, boost.Px());
    m_hardConstraintPy = MomentumConstraint(0, 0, 1, 0, boost.Py());
    m_hardConstraintPz = MomentumConstraint(0, 0, 0, 1, boost.Pz());
    m_hardConstraintE  = MomentumConstraint(1, 0, 0, 0, boost.E());

    m_hardConstraintPx.resetFOList();
    m_hardConstraintPy.resetFOList();
    m_hardConstraintPz.resetFOList();
    m_hardConstraintE.resetFOList();

    m_hardConstraintPx.setName("Sum(p_x) [hard]");
    m_hardConstraintPy.setName("Sum(p_y) [hard]");
    m_hardConstraintPz.setName("Sum(p_z) [hard]");
    m_hardConstraintE.setName("Sum(E) [hard]");

  } else if (m_orcaConstraint == "RecoilMass") {
    PCmsLabTransform T;
    const ROOT::Math::PxPyPzEVector boost = T.getBeamFourMomentum();

    m_hardConstraintRecoilMass = RecoilMassConstraint(m_recoilMass, boost.Px(), boost.Py(), boost.Pz(), boost.E());

    m_hardConstraintRecoilMass.resetFOList();
    m_hardConstraintRecoilMass.setName("Recoil Mass [hard]");

  } else if (m_orcaConstraint == "Mass") {
    m_hardConstraintMass = MassConstraint(m_invMass);

    m_hardConstraintMass.resetFOList();
    m_hardConstraintMass.setName("Mass [hard]");
  } else {
    B2FATAL("ParticleKinematicFitterModule:  " << m_orcaConstraint << " is an invalid OrcaKinFit constraint!");
  }
}

void ParticleKinematicFitterModule::resetFitter(BaseFitter& fitter)
{
  B2DEBUG(17, "ParticleKinematicFitterModule: Resetting the fitter");
  fitter.reset();
}

void ParticleKinematicFitterModule::addFitObjectToConstraints(ParticleFitObject& fitobject)
{
  if (m_orcaConstraint == "HardBeam") {
    m_hardConstraintPx.addToFOList(fitobject);
    m_hardConstraintPy.addToFOList(fitobject);
    m_hardConstraintPz.addToFOList(fitobject);
    m_hardConstraintE.addToFOList(fitobject);
  } else if (m_orcaConstraint == "RecoilMass") {
    m_hardConstraintRecoilMass.addToFOList(fitobject);
  } else if (m_orcaConstraint == "Mass") {
    m_hardConstraintMass.addToFOList(fitobject);
  } else {
    B2FATAL("ParticleKinematicFitterModule:  " << m_orcaConstraint << " is an invalid OrcaKinFit constraint!");
  }
}

void ParticleKinematicFitterModule::addConstraintsToFitter(BaseFitter& fitter)
{
  if (m_orcaConstraint == "HardBeam") {
    fitter.addConstraint(m_hardConstraintPx);
    fitter.addConstraint(m_hardConstraintPy);
    fitter.addConstraint(m_hardConstraintPz);
    fitter.addConstraint(m_hardConstraintE);
  } else if (m_orcaConstraint == "RecoilMass") {
    fitter.addConstraint(m_hardConstraintRecoilMass);
  } else if (m_orcaConstraint == "Mass") {
    fitter.addConstraint(m_hardConstraintMass);
  }

  else {
    B2FATAL("ParticleKinematicFitterModule:  " << m_orcaConstraint << " is an invalid OrcaKinFit constraint!");
  }
}

void ParticleKinematicFitterModule::addTracerToFitter(BaseFitter& fitter)
{
  if (m_orcaTracer == "Text") {
    m_textTracer = new TextTracer(std::cout);
    fitter.setTracer(m_textTracer);
  } else if (m_orcaTracer != "None") {
    B2FATAL("ParticleKinematicFitterModule:  " << m_orcaTracer << " is an invalid OrcaKinFit tracer!");
  }
}

void ParticleKinematicFitterModule::addUnmeasuredGammaToOrcaKinFit(BaseFitter& fitter)
{
  B2DEBUG(17, "ParticleKinematicFitterModule::addUnmeasuredGammaToOrcaKinFit: adding an unmeasured photon to the fitter!");
  // Initialize photon using the existing constraints
  ROOT::Math::PxPyPzEVector tlv = getLorentzVectorConstraints();
  double startingE = tlv.E();
  double startingPhi = tlv.Phi();
  double startingTheta = tlv.Theta();
  bool paramFlag = false;

  // create a fit object
  ParticleFitObject* pfitobject;

  std::string fitObjectName = "UnmeasuredAlongBeam";

  if (m_fixUnmeasuredPhotonToHER) {
    startingTheta = 41.5e-3;  // TODO: Read beam crossing angle from db if it's available
    startingPhi = 0.0;
    paramFlag = true;
  } else if (m_fixUnmeasuredPhotonToLER) {
    startingTheta = TMath::Pi() - 41.5e-3;
    startingPhi = 0.0;
    paramFlag = true;
  } else {
    fitObjectName = "Unmeasured";
  }

  // memory allocated: it will be deallocated via "delete fo" in doOrcaKinFitFit
  pfitobject  = new JetFitObject(startingE, startingTheta, startingPhi, 0.0, 0.0, 0.0, 0.);
  pfitobject->setParam(0, startingE, false, false);
  pfitobject->setParam(1, startingTheta, paramFlag, paramFlag);
  pfitobject->setParam(2, startingPhi, paramFlag, paramFlag);

  pfitobject->setName(fitObjectName.c_str());
  ParticleFitObject& fitobject = *pfitobject;

  // add this fit object (=particle) to the constraints
  addFitObjectToConstraints(fitobject);

  // add fit particle to the fitter
  fitter.addFitObject(fitobject);
}

bool ParticleKinematicFitterModule::updateOrcaKinFitDaughters(BaseFitter& fitter, Particle* mother)
{
  std::vector <Belle2::Particle*> bDau = mother->getDaughters();
  std::vector <BaseFitObject*>* fitObjectContainer = fitter.getFitObjects();

  const unsigned nd = bDau.size();
  unsigned l = 0;
  std::vector<std::vector<unsigned>> pars;
  std::vector<Particle*> allparticles;
  for (unsigned ichild = 0; ichild < nd; ichild++) {
    const Particle* daughter = mother->getDaughter(ichild);
    std::vector<unsigned> pard;
    if (daughter->getNDaughters() > 0) {
      updateMapOfTrackAndDaughter(l, pars, pard, allparticles, daughter);
      pars.push_back(pard);
      allparticles.push_back(bDau[ichild]);
    } else {
      pard.push_back(l);
      pars.push_back(pard);
      allparticles.push_back(bDau[ichild]);
      l++;
    }
  }

  if (l == fitObjectContainer->size() - m_addUnmeasuredPhoton) {

    if (fitter.getError() == 0) {
      for (unsigned iDaug = 0; iDaug < allparticles.size(); iDaug++) {
        ROOT::Math::PxPyPzEVector tlv ;
        TMatrixFSym errMatrixU(7);
        if (pars[iDaug].size() > 0) {
          for (unsigned int iChild : pars[iDaug]) {
            BaseFitObject* fo = fitObjectContainer->at(iChild);
            auto* fitobject = static_cast<ParticleFitObject*>(fo);
            ROOT::Math::PxPyPzEVector tlv_sub = getLorentzVector(fitobject);
            TMatrixFSym errMatrixU_sub = getCovMat7(fitobject);
            tlv = tlv + tlv_sub;
            errMatrixU = errMatrixU + errMatrixU_sub;
          }
        } else {
          B2FATAL("ParticleKinematicFitterModule:   no fitObject could be used to update the daughter!");
        }
        ROOT::Math::XYZVector pos = allparticles[iDaug]->getVertex(); // we don't update the vertex yet
        TMatrixFSym errMatrix     = allparticles[iDaug]->getMomentumVertexErrorMatrix();
        TMatrixFSym errMatrixMom  = allparticles[iDaug]->getMomentumErrorMatrix();
        TMatrixFSym errMatrixVer  = allparticles[iDaug]->getVertexErrorMatrix();

        for (int i = 0; i < 3; i++) {
          for (int j = i; j < 3; j++) {
            errMatrixU[i + 4][j + 4] = errMatrixVer[i][j];
          }
        }

        allparticles[iDaug]->set4Vector(tlv);
        allparticles[iDaug]->setVertex(pos);
        allparticles[iDaug]->setMomentumVertexErrorMatrix(errMatrixU);
      }
    }

    return true;
  } else {
    B2ERROR("updateOrcaKinFitDaughters: Cannot update daughters, mismatch between number of daughters and number of fitobjects!");
    return false;
  }
}

void ParticleKinematicFitterModule::updateMapOfTrackAndDaughter(unsigned& l, std::vector<std::vector<unsigned>>& pars,
    std::vector<unsigned>& parm, std::vector<Particle*>&  allparticles, const Particle* daughter)
{
  std::vector <Belle2::Particle*> dDau = daughter->getDaughters();
  for (unsigned ichild = 0; ichild < daughter->getNDaughters(); ichild++) {
    const Particle* child = daughter->getDaughter(ichild);
    std::vector<unsigned> pard;
    if (child->getNDaughters() > 0) {
      updateMapOfTrackAndDaughter(l, pars, pard, allparticles, child);
      parm.insert(parm.end(), pard.begin(), pard.end());
      pars.push_back(pard);
      allparticles.push_back(dDau[ichild]);
    } else {
      pard.push_back(l);
      parm.push_back(l);
      pars.push_back(pard);
      allparticles.push_back(dDau[ichild]);
      l++;
    }
  }
}

void ParticleKinematicFitterModule::updateOrcaKinFitMother(BaseFitter& fitter, std::vector<Particle*>& particleChildren,
                                                           Particle* mother)
{
  // get old values
  ROOT::Math::XYZVector pos = mother->getVertex();
  TMatrixFSym errMatrix = mother->getMomentumVertexErrorMatrix();
  float pvalue          = mother->getPValue();

  // update momentum vector
  ROOT::Math::PxPyPzEVector momnew(0., 0., 0., 0.);

  std::vector <BaseFitObject*>* fitObjectContainer = fitter.getFitObjects();
  for (unsigned iChild = 0; iChild < particleChildren.size(); iChild++) {
    BaseFitObject* fo = fitObjectContainer->at(iChild);
    auto* fitobject = static_cast<ParticleFitObject*>(fo);
    ROOT::Math::PxPyPzEVector tlv = getLorentzVector(fitobject);
    momnew += tlv;
  }

  // update
  // TODO: use pvalue of the fit or the old one of the mother? use fit covariance matrix?
  // Maybe here should use the pvalue and errmatrix of the fit  ----Yu Hu
  mother->updateMomentum(momnew, pos, errMatrix, pvalue);
}

bool ParticleKinematicFitterModule::storeOrcaKinFitParticles(const std::string& prefix, BaseFitter& fitter,
    std::vector<Particle*>& particleChildren, Particle* mother)
{
  bool updated = false;
  std::vector <BaseFitObject*>* fitObjectContainer = fitter.getFitObjects();

  for (unsigned iChild = 0; iChild < particleChildren.size(); iChild++) {
    BaseFitObject* fo = fitObjectContainer->at(iChild);
    auto* fitobject = static_cast<ParticleFitObject*>(fo);
    ROOT::Math::PxPyPzEVector tlv = getLorentzVector(fitobject);

    // name of extra variables
    std::string extraVariableParticlePx    = "OrcaKinFit" + prefix + "_" + SSTR(iChild) + "_Px";
    std::string extraVariableParticlePy    = "OrcaKinFit" + prefix + "_" + SSTR(iChild) + "_Py";
    std::string extraVariableParticlePz    = "OrcaKinFit" + prefix + "_" + SSTR(iChild) + "_Pz";
    std::string extraVariableParticleE     = "OrcaKinFit" + prefix + "_" + SSTR(iChild) + "_E";
    std::string extraVariableParticlePxErr = "OrcaKinFit" + prefix + "_" + SSTR(iChild) + "_PxErr";
    std::string extraVariableParticlePyErr = "OrcaKinFit" + prefix + "_" + SSTR(iChild) + "_PyErr";
    std::string extraVariableParticlePzErr = "OrcaKinFit" + prefix + "_" + SSTR(iChild) + "_PzErr";
    std::string extraVariableParticleEErr  = "OrcaKinFit" + prefix + "_" + SSTR(iChild) + "_EErr";

    mother->addExtraInfo(extraVariableParticlePx, tlv.Px());
    mother->addExtraInfo(extraVariableParticlePy, tlv.Py());
    mother->addExtraInfo(extraVariableParticlePz, tlv.Pz());
    mother->addExtraInfo(extraVariableParticleE, tlv.E());
    mother->addExtraInfo(extraVariableParticlePxErr, getFitObjectError(fitobject, 0));
    mother->addExtraInfo(extraVariableParticlePyErr, getFitObjectError(fitobject, 1));
    mother->addExtraInfo(extraVariableParticlePzErr, getFitObjectError(fitobject, 2));
    mother->addExtraInfo(extraVariableParticleEErr, -1.0);

  }

  return updated;
}

float ParticleKinematicFitterModule::getFitObjectError(ParticleFitObject* fitobject, int ilocal)
{
  //check if it is a PxPyPzMFitObject
  auto* pxpypzmfitobject = static_cast<PxPyPzMFitObject*>(fitobject);
  if (pxpypzmfitobject) {
    return fitobject->getError(ilocal);
  } else {
    B2FATAL("ParticleKinematicFitterModule: not implemented yet");
  }
}

// this returns the local covariance matrix
TMatrixFSym ParticleKinematicFitterModule::getFitObjectCovMat(ParticleFitObject* fitobject)
{

  //check if it is a PxPyPzMFitObject
  auto* pxpypzmfitobject = static_cast<PxPyPzMFitObject*>(fitobject);
  if (pxpypzmfitobject) {

    TMatrixFSym errMatrix(3);

    //loop over the i-j local variables.
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        errMatrix[i][j] = pxpypzmfitobject->getCov(i, j);
      }
    }

    return errMatrix;
  } else {
    B2FATAL("ParticleKinematicFitterModule: not implemented yet");
  }
}

TMatrixFSym ParticleKinematicFitterModule::getCovMat7(ParticleFitObject* fitobject)
{
  TMatrixFSym fitCovMatrix(3);

  if (strcmp(fitobject->getParamName(0), "E") == 0) {
    //check if it is a JetFitObject
    auto* jetfitObject = static_cast<JetFitObject*>(fitobject);
    if (jetfitObject) {

      fitCovMatrix = getFitObjectCovMat(fitobject);
      ROOT::Math::PxPyPzEVector tlv = getLorentzVector(fitobject);

      const double energy = tlv.E();
      const double theta  = tlv.Theta();
      const double phi    = tlv.Phi();

      const double st    = sin(theta);
      const double ct    = cos(theta);
      const double sp    = sin(phi);
      const double cp    = cos(phi);

      // updated covariance matrix is: A * cov * A^T where A is the Jacobi matrix (use Similarity)
      TMatrixF A(7, 3);
      A(0, 0) =  cp * st ; // dpx/dE
      A(0, 1) =  energy * cp * ct ; // dpx/dtheta
      A(0, 2) = -energy *  sp * st ; // dpx/dphi
      A(1, 0) =  sp * st ; // dpy/dE
      A(1, 1) =  energy *  sp * ct ; // dpz/dtheta
      A(1, 2) =  energy *  cp * st ; // dpy/dphi
      A(2, 0) =  ct ; // dpz/dE
      A(2, 1) = -energy * st ; //   dpz/dtheta
      A(2, 2) =  0 ; // dpz/dphi
      A(3, 0) = 1.0; // dE/dE
      A(3, 1) = 0.0; // dE/dphi
      A(3, 2) = 0.0; // dE/dtheta

      TMatrixFSym D = fitCovMatrix.Similarity(A);
      return D;

    } else {
      B2FATAL("ParticleKinematicFitterModule: not implemented yet");
    }
  } else {
    //check if it is a PxPyPzMFitObject
    auto* pxpypzmfitobject = static_cast<PxPyPzMFitObject*>(fitobject);
    if (pxpypzmfitobject) {

      fitCovMatrix = getFitObjectCovMat(fitobject);

      // updated covariance matrix is: A * cov * A^T where A is the Jacobi matrix (use Similarity)
      ROOT::Math::PxPyPzEVector tlv = getLorentzVector(fitobject);
      TMatrixF A(7, 3);
      A[0][0] = 1.; // px/dpx
      A[0][1] = 0.; // px/dpy
      A[0][2] = 0.; // px/dpz
      A[1][0] = 0.; // py/dpx
      A[1][1] = 1.; // py/dpy
      A[1][2] = 0.; // py/dpz
      A[2][0] = 0.; // pz/dpx
      A[2][1] = 0.; // pz/dpy
      A[2][2] = 1.; // pz/dpz
      if (tlv.E() > 0.0) {
        A[3][0] = tlv.Px() / tlv.E(); // E/dpx, E=sqrt(px^2 + py^2 + pz^2 + m^2)
        A[3][1] = tlv.Py() / tlv.E(); // E/dpy
        A[3][2] = tlv.Pz() / tlv.E(); // E/dpz
      }

      TMatrixFSym D = fitCovMatrix.Similarity(A);

      return D;
    } else {
      B2FATAL("ParticleKinematicFitterModule: not implemented yet");
    }
  }
}
