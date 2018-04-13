/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <sstream>
#define SSTR( x ) dynamic_cast< std::ostringstream & >(( std::ostringstream() << std::dec << x ) ).str()

// kinfitter
#include <analysis/modules/ParticleKinematicFitter/ParticleKinematicFitterModule.h>
#include <analysis/OrcaKinFit/PxPyPzMFitObject.h>
#include <analysis/OrcaKinFit/JetFitObject.h>
#include <analysis/OrcaKinFit/TextTracer.h>
#include <analysis/OrcaKinFit/BaseFitter.h>
#include <analysis/OrcaKinFit/BaseFitObject.h>

#include <mdst/dataobjects/ECLCluster.h>

// framework datastore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework utilities
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// analysis dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// analysis utilities (uses beamparameters)
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>

// extrainfo
#include <analysis/dataobjects/EventExtraInfo.h>

// others
#include <Math/SMatrix.h>
#include <TMath.h>
#include <TMatrixD.h>

// CLHEP
#include <CLHEP/Matrix/Matrix.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Geometry/Point3D.h>

using namespace CLHEP;
using namespace std;
//using namespace OrcaKinFit;

namespace Belle2 {
  namespace OrcaKinFit {



    //-----------------------------------------------------------------
    //                 Register module
    //-----------------------------------------------------------------

    REG_MODULE(ParticleKinematicFitter)

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    ParticleKinematicFitterModule::ParticleKinematicFitterModule() : Module(), m_initial(BeamParameters::c_smearALL)
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
      addParam("add3CPhoton", m_add3CPhoton, "Add one photon with unmeasured energy (-1C).", false);
      addParam("decayString", m_decayString, "Specifies which daughter particles are included in the kinematic fit.", string(""));
      addParam("updateMother", m_updateMother, "Update the mother kinematics.", true);
      addParam("updateDaughters", m_updateDaughters, "Update the daughter kinematics.", false);
      addParam("debugBeam", m_debugBeam, "Create debug plots for all HER, LER, and Beam kinematics.", true);
      addParam("debugBeamFilename", m_debugBeamFilename, "Debug file filename.", string("debug_beam.root"));
      addParam("nMCInitialParticles", m_nMCInitialParticles, "Number of initial MC particles used in debug plots.", 1000000);
      addParam("recoilMass", m_recoilMass, "Recoil mass in GeV. RecoilMass constraint only..", 0.0);
      addParam("invMass", m_invMass, "Inviriant mass in GeV. Mass constraint only..", 0.0);

    }


    void ParticleKinematicFitterModule::initialize()
    {
      StoreObjPtr<EventExtraInfo>::registerPersistent("", DataStore::c_Event, false);

      if (m_decayString != "")
        m_decaydescriptor.init(m_decayString);

      if (m_decayString != "")
        B2INFO("ParticleKinematicFitter: Using specified decay string: " << m_decayString);

      // If debug file is requested
      if (m_debugBeam) {

        //initial particle for beam parameters
        m_initial.initialize();

        m_debugFile = new TFile(m_debugBeamFilename.c_str(), "RECREATE");

        for (int i = 0; i < 3; ++i) {
          m_th1d_beam_phi[i]   = new TH1D(Form("m_th1d_beam_phi_%i", i), Form("m_th1d_beam_phi_%i; #phi (deg); Entries / Bin", i), 720, 0,
                                          360);
          m_th1d_beam_theta[i] = new TH1D(Form("m_th1d_beam_theta_%i", i), Form("m_th1d_beam_theta_%i; #theta (deg); Entries / Bin", i), 360,
                                          0, 180);
          m_th1d_beam_E[i]     = new TH1D(Form("m_th1d_beam_E_%i", i), Form("m_th1d_beam_E_%i; E (GeV); Entries / Bin", i), 120000, 0, 12);
          m_th1d_beam_px[i]    = new TH1D(Form("m_th1d_beam_px_%i", i), Form("m_th1d_beam_px_%i; p_{x} (GeV); Entries / Bin", i), 40000, -2,
                                          2);
          m_th1d_beam_py[i]    = new TH1D(Form("m_th1d_beam_py_%i", i), Form("m_th1d_beam_py_%i; p_{y} (GeV); Entries / Bin", i), 40000, -2,
                                          2);
          m_th1d_beam_pz[i]    = new TH1D(Form("m_th1d_beam_pz_%i", i), Form("m_th1d_beam_pz_%i; p_{z} (GeV); Entries / Bin", i), 220000, -11,
                                          11);
          m_th1d_beam_pt[i]    = new TH1D(Form("m_th1d_beam_pt_%i", i), Form("m_th1d_beam_pt_%i; p_{z} (GeV); Entries / Bin", i), 40000, -2,
                                          2);
          m_th1d_beam_eta[i]   = new TH1D(Form("m_th1d_beam_eta_%i", i), Form("m_th1d_beam_eta_%i; #eta; Entries / Bin", i), 60000, -3, 3);
          m_th1d_beam_M[i]     = new TH1D(Form("m_th1d_beam_M_%i", i), Form("m_th1d_beam_M_%i; M (GeV); Entries / Bin", i), 120000, 0, 12);
        }

        // create MCInitialParticles.
        for (int i = 0; i < m_nMCInitialParticles; ++i) {
          MCInitialParticles& initial = m_initial.generate();

          TLorentzVector beam[3] = {initial.getLER(), initial.getHER(), initial.getLER() + initial.getHER()};

          for (int j = 0; j < 3; ++j) {
            m_th1d_beam_E[j]->Fill(beam[j].E());
            m_th1d_beam_px[j]->Fill(beam[j].Px());
            m_th1d_beam_py[j]->Fill(beam[j].Py());
            m_th1d_beam_pz[j]->Fill(beam[j].Pz());
            m_th1d_beam_M[j]->Fill(beam[j].M());
            m_th1d_beam_eta[j]->Fill(beam[j].Eta());
            m_th1d_beam_pt[j]->Fill(beam[j].Pt());
            m_th1d_beam_phi[j]->Fill(beam[j].Phi()*TMath::RadToDeg());
            m_th1d_beam_theta[j]->Fill(beam[j].Theta()*TMath::RadToDeg());
          }
        }

        // save file.
        if (m_debugFile) {
          m_debugFile->Write();
          m_debugFile->Close();
          if (m_debugFile) delete m_debugFile;
        }


      }

    }


    void ParticleKinematicFitterModule::beginRun()
    {
      B2INFO("ParticleKinematicFitterModule::beginRun");
    }

    void ParticleKinematicFitterModule::endRun()
    {
      B2INFO("ParticleKinematicFitterModule::endRun");
    }

    void ParticleKinematicFitterModule::terminate()
    {
      B2INFO("ParticleKinematicFitterModule::terminate");
    }


    void ParticleKinematicFitterModule::event()
    {
      B2INFO("ParticleKinematicFitterModule::event");

      StoreObjPtr<ParticleList> plist(m_listName);
      if (!plist) {
        B2ERROR("ParticleList " << m_listName << " not found");
        return;
      }

      unsigned int n = plist->getListSize();

      for (unsigned i = 0; i < n; i++) {
//       Particle* particle = const_cast<Particle*> (plist->getParticle(i));
        Particle* particle = plist->getParticle(i);

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

      B2INFO("ParticleKinematicFitterModule::doKinematicFit");

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

      }
      // invalid fitter
      else if (m_kinematicFitter != "OrcaKinFit") {
        B2FATAL("ParticleKinematicFitter: " << m_kinematicFitter << " is an invalid kinematic fitter!");
      }

      if (!ok) return false;

      return true;

    }

//==========================================================================
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

      // choos eminimization
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
      }
      BaseFitter& fitter = *pfitter;

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
      mother->addExtraInfo("OrcaKinFitChi2", chi2);
      mother->addExtraInfo("OrcaKinFitErrorCode", errorcode);

      // if we added an unmeasured photon, add the kinematics to the mother - at some point we may want to create a particle list from this?
      if (m_addUnmeasuredPhoton) {
        std::vector <BaseFitObject*>* fitObjectContainer = fitter.getFitObjects();
        for (unsigned iChild = 0; iChild < fitObjectContainer->size(); iChild++) {
          BaseFitObject* fo = fitObjectContainer->at(iChild);
          const std::string name = fo->getName();
          if (name == "unmeasured") {
            ParticleFitObject* fitobject = (ParticleFitObject*) fo;
            TLorentzVector tlv = getTLorentzVector(fitobject);
            mother->addExtraInfo("OrcaKinFitUnmeasuredTheta", tlv.Theta());
            mother->addExtraInfo("OrcaKinFitUnmeasuredPhi", tlv.Phi());
            mother->addExtraInfo("OrcaKinFitUnmeasuredE", tlv.E());

            // Uncertainty
//           const double err0 = getFitObjectError(fitobject, 0);
            mother->addExtraInfo("OrcaKinFitUnmeasuredErrorTheta", getFitObjectError(fitobject, 1));
            mother->addExtraInfo("OrcaKinFitUnmeasuredErrorPhi", getFitObjectError(fitobject, 2));
            mother->addExtraInfo("OrcaKinFitUnmeasuredErrorE", getFitObjectError(fitobject, 0));

          }
        }
      }

      if (pfitter) delete pfitter;
      return true;
    }

    bool ParticleKinematicFitterModule::fillFitParticles(Particle* mother, std::vector<Particle*>& particleChildren)
    {
      for (unsigned ichild = 0; ichild < mother->getNDaughters(); ichild++) {
        Particle* child = const_cast<Particle*>(mother->getDaughter(ichild));

        if (child->getPValue() > 0) {
          particleChildren.push_back(child);
        } else if (child->getNDaughters() > 0) {
          bool err = fillFitParticles(child, particleChildren);
          if (!err) {
            B2WARNING("ParticleKinematicFitterModule: Cannot find valid children for the fit.");
            return false;
          }
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
        Particle* child = const_cast<Particle*>(mother->getDaughter(ichild));

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
      B2INFO("ParticleKinematicFitterModule: adding a particle to the fitter!");

      if (m_add3CPhoton && index == 0) {
        if (particle -> getPDGCode() != 22) {
          B2ERROR("In 3C Kinematic fit, the first daughter should be the Unmeasured Photon!");
        }

        double startingE = particle -> getECLCluster() -> getEnergy();
        double startingPhi = particle -> getECLCluster() -> getPhi();
        double startingTheta = particle -> getECLCluster() -> getTheta();

        double startingeE = particle->getECLCluster() -> getUncertaintyEnergy();
        double startingePhi = particle->getECLCluster() -> getUncertaintyPhi();
        double startingeTheta = particle->getECLCluster() -> getUncertaintyTheta();

        B2DEBUG(17, startingE << " " << startingPhi << " " << startingTheta << " " << startingeE << " " << startingePhi << " " <<
                startingeTheta);
        // create a fit object
        ParticleFitObject* pfitobject;
        pfitobject  = new JetFitObject(startingE, startingPhi, startingTheta, startingeE, startingePhi, startingeTheta, 0.);
        pfitobject->setParam(0, startingE, false, false);
        pfitobject->setParam(1, startingPhi, true, false);
        pfitobject->setParam(2, startingTheta, true, false);

        std::string fitObjectName = "unmeasured";
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
        CLHEP::HepSymMatrix clhepmomentumvertexerrormatrix = getCLHEPMomentumVertexErrorMatrix(particle);

        // create the fit object (ParticleFitObject is the base class)
        ParticleFitObject* pfitobject;
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
      CLHEP::HepLorentzVector mom(particle->getPx(), particle->getPy(), particle->getPz(), particle->getEnergy());
      return mom;
    }


    TLorentzVector ParticleKinematicFitterModule::getTLorentzVector(ParticleFitObject* fitobject)
    {
      TLorentzVector mom(fitobject->getPx(), fitobject->getPy(), fitobject->getPz(), fitobject->getE());
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

    TLorentzVector ParticleKinematicFitterModule::getTLorentzVectorConstraints()
    {

      if (m_orcaConstraint == "HardBeam") {
        TLorentzVector constraints4vector(m_hardConstraintPx.getValue(),
                                          m_hardConstraintPy.getValue(),
                                          m_hardConstraintPz.getValue(),
                                          m_hardConstraintE.getValue());
        return constraints4vector;
      } else {
        B2FATAL("ParticleKinematicFitterModule:  " << m_orcaConstraint << " is an invalid OrcaKinFit constraint!");
      }

      // should not reach this point...
      return TLorentzVector(0., 0., 0., 0.);
    }

    void ParticleKinematicFitterModule::setConstraints()
    {

      if (m_orcaConstraint == "HardBeam") {
        PCmsLabTransform T;
        const TLorentzVector boost = T.getBoostVector();

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
        const TLorentzVector boost = T.getBoostVector();

        m_hardConstraintRecoilMass = RecoilMassConstraint(m_recoilMass, boost.Px(), boost.Py(), boost.Pz(), boost.E());

        m_hardConstraintRecoilMass.resetFOList();
        m_hardConstraintRecoilMass.setName("Recoil Mass [hard]");

      } else if (m_orcaConstraint == "Mass") {
        PCmsLabTransform T;
        const TLorentzVector boost = T.getBoostVector();

        m_hardConstraintMass = MassConstraint(m_invMass);

        m_hardConstraintMass.resetFOList();
        m_hardConstraintMass.setName("Mass [hard]");
      } else {
        B2FATAL("ParticleKinematicFitterModule:  " << m_orcaConstraint << " is an invalid OrcaKinFit constraint!");
      }
    }


    void ParticleKinematicFitterModule::resetFitter(BaseFitter& fitter)
    {
      B2INFO("ParticleKinematicFitterModule: Resetting the fitter");
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
      // Initialize photon using the existing constraints
      TLorentzVector tlv = getTLorentzVectorConstraints();
      double startingE = tlv.E();
      double startingPhi = tlv.Phi();
      double startingTheta = tlv.Theta();

      // create a fit object
      ParticleFitObject* pfitobject;
      pfitobject  = new JetFitObject(startingE, startingPhi, startingTheta, 0.0, 0.0, 0.0, 0.);
      pfitobject->setParam(0, startingE, false, false);
      pfitobject->setParam(1, startingPhi, false, false);
      pfitobject->setParam(2, startingTheta, false, false);

      std::string fitObjectName = "unmeasured";
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
      std::vector<std::vector<unsigned>> u(nd);
      for (unsigned ichild = 0; ichild < nd; ichild++) {
        const Particle* daughter = mother->getDaughter(ichild);
        if (daughter->getNDaughters() > 0 && daughter->getPValue() < 0) {
          updateMapofTrackandDaughter(u[ichild], l, daughter);
        } else {
          u[ichild].push_back(l);
          l++;
        }
      }

      if (l == fitObjectContainer->size() - m_addUnmeasuredPhoton) {

        if (fitter.getError() == 0) {
          for (unsigned iDaug = 0; iDaug < bDau.size(); iDaug++) {
            TLorentzVector tlv ;
            TMatrixFSym errMatrixU(7);
            if (u[iDaug].size() > 0) {
              for (unsigned iChild = 0; iChild < u[iDaug].size(); iChild++) {
                BaseFitObject* fo = fitObjectContainer->at(u[iDaug][iChild]);
                ParticleFitObject* fitobject = (ParticleFitObject*) fo;
                TLorentzVector tlv_sub = getTLorentzVector(fitobject);
                TMatrixFSym errMatrixU_sub = getCovMat7(fitobject);
                tlv = tlv + tlv_sub;
                errMatrixU = errMatrixU + errMatrixU_sub;
              }
            } else {
              BaseFitObject* fo = fitObjectContainer->at(iDaug);
              ParticleFitObject* fitobject = (ParticleFitObject*) fo;
              TLorentzVector tlv = getTLorentzVector(fitobject);

              TMatrixFSym errMatrixU = getCovMat7(fitobject);
            }
            TVector3 pos          = bDau[iDaug]->getVertex(); // we dont update the vertex yet
            TMatrixFSym errMatrix = bDau[iDaug]->getMomentumVertexErrorMatrix();
            TMatrixFSym errMatrixMom = bDau[iDaug]->getMomentumErrorMatrix();
            bDau[iDaug]->set4Vector(tlv);
            bDau[iDaug]->setVertex(pos);
            bDau[iDaug]->setMomentumVertexErrorMatrix(errMatrixU);
          }
        }

        return true;
      } else {
        B2ERROR("updateOrcaKinFitDaughters: Cannot update daughters, mismatch betwen number of daughters and number of fitobjects!");
        return false;
      }

    }

    void ParticleKinematicFitterModule::updateMapofTrackandDaughter(std::vector<unsigned>& ui, unsigned& l,
        const       Particle* daughter)
    {
      for (unsigned ichild = 0; ichild < daughter->getNDaughters(); ichild++) {
        const Particle* child = daughter->getDaughter(ichild);
        if (child->getNDaughters() > 0) updateMapofTrackandDaughter(ui, l, child);
        else  ui.push_back(l);
        l++;
      }
    }


    void ParticleKinematicFitterModule::updateOrcaKinFitMother(BaseFitter& fitter, std::vector<Particle*>& particleChildren,
                                                               Particle* mother)
    {
      // get old values
      TLorentzVector mom    = mother->get4Vector();
      TVector3 pos          = mother->getVertex();
      TMatrixFSym errMatrix = mother->getMomentumVertexErrorMatrix();
      float pvalue          = mother->getPValue();

      // update momentum vector
      TLorentzVector momnew(0., 0., 0., 0.);

      std::vector <BaseFitObject*>* fitObjectContainer = fitter.getFitObjects();
      for (unsigned iChild = 0; iChild < particleChildren.size(); iChild++) {
        BaseFitObject* fo = fitObjectContainer->at(iChild);
        ParticleFitObject* fitobject = (ParticleFitObject*) fo;
        TLorentzVector tlv = getTLorentzVector(fitobject);
        momnew += tlv;
      }

      // update
      // TODO: use pvalue of the fit or the old one of the mother? use fit covariance matrix?
      // Maybe here should use the pvalue and errmatrix of the fit  ----Yu Hu
      mother->updateMomentum(momnew, pos, errMatrix, pvalue);
    }


    bool ParticleKinematicFitterModule::storeOrcaKinFitParticles(std::string prefix, BaseFitter& fitter,
        std::vector<Particle*>& particleChildren, Particle* mother)
    {
      bool updated = false;
      std::vector <BaseFitObject*>* fitObjectContainer = fitter.getFitObjects();

      for (unsigned iChild = 0; iChild < particleChildren.size(); iChild++) {
        BaseFitObject* fo = fitObjectContainer->at(iChild);
        ParticleFitObject* fitobject = (ParticleFitObject*) fo;
        TLorentzVector tlv = getTLorentzVector(fitobject);

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
      PxPyPzMFitObject* pxpypzmfitobject = (PxPyPzMFitObject*) fitobject;
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
      PxPyPzMFitObject* pxpypzmfitobject = (PxPyPzMFitObject*) fitobject;
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

      //check if it is a PxPyPzMFitObject
      PxPyPzMFitObject* pxpypzmfitobject = (PxPyPzMFitObject*) fitobject;
      if (pxpypzmfitobject) {

        fitCovMatrix = getFitObjectCovMat(fitobject);

        // updated covariance matrix is: A * cov * A^T where A is the Jacobi matrix (use Similarity)
        TLorentzVector tlv = getTLorentzVector(fitobject);
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


  }// end OrcaKinFit namespace

} // end Belle2 namespace
