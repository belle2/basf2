/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi, Thibaud Humair                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/TagVertex/TagVertexModule.h>

//to help printing out stuff
#include<sstream>

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
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/TagVertex.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>

// utilities
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/variables/TrackVariables.h>
#include <analysis/utility/ParticleCopy.h>
#include <analysis/utility/CLHEPToROOT.h>
#include <analysis/utility/ROOTToCLHEP.h>
#include <analysis/utility/DistanceTools.h>
#include <analysis/utility/RotationTools.h>

// vertex fitting
#include <analysis/VertexFitting/KFit/VertexFitKFit.h>

// msdt dataobject
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/HitPatternVXD.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

#include <TVector.h>
#include <TRotation.h>


using namespace std;

namespace Belle2 {

  // import tools from RotationTools.h
  using RotationTools::getRotationMatrixYZ;
  using RotationTools::getRotationMatrixXY;
  using RotationTools::toSymMatrix;

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------
  REG_MODULE(TagVertex)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TagVertexModule::TagVertexModule() : Module(),
    m_Bfield(0), m_rollbackStatus(0), m_fitTruthStatus(0), m_fitPval(0), m_MCtagLifeTime(-1), m_mcPDG(0), m_MCLifeTimeReco(-1),
    m_deltaT(0), m_deltaTErr(0), m_MCdeltaT(0), m_MCdeltaTapprox(0),
    m_shiftZ(0), m_FitType(0), m_tagVl(0),
    m_truthTagVl(0), m_tagVlErr(0), m_tagVol(0), m_truthTagVol(0), m_tagVolErr(0), m_tagVNDF(0), m_tagVChi2(0), m_tagVChi2IP(0),
    m_verbose(true)
  {
    // Set module properties
    setDescription("Tag side Vertex Fitter for modular analysis");

    // Parameter definitions
    addParam("listName", m_listName, "name of particle list", string(""));
    addParam("confidenceLevel", m_confidenceLevel,
             "required confidence level of fit to keep particles in the list. Note that even with confidenceLevel == 0.0, errors during the fit might discard Particles in the list. confidenceLevel = -1 if an error occurs during the fit",
             0.001);
    addParam("MCAssociation", m_useMCassociation,
             "'': no MC association. breco: use standard Breco MC association. internal: use internal MC association", string("breco"));
    addParam("constraintType", m_constraintType,
             "Choose the type of the constraint: noConstraint, IP (tag tracks constrained to be within the beam spot), tube (long tube along the BTag line of flight, only for fully reconstruced B rec), boost (long tube along the Upsilon(4S) boost direction), (breco)",
             string("IP"));
    addParam("trackFindingType", m_trackFindingType,
             "Choose how to reconstruct the tracks on the tag side: standard, standard_PXD",
             string("standard_PXD"));
    addParam("maskName", m_roeMaskName,
             "Choose ROE mask to get particles from ", string(""));
    addParam("askMCInformation", m_MCInfo,
             "TRUE when requesting MC Information from the tracks performing the vertex fit", false);
    addParam("reqPXDHits", m_reqPXDHits,
             "Minium number of PXD hits for a track to be used in the vertex fit", 0);
    addParam("fitAlgorithm", m_fitAlgo,
             "Fitter used for the tag vertex fit: Rave or KFit", string("Rave"));
    addParam("useTruthInFit", m_useTruthInFit,
             "Use the true track parameters in the vertex fit", false);
    addParam("useRollBack", m_useRollBack,
             "Use rolled back non-primary tracks", false);


  }

  void TagVertexModule::initialize()
  {
    // magnetic field
    m_Bfield = BFieldManager::getField(m_BeamSpotCenter).Z() / Unit::T;
    // RAVE setup
    analysis::RaveSetup::initialize(1, m_Bfield);
    B2INFO("TagVertexModule : magnetic field = " << m_Bfield);
    // truth fit status will be set to 2 only if the MC info cannot be recovered
    if (m_useTruthInFit) m_fitTruthStatus = 1;
    // roll back status will be set to 2 only if the MC info cannot be recovered
    if (m_useRollBack) m_rollbackStatus = 1;

    //TODO: this won't work with nonstandard name for Particle array (e.g. will fail when adding relations)
    //input
    StoreArray<Particle> particles;
    particles.isRequired();
    // output
    StoreArray<TagVertex> verArray;
    verArray.registerInDataStore();
    particles.registerRelationTo(verArray);
    //check if the fitting algorithm name  is set correctly
    if (m_fitAlgo != "Rave" && m_fitAlgo != "KFit")
      B2FATAL("TagVertexModule: invalid fitting algorithm (must be set to either Rave or KFit).");
    //temporary while the one track fit is broken
    if (m_trackFindingType == "singleTrack" || m_trackFindingType == "singleTrack_PXD")
      B2FATAL("TagVertexModule : the singleTrack option is temporarily broken.");
  }

  void TagVertexModule::beginRun()
  {
    //TODO: set magnetic field for each run
    //m_Bfield = BFieldMap::Instance().getBField(m_BeamSpotCenter).Z();
  }

  void TagVertexModule::event()
  {
    StoreObjPtr<ParticleList> plist(m_listName);
    if (!plist) {
      B2ERROR("TagVertexModule: ParticleList " << m_listName << " not found");
      return;
    }

    // input
    StoreArray<Particle> Particles(plist->getParticleCollectionName());

    // output
    StoreArray<TagVertex> verArray;
    analysis::RaveSetup::initialize(1, m_Bfield);

    std::vector<unsigned int> toRemove;

    for (unsigned i = 0; i < plist->getListSize(); ++i) {
      resetReturnParams();

      Particle* particle =  plist->getParticle(i);
      if (m_useMCassociation == "breco" || m_useMCassociation == "internal") BtagMCVertex(particle);
      bool ok = doVertexFit(particle);
      if (ok) deltaT(particle);

      if ((m_fitPval < m_confidenceLevel && m_confidenceLevel != 0)
          || (m_fitPval <= m_confidenceLevel && m_confidenceLevel == 0)) {
        toRemove.push_back(particle->getArrayIndex());
      } else {
        // save information in the Vertex StoreArray
        TagVertex* ver = verArray.appendNew();
        // create relation: Particle <-> Vertex
        particle->addRelationTo(ver);
        // fill Vertex with content
        if (ok) {
          ver->setTagVertex(m_tagV);
          ver->setTagVertexErrMatrix(m_tagVErrMatrix);
          ver->setTagVertexPval(m_fitPval);
          ver->setDeltaT(m_deltaT);
          ver->setDeltaTErr(m_deltaTErr);
          ver->setMCTagVertex(m_MCtagV);
          ver->setMCTagBFlavor(m_mcPDG);
          ver->setMCDeltaT(m_MCdeltaT);
          ver->setMCDeltaTapprox(m_MCdeltaTapprox);
          ver->setFitType(m_FitType);
          ver->setNTracks(m_tagParticles.size());
          ver->setTagVl(m_tagVl);
          ver->setTruthTagVl(m_truthTagVl);
          ver->setTagVlErr(m_tagVlErr);
          ver->setTagVol(m_tagVol);
          ver->setTruthTagVol(m_truthTagVol);
          ver->setTagVolErr(m_tagVolErr);
          ver->setTagVNDF(m_tagVNDF);
          ver->setTagVChi2(m_tagVChi2);
          ver->setTagVChi2IP(m_tagVChi2IP);
          ver->setVertexFitParticles(m_raveParticles);
          ver->setVertexFitMCParticles(m_raveMCParticles);
          ver->setRaveWeights(m_raveWeights);
          ver->setConstraintType(m_constraintType);
          ver->setConstraintCenter(m_constraintCenter);
          ver->setConstraintCov(m_constraintCov);
          ver->setFitTruthStatus(m_fitTruthStatus);
          ver->setRollBackStatus(m_rollbackStatus);
        } else {
          ver->setTagVertex(m_tagV);
          ver->setTagVertexPval(-1.);
          ver->setDeltaT(m_deltaT);
          ver->setDeltaTErr(m_deltaTErr);
          ver->setMCTagVertex(m_MCtagV);
          ver->setMCTagBFlavor(0.);
          ver->setMCDeltaT(m_MCdeltaT);
          ver->setMCDeltaTapprox(m_MCdeltaTapprox);
          ver->setFitType(m_FitType);
          ver->setNTracks(m_tagParticles.size());
          ver->setTagVl(m_tagVl);
          ver->setTruthTagVl(m_truthTagVl);
          ver->setTagVlErr(m_tagVlErr);
          ver->setTagVol(m_tagVol);
          ver->setTruthTagVol(m_truthTagVol);
          ver->setTagVolErr(m_tagVolErr);
          ver->setTagVNDF(-1111.);
          ver->setTagVChi2(-1111.);
          ver->setTagVChi2IP(-1111.);
          ver->setVertexFitParticles(m_raveParticles);
          ver->setVertexFitMCParticles(m_raveMCParticles);
          ver->setRaveWeights(m_raveWeights);
          ver->setConstraintType(m_constraintType);
          ver->setConstraintCenter(m_constraintCenter);
          ver->setConstraintCov(m_constraintCov);
          ver->setFitTruthStatus(m_fitTruthStatus);
          ver->setRollBackStatus(m_rollbackStatus);
        }
      }

    }
    plist->removeParticles(toRemove);

    //free memory allocated by rave. initialize() would be enough, except that we must clean things up before program end...
    //
    analysis::RaveSetup::getInstance()->reset();

  }

  bool TagVertexModule::doVertexFit(Particle* Breco)
  {
    //reset the fit truth status in case it was set to 2 in a previous fit

    if (m_useTruthInFit) m_fitTruthStatus = 1;

    //reset the roll back status in case it was set to 2 in a previous fit

    if (m_useRollBack) m_rollbackStatus = 1;

    //set constraint type, reset pVal and B field

    m_fitPval = 1;
    bool ok = false;

    if (!(Breco->getRelatedTo<RestOfEvent>())) {
      m_FitType = -1;
      return false;
    }

    if (m_Bfield == 0) {
      B2ERROR("TagVertex: No magnetic field");
      return false;
    }

    // recover beam spot info

    m_BeamSpotCenter = m_beamSpotDB->getIPPosition();
    m_BeamSpotCov.ResizeTo(3, 3);
    m_BeamSpotCov = m_beamSpotDB->getCovVertex();


    //make the beam spot bigger for the standard constraint

    PCmsLabTransform T;
    TVector3 boost = T.getBoostVector();
    double bg = boost.Mag() / TMath::Sqrt(1 - boost.Mag2());

    //TODO: What's the origin of these numbers?
    double cut = 8.717575e-02 * bg;

    m_shiftZ = 4.184436e+02 * bg *  0.0001;

    //tube length here set to 20 * 2 * c tau beta gamma ~= 0.5 cm, should be enough to not bias the decay
    //time but should still help getting rid of some pions from kshorts
    if (m_constraintType == "IP") ok = findConstraintBoost(cut);
    if (m_constraintType == "tube") ok = findConstraintBTube(Breco, 1000 * cut);
    if (m_constraintType == "boost") ok = findConstraintBoost(cut * 200000.);
    if (m_constraintType == "noConstraint") ok = true;
    if (m_constraintType == "breco") ok = findConstraint(Breco, cut * 2000.);

    if (!ok) {
      B2ERROR("TagVertex: No correct fit constraint");
      return false;
    }

    /* Depending on the user's choice, one of the possible algorithms is chosen for the fit. In case the algorithm does not converge, in order to assure
       high efficiency, the next algorithm less restictive is used. I.e, if standard_PXD does not work, the program tries with standard.
    */

    m_FitType = 0;

    double minPVal(0.001);
    if (m_fitAlgo == "KFit") minPVal = 0.;

    if (m_trackFindingType == "standard_PXD") {
      ok = getTagTracks_standardAlgorithm(Breco, 1);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 3;

      }
    }

    if ((ok == false || m_fitPval < minPVal) || m_trackFindingType == "standard") {
      ok = getTagTracks_standardAlgorithm(Breco, m_reqPXDHits);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 4;
      }
    }

    if ((ok == false || (m_fitPval <= 0. && m_fitAlgo == "Rave")) && m_constraintType != "noConstraint") {
      ok = findConstraintBoost(cut * 200000.);
      if (ok) ok = getTagTracks_standardAlgorithm(Breco, m_reqPXDHits);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 5;
      }
    }

    return ok;

  }


  bool TagVertexModule::findConstraint(Particle* Breco, double cut)
  {
    if (Breco->getPValue() < 0.) return false;

    TMatrixDSym beamSpotCov(3);
    beamSpotCov = m_beamSpotDB->getCovVertex();

    analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, beamSpotCov);

    double pmag = Breco->getMomentumMagnitude();
    double xmag = (Breco->getVertex() - m_BeamSpotCenter).Mag();

    TVector3 Pmom = (pmag / xmag) * (Breco->getVertex() - m_BeamSpotCenter);

    TMatrixDSym TerrMatrix = Breco->getMomentumVertexErrorMatrix();
    TMatrixDSym PerrMatrix(7);

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (i == j) {
          PerrMatrix(i, j) = (beamSpotCov(i, j) + TerrMatrix(i, j)) * Pmom.Mag() / xmag;
        } else {
          PerrMatrix(i, j) = TerrMatrix(i, j);
        }
        PerrMatrix(i + 4, j + 4) = TerrMatrix(i + 4, j + 4);
      }
    }

    PerrMatrix(3, 3) = 0.;

    Particle* Breco2 = new Particle(Breco->get4Vector(), Breco->getPDGCode());
    Breco2->updateMomentum(Breco->get4Vector(), Breco->getVertex(), PerrMatrix, Breco->getPValue());

    analysis::RaveVertexFitter rsf;
    rsf.addTrack(Breco2);

    int nvert = rsf.fit("kalman");

    TVector3 pos;
    TMatrixDSym RerrMatrix(3);

    delete Breco2;


    if (nvert > 0) {
      pos = rsf.getPos(0);
      RerrMatrix = rsf.getCov(0);
    } else {return false;}


    // simpler version of momentum

    TVector3 pFinal = Breco->getVertex() - pos;
    TMatrixDSym errFinal = TMatrixDSym(Breco->getVertexErrorMatrix()) + RerrMatrix;

    // end simpler version

    // TODO : to be deveoped the extraction of the momentum from the rave fitted track

    // start rotation of the error matrix
    TLorentzVector v4Final(pFinal, Breco->getPDGMass());
    PCmsLabTransform T;
    TLorentzVector vec = T.rotateLabToCms() * v4Final;

    TMatrixD r1 = getRotationMatrixYZ(v4Final.Theta(), v4Final.Phi());
    TMatrixD r1t = r1; r1t.T();
    TMatrixD TubeZ = r1t * errFinal * r1;

    TubeZ(2, 2) = cut;
    TubeZ(2, 0) = 0; TubeZ(0, 2) = 0;
    TubeZ(2, 1) = 0; TubeZ(1, 2) = 0;

    vec.SetVect(-vec.Vect());

    TLorentzVector vecLab = T.rotateCmsToLab() * vec;
    TMatrixD r2 = getRotationMatrixYZ(vecLab.Theta(), vecLab.Phi()); r2.T(); //inverse rotation
    TMatrixD r2t = r2; r2t.T();
    TMatrixD Tube = r2t * TubeZ * r2;

    m_constraintCov.ResizeTo(3, 3);
    m_constraintCov = toSymMatrix(Tube);
    m_constraintCenter = m_BeamSpotCenter; // Standard algorithm needs no shift

    return true;

  }

  bool TagVertexModule::findConstraintBTube(Particle* Breco, double cut)
  {
    //Use Breco as the creator of the B tube.

    bool ok0(true);

    if ((Breco->getVertexErrorMatrix()(2, 2)) == 0.0) {
      B2WARNING("In TagVertexModule::findConstraintBTube: cannot get a proper vertex for BReco. BTube constraint replaced by Boost.");
      ok0 = findConstraintBoost(cut);
      return ok0;
    }

    //make a copy of tubecreatorB so as not to modify the original object

    Particle tubecreatorBCopy(Particle(Breco->get4Vector(), Breco->getPDGCode()));
    tubecreatorBCopy.updateMomentum(Breco->get4Vector(), Breco->getVertex(), Breco->getMomentumVertexErrorMatrix(),
                                    Breco->getPValue());

    //vertex fit will give the intersection between the beam spot and the trajectory of the B
    //(base of the BTube, or primary vtx cov matrix)

    ok0 = doVertexFitForBTube(&tubecreatorBCopy);

    if (!ok0) return false;

    //get direction of B tag = opposite direction of B rec in CMF

    TLorentzVector v4Final = tubecreatorBCopy.get4Vector();

    //if we want the true info, replace the 4vector by the true one

    if (m_useTruthInFit) {
      const MCParticle* mcBr = Breco->getRelated<MCParticle>();
      if (!mcBr)
        m_fitTruthStatus = 2;
      if (mcBr)
        v4Final = mcBr->get4Vector();
    }

    PCmsLabTransform T;
    TLorentzVector vec = T.rotateLabToCms() * v4Final;
    TLorentzVector vecNew(-vec.Vect(), vec.E());
    TLorentzVector v4FinalNew = T.rotateCmsToLab() * vecNew;

    //To creat the B tube, strategy is: take the primary vtx cov matrix, and add to it a cov
    //matrix corresponding to an very big error in the direction of the B tag

    TMatrixDSym pv = tubecreatorBCopy.getVertexErrorMatrix();

    //print some stuff if wanted
    if (m_verbose) {
      B2DEBUG(10, "Brec decay vertex before fit: " << printVector(Breco->getVertex()));
      B2DEBUG(10, "Brec decay vertex after fit: " << printVector(tubecreatorBCopy.getVertex()));
      B2DEBUG(10, "Brec direction before fit: " << printVector((1. / Breco->getP()) * Breco->getMomentum()));
      B2DEBUG(10, "Brec direction after fit: " << printVector((1. / tubecreatorBCopy.getP()) * tubecreatorBCopy.getMomentum()));
      B2DEBUG(10, "IP position: " << printVector(m_BeamSpotCenter));
      B2DEBUG(10, "IP covariance: " << printMatrix(m_BeamSpotCov));
      B2DEBUG(10, "Brec primary vertex: " << printVector(tubecreatorBCopy.getVertex()));
      B2DEBUG(10, "Brec PV covariance: " << printMatrix(pv));
      B2DEBUG(10, "BTag direction: " << printVector((1. / v4FinalNew.P())*v4FinalNew.Vect()));
      B2DEBUG(10, "BTag direction in CMF: " << printVector((1. / vecNew.P())*vecNew.Vect()));
      B2DEBUG(10, "Brec direction in CMF: " << printVector((1. / vec.P())*vec.Vect()));
    }


    // make rotation matrix from z axis to BTag line of flight
    TMatrixD r2 = getRotationMatrixYZ(v4FinalNew.Theta(), v4FinalNew.Phi());
    TMatrixD r2t = r2; r2t.T();


    //make a long error matrix along BTag direction
    TMatrixD longerror(3, 3); longerror(2, 2) = cut * cut;
    TMatrixD longerrorRotated = r2 * longerror * r2t;

    //pvNew will correspond to the covariance matrix of the B tube
    TMatrixD pvNew = TMatrixD(pv) + longerrorRotated;

    //set the constraint
    m_constraintCenter = tubecreatorBCopy.getVertex();

    //if we want the true info, set the centre of the constraint to the primary vertex

    if (m_useTruthInFit) {
      const MCParticle* mcBr = Breco->getRelated<MCParticle>();
      if (mcBr) {
        m_constraintCenter = mcBr->getProductionVertex();
      }
    }

    m_constraintCov.ResizeTo(3, 3);
    m_constraintCov = toSymMatrix(pvNew);

    if (m_verbose) {
      B2DEBUG(10, "IPTube covariance: " << printMatrix(m_constraintCov));
    }

    //The following is done to do the BTube constraint with a virtual track
    //(ie KFit way)

    m_tagMomentum = v4FinalNew;

    m_pvCov.ResizeTo(pv);
    m_pvCov = pv;

    return true;
  }


  bool TagVertexModule::findConstraintBoost(double cut, double shiftAlongBoost)
  {

    PCmsLabTransform T;

    TVector3 boost = T.getBoostVector();
    TVector3 boostDir = boost.Unit();

    TMatrixDSym beamSpotCov(3);
    beamSpotCov = m_beamSpotDB->getCovVertex();
    beamSpotCov(2, 2) = cut * cut; //cut on z-BeamSpot Cov

    TMatrixD r = getRotationMatrixYZ(-boostDir.Theta(), -boostDir.Phi());
    TMatrixD rt = r; rt.T();

    TMatrixD Tube = rt * beamSpotCov * r; //BeamSpot in CMS


    m_constraintCov.ResizeTo(3, 3);
    m_constraintCov = toSymMatrix(Tube);
    m_constraintCenter = m_BeamSpotCenter; // Standard algorithm needs no shift

    // The constraint used in the Single Track Fit needs to be shifted in the boost direction.

    if (shiftAlongBoost > -1000) {
      double boostAngle = atan2(boostDir[0] , boostDir[2]); // boost angle with respect from Z
      m_constraintCenter = m_BeamSpotCenter +
                           TVector3(shiftAlongBoost * sin(boostAngle), 0., shiftAlongBoost * cos(boostAngle)); // boost in the XZ plane
    }

    return true;


  }

  static double getProperLifeTime(MCParticle* mc) //in ps
  {
    double beta = mc->getMomentum().Mag() / mc->getEnergy();
    return 1e3 * mc->getLifetime() * sqrt(1 - pow(beta, 2));
  }

  void TagVertexModule::BtagMCVertex(Particle* Breco)
  {

    bool isBreco = false;
    int nReco = 0;

    TVector3 MCTagVert(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                       std::numeric_limits<float>::quiet_NaN());
    int mcPDG = 0;
    double mcTagLifeTime = -1;

    // Array of MC particles
    StoreArray<Belle2::MCParticle> mcParticles("");
    for (int i = 0; i < mcParticles.getEntries(); i++) {
      MCParticle* mc = mcParticles[i];
      if (TMath::Abs(mc->getPDG()) == TMath::Abs(Breco->getPDGCode())) {

        if (m_useMCassociation == "breco") {
          const MCParticle* mcBr = Breco->getRelated<MCParticle>();
          if (mcBr == mc) {
            isBreco = true;
          } else {
            isBreco = false;
          }
        } else {
          if (m_useMCassociation == "internal") isBreco = compBrecoBgen(Breco, mc);
        }
        if (isBreco) {
          m_MCVertReco = mc->getDecayVertex();
          m_MCLifeTimeReco =  getProperLifeTime(mc);
          nReco++;
        } else {
          MCTagVert = mc->getDecayVertex();
          mcTagLifeTime = getProperLifeTime(mc);
          mcPDG = mc->getPDG();
        }
      }
    }


    if (nReco == 2) {
      double dref = 1000;
      for (int i = 0; i < mcParticles.getEntries(); i++) {
        MCParticle* mc = mcParticles[i];
        if (TMath::Abs(mc->getPDG()) == TMath::Abs(Breco->getPDGCode())) {
          double dcalc = (mc->getDecayVertex() - Breco->getVertex()).Mag();
          m_MCVertReco = mc->getDecayVertex();
          m_MCLifeTimeReco  = getProperLifeTime(mc);
          if (dcalc < dref) {
            dref = dcalc;
            MCTagVert = mc->getDecayVertex();
            mcTagLifeTime = getProperLifeTime(mc);
            mcPDG = mc->getPDG();
          } else {
            m_MCVertReco = mc->getDecayVertex();
            m_MCLifeTimeReco  = getProperLifeTime(mc);
          }
        }
      }
    }

    m_MCtagV = MCTagVert;
    m_MCtagLifeTime = mcTagLifeTime;
    m_mcPDG = mcPDG;
  }


  bool TagVertexModule::compBrecoBgen(Particle* Breco, MCParticle* Bgen)
  {

    bool isDecMode = true;

    const std::vector<Belle2::Particle*> recDau = Breco->getDaughters();
    const std::vector<Belle2::MCParticle*> genDau = Bgen->getDaughters();

    if (recDau.size() > 0 && genDau.size() > 0) {
      for (auto dauRec : recDau) {
        bool isDau = false;
        for (auto dauGen : genDau) {
          if (dauGen->getPDG() == dauRec->getPDGCode())
            isDau = compBrecoBgen(dauRec, dauGen) ;
        }
        if (!isDau) isDecMode = false;
      }
    } else {
      if (recDau.size() == 0) { //&& genDau.size()==0){
        if (Bgen->getPDG() != Breco->getPDGCode()) isDecMode = false;;
      } else {isDecMode = false;}
    }

    return isDecMode;
  }

  // STANDARD FIT ALGORITHM
  /* This algorithm basically takes all the tracks coming from the Rest Of Events and send them to perform a multi-track fit
   The option of requestion PXD hits for the tracks can be chosen by the user.
   */
  bool TagVertexModule::getTagTracks_standardAlgorithm(Particle* Breco, int reqPXDHits)
  {
    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();
    if (!roe) return false;
    std::vector<const Particle*> ROEParticles = roe->getChargedParticles(m_roeMaskName, Const::pion.getPDGCode(), false);
    if (ROEParticles.size() == 0) return false;
    std::vector<const Particle*> fitParticles;

    for (auto& ROEParticle : ROEParticles) {
      HitPatternVXD roeTrackPattern = ROEParticle->getTrackFitResult()->getHitPatternVXD();

      if (roeTrackPattern.getNPXDHits() >= reqPXDHits) {
        fitParticles.push_back(ROEParticle);

      }
    }
    if (fitParticles.size() == 0) return false;
    m_tagParticles = fitParticles;

    return true;
  }

  bool TagVertexModule::getParticlesWithoutKS(vector<const Particle*> const&  tagParticles,
                                              vector<ParticleAndWeight>& particleAndWeights,
                                              double massWindowWidth)
  {
    //clear the vector
    if (particleAndWeights.size() > 0)
      B2WARNING("In TagVertexModule::getParticlesWithoutKS, particleAndWeights has non-zero size, risk of memory leak.");
    particleAndWeights.clear();

    ParticleAndWeight particleAndWeight;
    particleAndWeight.mcParticle = 0;
    particleAndWeight.weight = -1111.;


    TLorentzVector mom1;
    TLorentzVector mom2;
    const Particle* particle1;
    const Particle* particle2;
    const double mks(Const::K0Mass);
    double mass;

    // remove tracks from KS
    for (unsigned i = 0; i < tagParticles.size(); ++i) {
      particle1 = tagParticles.at(i);

      if (particle1) mom1 = particle1->get4Vector();
      if (particle1 && !std::isinf(mom1.Mag2()) && !std::isnan(mom1.Mag2())) {

        bool isKsDau = false;
        for (unsigned j = 0; j < tagParticles.size() && !isKsDau; ++j) {
          if (i != j) {
            particle2 = tagParticles.at(j);

            if (particle2) mom2 = particle2->get4Vector();
            if (particle2 && !std::isinf(mom2.Mag2()) && !std::isnan(mom2.Mag2())) {
              mass = (mom1 + mom2).M();
              if (abs(mass - mks) < massWindowWidth) isKsDau = true;
            }
          }
        }
        if (!isKsDau) {
          particleAndWeight.particle = particle1;

          if (m_useMCassociation == "breco" || m_useMCassociation == "internal")
            particleAndWeight.mcParticle = particle1->getRelatedTo<MCParticle>();

          particleAndWeights.push_back(particleAndWeight);
        }
      }
    }

    return true;
  }

  bool TagVertexModule::makeGeneralFit()
  {
    if (m_fitAlgo == "Rave") return makeGeneralFitRave();
    if (m_fitAlgo == "KFit") return makeGeneralFitKFit();

    return false;
  }

  bool TagVertexModule::makeGeneralFitRave()
  {
    // apply constraint
    analysis::RaveSetup::getInstance()->unsetBeamSpot();
    if (m_constraintType != "noConstraint") analysis::RaveSetup::getInstance()->setBeamSpot(m_constraintCenter, m_constraintCov);
    analysis::RaveVertexFitter rFit;

    //feed rave with tracks without Kshorts

    vector<ParticleAndWeight> particleAndWeights;
    getParticlesWithoutKS(m_tagParticles, particleAndWeights);

    for (unsigned int i(0); i < particleAndWeights.size(); ++i) {
      try {
        if (!m_useTruthInFit && !m_useRollBack)
          rFit.addTrack(particleAndWeights.at(i).particle->getTrackFitResult());
        if (m_useTruthInFit && !particleAndWeights.at(i).mcParticle)
          m_fitTruthStatus = 2;
        if (m_useRollBack && !particleAndWeights.at(i).mcParticle)
          m_rollbackStatus = 2;
        if (m_useTruthInFit && particleAndWeights.at(i).mcParticle) {
          TrackFitResult tfr(getTrackWithTrueCoordinates(particleAndWeights.at(i)));
          rFit.addTrack(&tfr);
        }
        if (m_useRollBack && !m_useTruthInFit && particleAndWeights.at(i).mcParticle) {
          TrackFitResult tfr(getTrackWithRollBackCoordinates(particleAndWeights.at(i)));
          rFit.addTrack(&tfr);
        }
      } catch (const rave::CheckedFloatException&) {
        B2ERROR("Exception caught in TagVertexModule::makeGeneralFitRave(): Invalid inputs (nan/inf)?");
      }
    }

    //perform fit

    int isGoodFit(-1);
    try {
      isGoodFit = rFit.fit("avf");
    } catch (const rave::CheckedFloatException&) {
      B2ERROR("Exception caught in TagVertexModule::makeGeneralFitRave(): Invalid inputs (nan/inf)?");
      return false;
    }

    //save the track info for later use
    //Tracks are sorted from highest rave weight to lowest

    unsigned int n(particleAndWeights.size());
    for (unsigned int i(0); i < n && isGoodFit >= 1; ++i)
      particleAndWeights.at(i).weight = rFit.getWeight(i);

    sort(particleAndWeights.begin(), particleAndWeights.end(), compare);

    m_raveParticles.resize(n);
    m_raveWeights.resize(n);
    m_raveMCParticles.resize(n);

    for (unsigned i = 0; i < n; ++i) {
      m_raveParticles.at(i) = particleAndWeights.at(i).particle;
      m_raveMCParticles.at(i) = particleAndWeights.at(i).mcParticle;
      m_raveWeights.at(i) = particleAndWeights.at(i).weight;
    }

    //if the fit is good, save the infos related to the vertex

    if (isGoodFit < 1) return false;

    if (m_constraintType != "noConstraint") {
      TMatrixDSym tubeInv = m_constraintCov;
      tubeInv.Invert();
      TVector3 dTagV = rFit.getPos(0) - m_BeamSpotCenter;
      TVectorD dV(0, 2,
                  dTagV.X(),
                  dTagV.Y(),
                  dTagV.Z(),
                  "END");
      m_tagVChi2IP = tubeInv.Similarity(dV);
    }

    m_tagV = rFit.getPos(0);
    m_tagVErrMatrix.ResizeTo(rFit.getCov(0));
    m_tagVErrMatrix = rFit.getCov(0);
    m_tagVNDF = rFit.getNdf(0);
    m_tagVChi2 = rFit.getChi2(0);

    m_fitPval = rFit.getPValue();

    return true;
  }

  bool TagVertexModule::makeGeneralFitKFit()
  {
    //initialize KFit

    analysis::VertexFitKFit kFit;
    kFit.setMagneticField(m_Bfield);

    // apply constraint

    if (m_constraintType != "noConstraint" && m_constraintType != "tube")
      kFit.setIpProfile(ROOTToCLHEP::getPoint3D(m_constraintCenter), ROOTToCLHEP::getHepSymMatrix(m_constraintCov));

    if (m_constraintType == "tube") {
      CLHEP::HepSymMatrix err(7, 0);

      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
          err[i + 4][j + 4] = m_pvCov(i, j);
        }
      }

      kFit.setIpTubeProfile(
        ROOTToCLHEP::getHepLorentzVector(m_tagMomentum),
        ROOTToCLHEP::getPoint3D(m_constraintCenter),
        err,
        0.);
    }

    //feed KFit with tracks without Kshorts

    vector<ParticleAndWeight> particleAndWeights;
    getParticlesWithoutKS(m_tagParticles, particleAndWeights);
    int nTracksAdded(0);

    for (unsigned int i(0); i < particleAndWeights.size(); ++i) {
      int addedOK(1);

      ParticleAndWeight pawi(particleAndWeights.at(i));

      if (!m_useTruthInFit && !m_useRollBack)
        addedOK = kFit.addParticle(pawi.particle);

      if (m_useTruthInFit && !pawi.mcParticle) {
        addedOK = 1;
        m_fitTruthStatus = 2;
      }

      if (m_useRollBack && !pawi.mcParticle) {
        addedOK = 1;
        m_rollbackStatus = 2;
      }

      if (m_useTruthInFit && pawi.mcParticle) {
        addedOK = kFit.addTrack(
                    ROOTToCLHEP::getHepLorentzVector(pawi.mcParticle->get4Vector()),
                    ROOTToCLHEP::getPoint3D(getTruePoca(pawi)),
                    ROOTToCLHEP::getHepSymMatrix(pawi.particle->getMomentumVertexErrorMatrix()),
                    pawi.particle->getCharge());
      }

      if (m_useRollBack && !m_useTruthInFit && pawi.mcParticle) {
        addedOK = kFit.addTrack(
                    ROOTToCLHEP::getHepLorentzVector(pawi.mcParticle->get4Vector()),
                    ROOTToCLHEP::getPoint3D(getRollBackPoca(pawi)),
                    ROOTToCLHEP::getHepSymMatrix(pawi.particle->getMomentumVertexErrorMatrix()),
                    pawi.particle->getCharge());
      }

      if (addedOK != 0) {
        B2WARNING("TagVertexModule::makeGeneralFitKFit: failed to add a track");
        particleAndWeights.at(i).weight = 0.;
      }

      if (addedOK == 0) {
        nTracksAdded++;
        particleAndWeights.at(i).weight = 1.;
      }
    }

    //perform fit if there are enough tracks

    if ((nTracksAdded < 2 && m_constraintType == "noConstraint") || nTracksAdded < 1)
      return false;

    int isGoodFit(-1);

    isGoodFit = kFit.doFit();

    //save the track info for later use
    //Tracks are sorted by weight, ie pushing the tracks with 0 weight (from KS) to the end of the list

    unsigned int n(particleAndWeights.size());
    sort(particleAndWeights.begin(), particleAndWeights.end(), compare);

    m_raveParticles.resize(n);
    m_raveWeights.resize(n);
    m_raveMCParticles.resize(n);

    for (unsigned int i(0); i < n; ++i) {
      m_raveParticles.at(i) = particleAndWeights.at(i).particle;
      m_raveMCParticles.at(i) = particleAndWeights.at(i).mcParticle;
      m_raveWeights.at(i) = particleAndWeights.at(i).weight;
    }

    //if the fit is good, save the infos related to the vertex

    if (isGoodFit != 0) return false;

    m_tagV = CLHEPToROOT::getTVector3(kFit.getVertex());

    if (m_constraintType != "noConstraint") {
      TMatrixDSym tubeInv = m_constraintCov;
      tubeInv.Invert();
      TVector3 dTagV = m_tagV - m_BeamSpotCenter;
      TVectorD dV(0, 2,
                  dTagV.X(),
                  dTagV.Y(),
                  dTagV.Z(),
                  "END");
      m_tagVChi2IP = tubeInv.Similarity(dV);
    }

    TMatrixDSym errMat(CLHEPToROOT::getTMatrixDSym(kFit.getVertexError()));

    m_tagVErrMatrix.ResizeTo(errMat);
    m_tagVErrMatrix = errMat;
    m_tagVNDF = kFit.getNDF();
    m_tagVChi2 = kFit.getCHIsq();

    m_fitPval = TMath::Prob(m_tagVChi2, m_tagVNDF);

    return true;
  }


  void TagVertexModule::deltaT(Particle* Breco)
  {

    // deltaT and Approximated MCdeltaT

    PCmsLabTransform T;

    TVector3 boost = T.getBoostVector();

    double bg = boost.Mag() / sqrt(1 - boost.Mag2());

    double c = Const::speedOfLight / 1000.; // cm ps-1

    TVector3 dVert = Breco->getVertex() - m_tagV;
    TVector3 MCdVert = m_MCVertReco - m_MCtagV;

    TVector3 boostDir = boost.Unit();
    double dl = dVert.Dot(boostDir);
    double dt = dl / (bg * c);
    double MCdl = MCdVert.Dot(boostDir);
    double MCdt = MCdl / (bg * c);

    m_deltaT = dt;
    m_MCdeltaTapprox = MCdt;

    // MCdeltaT=tauRec-tauTag
    m_MCdeltaT = m_MCLifeTimeReco - m_MCtagLifeTime;
    if (m_MCLifeTimeReco  == -1 || m_MCtagLifeTime == -1)
      m_MCdeltaT =  std::numeric_limits<double>::quiet_NaN();


    // Calculate Delta t error
    double zxB = sqrt(boost.Z() * boost.Z() + boost.X() * boost.X());
    double angleY = atan2(boost.X(), boost.Z());
    double angleX = atan2(boost.Y(), zxB);
    TMatrixD Rot = getRotationMatrixXY(angleX, -angleY);
    TMatrixD RotT = Rot; RotT.T();

    TMatrixD RotErr = RotT * m_tagVErrMatrix * Rot;
    TMatrixD RR = (TMatrixD)Breco->getVertexErrorMatrix();
    TMatrixD RotErrBreco = RotT * RR * Rot;

    double dtErr = sqrt(RotErr(2, 2) + RotErrBreco(2, 2)) / (bg * c);

    m_tagVl = m_tagV.Dot(boostDir);
    m_truthTagVl = m_MCtagV.Dot(boostDir);
    m_tagVlErr = sqrt(RotErr(2, 2));
    m_deltaTErr = dtErr;


    // calculate tagV component and error in the direction orthogonal to the boost

    TVector3 oboost(boostDir.Z(), boostDir.Y(), -boostDir.X());
    double zxOB = sqrt(oboost.Z() * oboost.Z() + oboost.X() * oboost.X());
    double angleOY = atan2(oboost.X(), oboost.Z());
    double angleOX = atan2(oboost.Y(), zxOB);
    TMatrixD oRot = getRotationMatrixXY(angleOX, -angleOY);
    TMatrixD oRotT = oRot; oRotT.T();

    TMatrixD oRotErr = oRotT * m_tagVErrMatrix * oRot;

    m_tagVol = m_tagV.Dot(oboost);
    m_truthTagVol = m_MCtagV.Dot(oboost);
    m_tagVolErr = sqrt(oRotErr(2, 2));

  }

  bool TagVertexModule::doVertexFitForBTube(Particle* mother)
  {
    //Here rave is used to find the upsilon(4S) vtx as the intersection
    //between the mother B trajectory and the beam spot

    analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_BeamSpotCov);

    analysis::RaveVertexFitter rsg;
    rsg.addTrack(mother);
    int nvert = rsg.fit("avf");

    if (nvert == 1) {
      rsg.updateDaughters();
    } else {return false;}
    return true;
  }

  TrackFitResult TagVertexModule::getTrackWithTrueCoordinates(ParticleAndWeight const& paw)
  {
    if (!paw.mcParticle) {
      B2ERROR("In TagVertexModule::getTrackWithTrueCoordinate: no MC particle set");
      return TrackFitResult();
    }

    const TrackFitResult* tfr(paw.particle->getTrackFitResult());

    return TrackFitResult(getTruePoca(paw),
                          paw.mcParticle->getMomentum(),
                          tfr->getCovariance6(),
                          tfr->getChargeSign(),
                          tfr->getParticleType(),
                          tfr->getPValue(),
                          m_Bfield, 0, 0);
  }

  TVector3 TagVertexModule::getTruePoca(ParticleAndWeight const& paw)
  {
    if (!paw.mcParticle) {
      B2ERROR("In TagVertexModule::getTruePoca: no MC particle set");
      return TVector3(0., 0., 0.);
    }

    return DistanceTools::poca(paw.mcParticle->getProductionVertex(),
                               paw.mcParticle->getMomentum(),
                               paw.particle->getTrackFitResult()->getPosition());

  }

  TrackFitResult TagVertexModule::getTrackWithRollBackCoordinates(ParticleAndWeight const& paw)
  {
    const TrackFitResult* tfr(paw.particle->getTrackFitResult());

    return TrackFitResult(getRollBackPoca(paw),
                          tfr->getMomentum(),
                          tfr->getCovariance6(),
                          tfr->getChargeSign(),
                          tfr->getParticleType(),
                          tfr->getPValue(),
                          m_Bfield, 0, 0);
  }

  TVector3 TagVertexModule::getRollBackPoca(ParticleAndWeight const& paw)
  {
    if (!paw.mcParticle) {
      B2ERROR("In TagVertexModule::getTruePoca: no MC particle set");
      return TVector3(0., 0., 0.);
    }

    return paw.particle->getTrackFitResult()->getPosition() - paw.mcParticle->getProductionVertex() + m_MCtagV;

  }

  void TagVertexModule::resetReturnParams()
  {
    m_raveParticles.resize(0);
    m_raveMCParticles.resize(0);
    m_tagParticles.resize(0);
    m_raveWeights.resize(0);

    double quietNaN(std::numeric_limits<double>::quiet_NaN());

    TMatrixDSym nanMatrix(3);
    for (int i(0); i < 3; ++i)
      for (int j(0); j < 3; ++j) nanMatrix(i, j) = quietNaN;

    m_fitPval = quietNaN;
    m_tagV = TVector3(quietNaN, quietNaN, quietNaN);
    m_tagVErrMatrix.ResizeTo(nanMatrix);
    m_tagVErrMatrix = nanMatrix;
    m_MCtagV = TVector3(quietNaN, quietNaN, quietNaN);
    m_MCVertReco = TVector3(quietNaN, quietNaN, quietNaN);
    m_deltaT = quietNaN;
    m_deltaTErr = quietNaN;
    m_MCdeltaT = quietNaN;
    m_constraintCov.ResizeTo(nanMatrix);
    m_constraintCov = nanMatrix;
    m_constraintCenter = TVector3(quietNaN, quietNaN, quietNaN);
    m_tagVl = quietNaN;
    m_truthTagVl = quietNaN;
    m_tagVlErr = quietNaN;
    m_tagVol = quietNaN;
    m_truthTagVol = quietNaN;
    m_tagVolErr = quietNaN;
    m_tagVNDF = quietNaN;
    m_tagVChi2 = quietNaN;
    m_tagVChi2IP = quietNaN;
    m_pvCov.ResizeTo(nanMatrix);
    m_pvCov = nanMatrix;
    m_tagMomentum = TLorentzVector(quietNaN, quietNaN, quietNaN, quietNaN);


  }

  //The following functions are just here to help printing stuff

  std::string TagVertexModule::printVector(TVector3 const& vec)
  {
    std::ostringstream oss;
    int w(14);
    oss << "(" << std::setw(w) << vec[0] << ", " << std::setw(w) << vec[1] << ", " << std::setw(w) << vec[2] << ")" << std::endl;
    return oss.str();
  }

  std::string TagVertexModule::printMatrix(TMatrix const& mat)
  {
    std::ostringstream oss;
    int w(14);
    for (int i(0); i < mat.GetNrows(); ++i) {
      for (int j(0); j < mat.GetNcols(); ++j) {
        oss << std::setw(w) << mat(i, j) << " ";
      }
      oss << endl;
    }
    return oss.str();
  }

  std::string TagVertexModule::printMatrix(TMatrixDSym const& mat)
  {
    std::ostringstream oss;
    int w(14);
    for (int i(0); i < mat.GetNrows(); ++i) {
      for (int j(0); j < mat.GetNcols(); ++j) {
        oss << std::setw(w) << mat(i, j) << " ";
      }
      oss << endl;
    }
    return oss.str();
  }


} // end Belle2 namespace
