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

// vertex fitting
#include <analysis/VertexFitting/KFit/VertexFitKFit.h>

// msdt dataobject
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/HitPatternVXD.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

#include <TVector.h>


using namespace std;

namespace Belle2 {




  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------
  REG_MODULE(TagVertex)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TagVertexModule::TagVertexModule() : Module(),
    m_Bfield(0), m_fitTruthStatus(0), m_fitPval(0), m_mcPDG(0), m_deltaT(0), m_deltaTErr(0), m_MCdeltaT(0), m_shiftZ(0), m_FitType(0),
    m_tagVl(0),
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

    for (unsigned i = 0; i < plist->getListSize(); i++) {
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
        } else {
          ver->setTagVertex(m_tagV);
          ver->setTagVertexPval(-1.);
          ver->setDeltaT(m_deltaT);
          ver->setDeltaTErr(m_deltaTErr);
          ver->setMCTagVertex(m_MCtagV);
          ver->setMCTagBFlavor(0.);
          ver->setMCDeltaT(m_MCdeltaT);
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
    TMatrixDSym errFinal(3); errFinal += Breco->getVertexErrorMatrix(); errFinal += RerrMatrix;

    // end simpler version

    // TODO : to be deveoped the extraction of the momentum from the rave fitted track

    // start rotation of the error matrix
    TLorentzVector v4Final(pFinal, Breco->getPDGMass());
    PCmsLabTransform T;
    TLorentzVector vec = T.rotateLabToCms() * v4Final;

    double thetar = v4Final.Theta();
    double phir = v4Final.Phi();

    double str = TMath::Sin(-1 * thetar);
    double ctr = TMath::Cos(-1 * thetar);
    double spr = TMath::Sin(-1 * phir);
    double cpr = TMath::Cos(-1 * phir);

    TMatrix r1z(3, 3);  r1z(2, 2) = 1;
    r1z(0, 0) = cpr; r1z(0, 1) = spr;
    r1z(1, 0) = -1 * spr; r1z(1, 1) = cpr;

    TMatrix r1y(3, 3);  r1y(1, 1) = 1;
    r1y(0, 0) = ctr; r1y(0, 2) = -1 * str;
    r1y(2, 0) = str; r1y(2, 2) = ctr;


    TMatrix r1(3, 3);  r1.Mult(r1z, r1y);
    TMatrix r1t(3, 3); r1t.Transpose(r1);

    TMatrix TubeZPart(3, 3);  TubeZPart.Mult(r1t, errFinal);
    TMatrix TubeZ(3, 3); TubeZ.Mult(TubeZPart, r1);

    TubeZ(2, 2) = cut;
    TubeZ(2, 0) = 0; TubeZ(0, 2) = 0;
    TubeZ(2, 1) = 0; TubeZ(1, 2) = 0;

    vec.SetX(-1 * vec.X());
    vec.SetY(-1 * vec.Y());
    vec.SetZ(-1 * vec.Z());

    TLorentzVector vecLab = T.rotateCmsToLab() * vec;
    double theta = vecLab.Theta();
    double phi = vecLab.Phi();

    double st = TMath::Sin(theta);
    double ct = TMath::Cos(theta);
    double sp = TMath::Sin(phi);
    double cp = TMath::Cos(phi);

    TMatrix r2z(3, 3);  r2z(2, 2) = 1;
    r2z(0, 0) = cp; r2z(0, 1) = sp;
    r2z(1, 0) = -1 * sp; r2z(1, 1) = cp;

    TMatrix r2y(3, 3);  r2y(1, 1) = 1;
    r2y(0, 0) = ct; r2y(0, 2) = -1 * st;
    r2y(2, 0) = st; r2y(2, 2) = ct;

    TMatrix r2(3, 3);  r2.Mult(r2y, r2z);
    TMatrix r2t(3, 3); r2t.Transpose(r2);

    TMatrix TubePart(3, 3);  TubePart.Mult(r2t, TubeZ);
    TMatrix Tube(3, 3); Tube.Mult(TubePart, r2);

    m_constraintCov.ResizeTo(3, 3);

    m_constraintCov(0, 0) = Tube(0, 0);  m_constraintCov(0, 1) = Tube(0, 1);  m_constraintCov(0, 2) = Tube(0, 2);
    m_constraintCov(1, 0) = Tube(1, 0);  m_constraintCov(1, 1) = Tube(1, 1);  m_constraintCov(1, 2) = Tube(1, 2);
    m_constraintCov(2, 0) = Tube(2, 0);  m_constraintCov(2, 1) = Tube(2, 1);  m_constraintCov(2, 2) = Tube(2, 2);

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
    TLorentzVector vecNew(-1 * vec.Px(), -1 * vec.Py(), -1 * vec.Pz(), vec.E());
    TLorentzVector v4FinalNew = T.rotateCmsToLab() * vecNew;

    //To creat the B tube, strategy is: take the primary vtx cov matrix, and add to it a cov
    //matrix corresponding to an very big error in the direction of the B tag

    TMatrixFSym pv = tubecreatorBCopy.getVertexErrorMatrix();

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

    double theta = v4FinalNew.Theta();
    double phi = v4FinalNew.Phi();

    double st = TMath::Sin(theta);
    double ct = TMath::Cos(theta);
    double sp = TMath::Sin(phi);
    double cp = TMath::Cos(phi);

    TMatrix r2z(3, 3);  r2z(2, 2) = 1;
    r2z(0, 0) = cp; r2z(0, 1) = -1 * sp;
    r2z(1, 0) = sp; r2z(1, 1) = cp;

    TMatrix r2y(3, 3);  r2y(1, 1) = 1;
    r2y(0, 0) = ct; r2y(0, 2) = st;
    r2y(2, 0) = -1 * st; r2y(2, 2) = ct;

    TMatrix r2(3, 3);  r2.Mult(r2z, r2y);
    TMatrix r2t(3, 3); r2t.Transpose(r2);


    //make a long error matrix along BTag direction

    TMatrix longerror(3, 3); longerror(2, 2) = cut * cut;
    TMatrix longerror_temp(3, 3); longerror_temp.Mult(r2, longerror);
    TMatrix longerrorRotated(3, 3); longerrorRotated.Mult(longerror_temp, r2t);

    //pvNew will correspond to the covariance matrix of the B tube

    TMatrix pvNew(3, 3);
    pvNew += pv;
    pvNew += longerrorRotated;

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

    for (int i(0); i < 3; ++i) {
      for (int j(0); j < 3; ++j) {
        m_constraintCov(i, j) = pvNew(i, j);
      }
    }

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

    m_constraintCov.ResizeTo(3, 3);

    m_constraintCov(0, 0) = Tube(0, 0); m_constraintCov(0, 1) = Tube(0, 1);  m_constraintCov(0, 2) = Tube(0, 2);
    m_constraintCov(1, 0) = Tube(1, 0); m_constraintCov(1, 1) = Tube(1, 1);  m_constraintCov(1, 2) = Tube(1, 2);
    m_constraintCov(2, 0) = Tube(2, 0); m_constraintCov(2, 1) = Tube(2, 1);  m_constraintCov(2, 2) = Tube(2, 2);


    m_constraintCenter = m_BeamSpotCenter; // Standard algorithm needs no shift

    // The constraint used in the Single Track Fit needs to be shifted in the boost direction.

    if (shiftAlongBoost > -1000) {
      float boostAngle = TMath::ATan(float(boostDir[0]) / boostDir[2]); // boost angle with respect from Z

      m_constraintCenter = m_BeamSpotCenter +
                           TVector3(shiftAlongBoost * TMath::Sin(boostAngle), 0., shiftAlongBoost * TMath::Cos(boostAngle)); // boost in the XZ plane
    }

    return true;


  }


  void TagVertexModule::BtagMCVertex(Particle* Breco)
  {

    bool isBreco = false;
    int nReco = 0;

    TVector3 MCTagVert(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                       std::numeric_limits<float>::quiet_NaN());
    int mcPDG = 0;

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
          nReco++;
        } else {
          MCTagVert = mc->getDecayVertex();
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
          if (dcalc < dref) {
            dref = dcalc;
            MCTagVert = mc->getDecayVertex();
            mcPDG = mc->getPDG();
          } else {
            m_MCVertReco = mc->getDecayVertex();
          }
        }
      }
    }

    m_MCtagV = MCTagVert;
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
    for (unsigned int i(0); i < tagParticles.size(); ++i) {
      particle1 = tagParticles.at(i);

      if (particle1) mom1 = particle1->get4Vector();
      if (particle1 && !std::isinf(mom1.Mag2()) && !std::isnan(mom1.Mag2())) {

        bool isKsDau = false;
        for (unsigned int j(0); j < tagParticles.size() && !isKsDau; ++j) {
          if (i != j) {
            particle2 = tagParticles.at(j);

            if (particle2) mom2 = particle2->get4Vector();
            if (particle2 && !std::isinf(mom2.Mag2()) && !std::isnan(mom2.Mag2())) {
              mass = (mom1 + mom2).M();
              if (TMath::Abs(mass - mks) < massWindowWidth) isKsDau = true;
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
        if (!m_useTruthInFit)
          rFit.addTrack(particleAndWeights.at(i).particle->getTrackFitResult());
        if (m_useTruthInFit && !particleAndWeights.at(i).mcParticle)
          m_fitTruthStatus = 2;
        if (m_useTruthInFit && particleAndWeights.at(i).mcParticle) {
          TrackFitResult tfr(getTrackWithTrueCoordinates(particleAndWeights.at(i)));
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

    for (unsigned int i(0); i < n; ++i) {
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

      if (!m_useTruthInFit)
        addedOK = kFit.addParticle(pawi.particle);

      if (m_useTruthInFit && !pawi.mcParticle) {
        addedOK = 1;
        m_fitTruthStatus = 2;
      }

      if (m_useTruthInFit && pawi.mcParticle) {
        addedOK = kFit.addTrack(
                    ROOTToCLHEP::getHepLorentzVector(pawi.mcParticle->get4Vector()),
                    ROOTToCLHEP::getPoint3D(getTruePoca(pawi)),
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

    PCmsLabTransform T;

    TVector3 boost = T.getBoostVector();

    double bg = boost.Mag() / TMath::Sqrt(1 - boost.Mag2());

    double c = Const::speedOfLight / 1000.; // cm ps-1

    TVector3 dVert = Breco->getVertex() - m_tagV;
    TVector3 MCdVert = m_MCVertReco - m_MCtagV;

    TVector3 boostDir = boost.Unit();
    double dl = dVert.Dot(boostDir);
    double dt = dl / (bg * c);
    double MCdl = MCdVert.Dot(boostDir);
    double MCdt = MCdl / (bg * c);


    m_deltaT = dt;
    m_MCdeltaT = MCdt;


    // Calculate Delta t error

    double cy = boost.Z() / TMath::Sqrt(boost.Z() * boost.Z() + boost.X() * boost.X());
    double sy = boost.X() / TMath::Sqrt(boost.Z() * boost.Z() + boost.X() * boost.X());
    double cx = TMath::Sqrt(boost.Z() * boost.Z() + boost.X() * boost.X()) / boost.Mag();
    double sx = boost.Y() / boost.Mag();

    TMatrixD RotY(3, 3);
    RotY(0, 0) = cy;  RotY(0, 1) = 0;   RotY(0, 2) = -sy;
    RotY(1, 0) = 0;   RotY(1, 1) = 1;   RotY(1, 2) = 0;
    RotY(2, 0) = sy;  RotY(2, 1) = 0;   RotY(2, 2) = cy;

    TMatrixD RotX(3, 3);
    RotX(0, 0) = 1;   RotX(0, 1) = 0;   RotX(0, 2) = 0;
    RotX(1, 0) = 0;   RotX(1, 1) = cx;  RotX(1, 2) = -sx;
    RotX(2, 0) = 0;   RotX(2, 1) = sx;  RotX(2, 2) = cx;

    TMatrixD Rot = RotY * RotX;
    TMatrixD RotCopy = Rot;
    TMatrixD RotInv = Rot.Invert();

    TMatrixD RotErr = RotInv * m_tagVErrMatrix * RotCopy;
    TMatrixD RR = (TMatrixD)Breco->getVertexErrorMatrix();
    TMatrixD RotErrBreco = RotInv * RR * RotCopy;

    double dtErr = sqrt(RotErr(2, 2) + RotErrBreco(2, 2)) / (bg * c);

    m_tagVl = m_tagV.Dot(boostDir);
    m_truthTagVl = m_MCtagV.Dot(boostDir);
    m_tagVlErr = sqrt(RotErr(2, 2));
    m_deltaTErr = dtErr;


    // calculate tagV component and error in the direction orthogonal to the boost

    TVector3 oboost(boostDir.Z(), boostDir.Y(), -1 * boostDir.X());
    double ocy = oboost.Z() / TMath::Sqrt(oboost.Z() * oboost.Z() + oboost.X() * oboost.X());
    double osy = oboost.X() / TMath::Sqrt(oboost.Z() * oboost.Z() + oboost.X() * oboost.X());
    double ocx = TMath::Sqrt(oboost.Z() * oboost.Z() + oboost.X() * oboost.X()) / oboost.Mag();
    double osx = oboost.Y() / oboost.Mag();

    TMatrixD oRotY(3, 3);
    oRotY(0, 0) = ocy;  oRotY(0, 1) = 0;   oRotY(0, 2) = -osy;
    oRotY(1, 0) = 0;    oRotY(1, 1) = 1;   oRotY(1, 2) = 0;
    oRotY(2, 0) = osy;  oRotY(2, 1) = 0;   oRotY(2, 2) = ocy;

    TMatrixD oRotX(3, 3);
    oRotX(0, 0) = 1;   oRotX(0, 1) = 0;    oRotX(0, 2) = 0;
    oRotX(1, 0) = 0;   oRotX(1, 1) = ocx;  oRotX(1, 2) = -osx;
    oRotX(2, 0) = 0;   oRotX(2, 1) = osx;  oRotX(2, 2) = ocx;

    TMatrixD oRot = oRotY * oRotX;
    TMatrixD oRotCopy = oRot;
    TMatrixD oRotInv = oRot.Invert();

    TMatrixD oRotErr = oRotInv * m_tagVErrMatrix * oRotCopy;

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

  std::string TagVertexModule::printMatrix(TMatrixFSym const& mat)
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
