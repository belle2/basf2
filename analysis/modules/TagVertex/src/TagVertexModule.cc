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
    m_Bfield(0), m_fitPval(0), m_mcPDG(0), m_deltaT(0), m_deltaTErr(0), m_MCdeltaT(0), m_MCdeltaTapprox(0),
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
             "Choose how to reconstruct the tracks on the tag side: standard, standard_PXD, singleTrack, singleTrack_PXD",
             string("standard_PXD"));
    addParam("maskName", m_roeMaskName,
             "Choose ROE mask to get particles from ", string(""));
    addParam("askMCInformation", m_MCInfo,
             "TRUE when requesting MC Information from the tracks performing the vertex fit", false);
    addParam("reqPXDHits", m_reqPXDHits,
             "Minium number of PXD hits for a track to be used in the vertex fit", 0);
    addParam("fitAlgorithm", m_fitAlgo,
             "Fitter used for the tag vertex fit: Rave or KFit", string("Rave"));


  }

  void TagVertexModule::initialize()
  {
    // magnetic field
    m_Bfield = BFieldManager::getField(m_BeamSpotCenter).Z() / Unit::T;
    // RAVE setup
    analysis::RaveSetup::initialize(1, m_Bfield);
    B2INFO("TagVertexModule : magnetic field = " << m_Bfield);

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
      B2ERROR("TagVertexModule: invalid fitting algorithm (must be set to either Rave or KFit).");
    //temporary while the one track fit is broken
    if (m_trackFindingType == "singleTrack" || m_trackFindingType == "singleTrack_PXD")
      B2ERROR("TagVertexModule : the singleTrack option is temporarily broken.");
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
          ver->setNTracks(m_tagTracks.size());
          ver->setTagVl(m_tagVl);
          ver->setTruthTagVl(m_truthTagVl);
          ver->setTagVlErr(m_tagVlErr);
          ver->setTagVol(m_tagVol);
          ver->setTruthTagVol(m_truthTagVol);
          ver->setTagVolErr(m_tagVolErr);
          ver->setTagVNDF(m_tagVNDF);
          ver->setTagVChi2(m_tagVChi2);
          ver->setTagVChi2IP(m_tagVChi2IP);
          ver->setVertexFitTracks(m_raveTracks);
          ver->setVertexFitMCParticles(m_raveTracksMCParticles);
          ver->setRaveWeights(m_raveWeights);
          ver->setConstraintType(m_constraintType);
          ver->setConstraintCenter(m_constraintCenter);
          ver->setConstraintCov(m_constraintCov);
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
          ver->setNTracks(m_tagTracks.size());
          ver->setTagVl(m_tagVl);
          ver->setTruthTagVl(m_truthTagVl);
          ver->setTagVlErr(m_tagVlErr);
          ver->setTagVol(m_tagVol);
          ver->setTruthTagVol(m_truthTagVol);
          ver->setTagVolErr(m_tagVolErr);
          ver->setTagVNDF(-1111.);
          ver->setTagVChi2(-1111.);
          ver->setTagVChi2IP(-1111.);
          ver->setVertexFitTracks(m_raveTracks);
          ver->setVertexFitMCParticles(m_raveTracksMCParticles);
          ver->setRaveWeights(m_raveWeights);
          ver->setConstraintType(m_constraintType);
          ver->setConstraintCenter(m_constraintCenter);
          ver->setConstraintCov(m_constraintCov);
        }
      }

    }
    plist->removeParticles(toRemove);

    //free memory allocated by rave. initialize() would be enough, except that we must clean things up before program end...
    analysis::RaveSetup::getInstance()->reset();

  }

  bool TagVertexModule::doVertexFit(Particle* Breco)
  {
    if ((m_trackFindingType == "singleTrack" || m_trackFindingType == "singleTrack_PXD") && m_constraintType == "noConstraint") {
      B2ERROR("TagVertex: not possible to use singleTrack with no constraint");
      return false;
    }

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

    // Each fit algorithm has its own constraint. Therefore, depending on the user's choice, the constraint will change.


    if (m_trackFindingType != "singleTrack" && m_trackFindingType != "singleTrack_PXD"
        && m_constraintType == "IP") ok = findConstraintBoost(cut);
    //tube length here set to 20 * 2 * c tau beta gamma ~= 0.5 cm, should be enough to not bias the decay
    //time but should still help getting rid of some pions from kshorts
    if (m_constraintType == "tube") ok = findConstraintBTube(Breco, 1000 * cut);
    if (m_constraintType == "boost") ok = findConstraintBoost(cut * 200000.);
    if (m_constraintType == "noConstraint") ok = true;
    if (m_constraintType == "breco") ok = findConstraint(Breco, cut * 2000.);
    if ((m_trackFindingType == "singleTrack" || m_trackFindingType == "singleTrack_PXD") && m_constraintType == "IP") {
      // The constraint size is specially squeezzed when using the Single Track Algorithm
      // and shifted along the boost direction
      ok = findConstraintBoost(cut - m_shiftZ, m_shiftZ);

      if (ok && m_MCInfo) FlavorTaggerInfoMCMatch(
          Breco); // When using the STA, the user can ask for MC information from the tracks performing the fit
    }




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

    if (m_trackFindingType == "singleTrack_PXD") {
      ok = getTagTracks_singleTrackAlgorithm(Breco, 1);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 1;
      }
    }
    if ((ok == false || m_fitPval < minPVal) || m_trackFindingType == "singleTrack") {
      ok = getTagTracks_singleTrackAlgorithm(Breco, 0);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 2;
      }
    }

    //if the IP constraint is used and the 1 track fit fails, must re-compute the constraint
    if ((ok == false || m_fitPval < minPVal) && m_constraintType == "IP" && (m_trackFindingType == "singleTrack"
        || m_trackFindingType == "singleTrack_PXD")) {
      ok = findConstraintBoost(cut);
    }

    if ((ok == false || m_fitPval < minPVal) || m_trackFindingType == "standard_PXD") {
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



  // MC MATCHING OF THE TRACKS PERFORMING THE FIT
  /*
   This function extracts MC information from the tracks performing the fit, and needs to be activated by the user.
   The function has been created to investigate the procedence of each track so the user can elaborate cuts and selection criterias
   for the tracks when performing the vertex fit. It also extracts this information for the tracks inside the const RestOfEvent.
   The MAIN interest is to know for a given track whether it comes directly from the B0 or from one of their immediately decaying daughters.
   Finally, the function saves this information in a codified form inside the FlavorTaggerInfo dataObject.
   */
  std::vector< std::pair<const MCParticle*, int> > TagVertexModule::FlavorTaggerInfoMCMatch(Particle* Breco)
  {

    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();
    auto* flavorTagInfo = Breco->getRelatedTo<FlavorTaggerInfo>();

    if (!flavorTagInfo) return std::vector< std::pair<const MCParticle*, int> >(0);

    std::vector<int> FTGoodTracks;
    std::vector<int> FTTotalTracks;

    std::vector<float> momentum = flavorTagInfo->getP();
    std::vector<Particle*> particle = flavorTagInfo->getParticle();
    std::vector<Belle2::Track*> tracksFT = flavorTagInfo->getTracks();


    // FLAVOR TAG MC MATCHING
    /* The loop runs through all the tracks stored in the FlavorTaggerInfo. For each one it tracks back the mother, grandmother, grand grand mother...
       The iteration will go on while the mother is an immediately decaying particle (PDG). The iteration will stop tracking back mothers once it reaches either the B0, or another particle coming from the B0 that does not decay immediately. In the later case it assumes that the correspondent track does not share its production point with the decaying point of the B0 */
    for (unsigned i = 0; i < tracksFT.size(); i++) {

      if (i == 6 || (tracksFT[i] == nullptr)) { // Tracks belonging to the Lambda category or not well reconstructed are discarted
        flavorTagInfo->setIsFromB(0);
        flavorTagInfo->setProdPointResolutionZ(100);
        continue;
      }

      auto* trackMCParticle = particle[i]->getRelatedTo<MCParticle>();
      flavorTagInfo->setMCParticle(trackMCParticle);

      flavorTagInfo->setProdPointResolutionZ((trackMCParticle->getProductionVertex() - m_MCtagV).Mag2());
      MCParticle* trackMCParticleMother = trackMCParticle->getMother();
      int step = 0;
      bool exitFTWhile = false;
      do {
        int PDG = TMath::Abs(trackMCParticleMother->getPDG()); // In order to identify the mother nature, we compare with the PDG code
        std::string motherPDGString = std::to_string(PDG);
        // Particles that do not decay immediately: like pi+ , kaon, D+ ... Those give the code number : 0
        if (PDG == 211 || PDG == 130 || PDG == 310 || PDG == 311 ||
            PDG == 321 || PDG == 411 || PDG == 421 || PDG == 431) {
          flavorTagInfo->setIsFromB(0);
          break;
        }
        // Here Gauge bosons, leptons and special resonances are discarted
        if (motherPDGString.size() == 4 || motherPDGString.size() < 3) {
          flavorTagInfo->setIsFromB(0);
          break; // Exit the while with false
        }
        // If the mother is the B_CP and not the B_tag, we discard this track
        if (trackMCParticleMother->getPDG() == -m_mcPDG) {
          flavorTagInfo->setIsFromB(0);
          break; // Exit the while with false
        }
        // If the first step mother is already the B_tag, the code number is: 5
        if (trackMCParticleMother->getPDG() == m_mcPDG) {
          flavorTagInfo->setIsFromB(5);
          exitFTWhile = true; // Exit the while with true
          break;
        }


        /* If none of the previous work, the mother may be an immediately decaying meson, daugther of the B_tag. Thus, this checks which kind of
          meson is it, and whether the grandmother is a B0. The given code depends on the PDG code of the meson */
        MCParticle* trackMCParticleGrandMother = trackMCParticleMother->getMother();
        if (motherPDGString[motherPDGString.size() - 3] == '1' && trackMCParticleGrandMother->getPDG() == m_mcPDG) {
          flavorTagInfo->setIsFromB(step * 10 + 1);
          exitFTWhile = true; // Exit the while with true
        } else if (motherPDGString[motherPDGString.size() - 3] == '2' && trackMCParticleGrandMother->getPDG() == m_mcPDG) {
          flavorTagInfo->setIsFromB(step * 10 + 2);
          exitFTWhile = true; // Exit the while with true
        } else if (motherPDGString[motherPDGString.size() - 3] == '3' && trackMCParticleGrandMother->getPDG() == m_mcPDG) {
          flavorTagInfo->setIsFromB(step * 10 + 3);
          exitFTWhile = true; // Exit the while with true
        } else if (motherPDGString[motherPDGString.size() - 3] == '4' && trackMCParticleGrandMother->getPDG() == m_mcPDG) {
          flavorTagInfo->setIsFromB(step * 10 + 4);
          exitFTWhile = true; // Exit the while with true

        } else { // If it corresponds to an immediately decaying meson, but this one does not come from the B_tag directly, the iteration carries on
          trackMCParticle = trackMCParticleMother;
          trackMCParticleMother = trackMCParticleGrandMother;
          step++;
          exitFTWhile = false;
        }
      } while (exitFTWhile == false);


      /* In this part of the function, the code finds hoy many tracks from the FlavorTaggerInfo come directly from the B_tag
       (or from immediately decaying daughters, that is, code number > 0. This will be stored in the FlavorTaggerInfo DataObject
       NOTE: Good means coming from the B_tag, and Bad means not coming from the B_tag */
      if (i == 2 || i == 7) continue; // Skip KinLepton and MaxP categories, for they are repeated tracks from other categories
      unsigned totalsize = FTTotalTracks.size();
      unsigned goodsize = FTGoodTracks.size();

      if (goodsize == 0 && exitFTWhile == true) {
        FTGoodTracks.push_back(i);
        FTTotalTracks.push_back(i);
      } else if (totalsize == 0) {
        FTTotalTracks.push_back(i);
      } else {
        for (unsigned j = 0; j < totalsize; j++) {
          if (tracksFT[FTTotalTracks[j]] == tracksFT[i]) {
            break;
          }

          if (momentum[i] == 0) break; // Skip tracks with no momenta, i.e tracks not well reconstructed
          if (j == totalsize - 1) {
            if (exitFTWhile == true) {
              FTTotalTracks.push_back(i);
              FTGoodTracks.push_back(i);
            } else {
              FTTotalTracks.push_back(i);
            }
          }
        }
      }
    }
    if (momentum[6] != 0) FTTotalTracks.push_back(
        6); // Since Lambdas have been discarted manually before, we add that track (if it exist) by hand



    // REST OF EVENT MC MATCHING
    /* In this part of the code the tracks from the RestOfEvent are taken into account. The same MC analysis is performed as
     before with the exact same criteria */
    std::vector<const Track*> ROETracks = roe->getTracks(m_roeMaskName);
    int ROEGoodTracks = 0;
    int ROETotalTracks = ROETracks.size();

    std::pair<MCParticle*, int> emptyPair;
    emptyPair.first = 0;
    emptyPair.second = 2;
    std::vector< std::pair<const MCParticle*, int> > roeTracksAndMatch(ROETotalTracks, emptyPair);

    for (int i = 0; i < ROETotalTracks; i++) {
      auto* roeTrackMCParticle = ROETracks[i]->getRelatedTo<MCParticle>();
      roeTracksAndMatch.at(i).first = roeTrackMCParticle;
      if (roeTrackMCParticle) {
        MCParticle* roeTrackMCParticleMother = roeTrackMCParticle->getMother();
        bool exitROEWhile;
        do {
          int PDG = TMath::Abs(roeTrackMCParticleMother->getPDG());
          std::string motherPDGString = std::to_string(PDG);
          if (PDG == 211 || PDG == 130 || PDG == 310 || PDG == 311 ||
              PDG == 321 || PDG == 411 || PDG == 421
              || PDG == 431) break; //this checks if the mother of the pcl is a secondary (pi, K, D, KS, ...)
          if (motherPDGString.size() == 4 || motherPDGString.size() < 3) break; //checks if the mother is a baryon
          if (roeTrackMCParticleMother->getPDG() == -m_mcPDG) break;
          if (roeTrackMCParticleMother->getPDG() == m_mcPDG) {
            ROEGoodTracks++;
            roeTracksAndMatch.at(i).second = 1;
            break;
          }
          /* If none of the previous work, the mother may be an immediately decaying meson, daugther of the B_tag. Thus, this checks which kind of
             meson is it, and whether the grandmother is a B0. The given code depends on the PDG code of the meson */
          MCParticle* roeTrackMCParticleGrandMother = roeTrackMCParticleMother->getMother();
          if (motherPDGString[motherPDGString.size() - 3] == '1' && roeTrackMCParticleGrandMother->getPDG() == m_mcPDG) {
            ROEGoodTracks++;
            roeTracksAndMatch.at(i).second = 1;
            exitROEWhile = true;
          } else if (motherPDGString[motherPDGString.size() - 3] == '2' && roeTrackMCParticleGrandMother->getPDG() == m_mcPDG) {
            ROEGoodTracks++;
            roeTracksAndMatch.at(i).second = 1;
            exitROEWhile = true;
          } else if (motherPDGString[motherPDGString.size() - 3] == '3' && roeTrackMCParticleGrandMother->getPDG() == m_mcPDG) {
            ROEGoodTracks++;
            roeTracksAndMatch.at(i).second = 1;
            exitROEWhile = true;
          } else if (motherPDGString[motherPDGString.size() - 3] == '4' && roeTrackMCParticleGrandMother->getPDG() == m_mcPDG) {
            ROEGoodTracks++;
            roeTracksAndMatch.at(i).second = 1;
            exitROEWhile = true;

          } else {
            roeTrackMCParticle = roeTrackMCParticleMother;
            roeTrackMCParticleMother = roeTrackMCParticleGrandMother;
            exitROEWhile = false;
          }
        } while (exitROEWhile == false);
      }
    }


    // SET IN THE FT DATAOBJECT THE GOOD/BAD TRACKS INFORMATION FROM FT AND ROE
    /* Finally the MC information is stored in the FlavorTaggerInfo DataObject for future uses. More concretely the number
       of tracks coming directly from the B_tag and immediately decaying daughters (good tracks), and tracks coming from any
       other intermediate particle (bad track) */
    flavorTagInfo->setGoodTracksROE(ROEGoodTracks);
    flavorTagInfo->setBadTracksROE(ROETotalTracks - ROEGoodTracks);
    flavorTagInfo->setGoodTracksFT(FTGoodTracks.size());
    flavorTagInfo->setBadTracksFT(FTTotalTracks.size() - FTGoodTracks.size());

    if (FTTotalTracks.size() == 0) {
      flavorTagInfo->setGoodTracksPurityFT(-1);
    } else {
      flavorTagInfo->setGoodTracksPurityFT(float(FTGoodTracks.size()) / FTTotalTracks.size());
    }

    if (ROETotalTracks == 0) {
      flavorTagInfo->setGoodTracksPurityROE(-1);
    } else {
      flavorTagInfo->setGoodTracksPurityROE(float(ROEGoodTracks) / ROETotalTracks);
    }

    return roeTracksAndMatch;

  }


  // SINGLE TRACK FIT ALGORITHM
  /* The algorithm basically selects only one track to perform the vertex fit. The first idea was to select all tracks
     coming from the B_tag directly together with the tracks coming from its immediately decaying daughters. It did not work though,
     and therefore it was opted to select only one track.
     Nevertheless there is still some basic cuts applied to all tracks before performing the "one track selection", as there is
     still room for improvement, trying to take more than one track if possible. That is not implemented now.
     */
  bool TagVertexModule::getTagTracks_singleTrackAlgorithm(Particle* Breco, int reqPXDHits)
  {
    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();
    std::vector<const Track*> fitTracks; // Vector of track that will be returned after the selection. Now it must contain only 1

    auto* flavorTagInfo = Breco->getRelatedTo<FlavorTaggerInfo>();
    if (!flavorTagInfo) return false;
    std::vector<const Track*> ROETracks = roe->getTracks(m_roeMaskName);
    std::vector<float> listMomentum = flavorTagInfo->getP(); // Momentum of the tracks
    std::vector<float> listTargetP = flavorTagInfo->getTargProb(); // Probability of a track to come directly from B_tag
    std::vector<float> listCategoryP = flavorTagInfo->getCatProb(); // Probability of a track to belong to a given category
    std::vector<int> listTracks(8);
    std::vector<Belle2::Track*> originalTracks = flavorTagInfo->getTracks();
    std::vector<Particle*> listParticle = flavorTagInfo->getParticle();
    std::vector<std::string> categories = flavorTagInfo->getCategories();

    if (ROETracks.size() == 0) return false;

    if (m_MCInfo == 0) {
      flavorTagInfo->setGoodTracksROE(0);
      flavorTagInfo->setBadTracksROE(0);
      flavorTagInfo->setGoodTracksFT(0);
      flavorTagInfo->setBadTracksFT(0);
      for (unsigned i = 0; i < listTracks.size(); i++) {
        flavorTagInfo->setProdPointResolutionZ(0);
        flavorTagInfo->setIsFromB(0);
      }
    }

    // Obtain the impact parameters of the tracks, D0 and Z0. Need the result of the track Fit.
    Const::ChargedStable constArray[8] = {Const::electron, Const::muon, Const::muon, Const::kaon,
                                          Const::pion, Const::pion, Const::kaon, Const::muon
                                         };

    for (unsigned i = 0; i < listCategoryP.size(); i++) {
      if (i ==  6 || (originalTracks[i] == nullptr)) { // Skip Lambdas and non-reconstructed tracks

        flavorTagInfo->setD0(1.0); // Giving by hand 1cm is more than enough to make Lambdas discardable
        flavorTagInfo->setZ0(1.0);
        continue;
      }
      float D0, Z0;
      D0 = originalTracks[i]->getTrackFitResultWithClosestMass(constArray[i])->getD0();
      Z0 = originalTracks[i]->getTrackFitResultWithClosestMass(constArray[i])->getZ0();
      flavorTagInfo->setD0(D0); // Save them on the FlavorTaggerInfo
      flavorTagInfo->setZ0(Z0);
    }
    std::vector<float> listZ0 = flavorTagInfo->getZ0();
    std::vector<float> listD0 = flavorTagInfo->getD0();

    // Save in a vector the hits left by each track in the Pixel Vertex Detector. This will be useful when requesting PXD hits.
    std::vector<int> listNPXDHits(listParticle.size());
    for (unsigned i = 0; i < listParticle.size(); i++) {
      listNPXDHits[i] = int(Variable::trackNPXDHits(listParticle[i]));
    }

    // Here the program keeps track of the tracks that are repeated inside the FlavorTaggerInfo
    int nonRepeated = 1;
    for (unsigned i = 0; i < listTracks.size(); i++) {
      bool repeatedTrack = false;
      for (int j = i - 1; j >= 0; j--) {
        if (originalTracks.at(i) == originalTracks.at(j)) {
          repeatedTrack = true;
          listTracks.at(i) = listTracks.at(j); // If repeated, assign the same number for both tracks
          break;
        }
      }
      if (repeatedTrack == true) continue;
      listTracks.at(i) = nonRepeated; // Assign different numbers for different tracks
      nonRepeated++;
    }

    // Basic cut. Impact parameter needs to be small.
    for (unsigned i = 0; i < listTracks.size(); i++) {
      if ((listZ0.at(i) > 0.1 || listD0.at(i) > 0.1) && listTracks.at(i) != 0) eliminateTrack(listTracks, i);
    }

    B2DEBUG(10, "Required PXD hits " << reqPXDHits);
    for (unsigned i = 0; i < listTracks.size(); i++) {
      if (listNPXDHits[i] < reqPXDHits) {
        B2DEBUG(10, "Track " << i << " eliminated with pxd hits " << listNPXDHits[i]);
        eliminateTrack(listTracks, i);
      }
    }

    // Residual cut from the previous algorithm. Used to give good results discarding secondary tracks. Could be more useful for future non-single track algorithms.
    for (unsigned i = 0; i < listTracks.size(); i++) {
      for (int j = 4; j > 1 ; j--) {
        if (((TMath::Abs(listD0[j]) - TMath::Abs(listD0[i])) < -0.25
             || (TMath::Abs(listZ0[j]) - TMath::Abs(listZ0[i])) < -0.25) && listTracks[i] != 0) {
          eliminateTrack(listTracks, i);
        } else if (((TMath::Abs(listD0[j]) - TMath::Abs(listD0[i])) > 0.25
                    || (TMath::Abs(listZ0[j]) - TMath::Abs(listZ0[i])) > 0.25) && listTracks[j] != 0) {
          eliminateTrack(listTracks, j);
        }
      }
    }

    // SINGLE TRACK SELECTION
    /* Here the code selects only one track to perform the Single Track Fit. Up to now 3 conditions has been implemented for the chosen track to be taken as primary:
       - Maximum momentum
       - High Target Probability
       - High Category Probability
       The last two parameters can be tunned to make the criteria more or less restrictive. The values written here are the standard ones.
       The conditions have been taken only for the Muon and Electron categories. At this moment (Aug 2015) the other categories still are not very easily filtered. A deep MC study confirms it.
       */

    float maxP = listMomentum[7];
    float minTargetProb = 0.2;
    float minCategoryProb = 0.2;

    if (listMomentum[1] == maxP && listTargetP[1] > minTargetProb && listCategoryP[1] > minCategoryProb && listTracks[1] != 0) {
      fitTracks.push_back(originalTracks[1]);
      m_tagTracks = fitTracks;
    } else if (listMomentum[0] == maxP && listTargetP[0] > minTargetProb && listCategoryP[0] > minCategoryProb && listTracks[0] != 0) {
      fitTracks.push_back(originalTracks[0]);
      m_tagTracks = fitTracks;
    } else { // When no single track is available, return false and try with other algorithm.
      return false;
    }

    return true;

  }

  // This function puts a 0 in the position of listTracks where is placed the eliminated track.
  // It has been specially useful when using a track elimination algorithm, instead of a track selection
  void TagVertexModule::eliminateTrack(std::vector<int>& listTracks, int trackPosition)
  {
    if (listTracks[trackPosition] == 0) return;
    int toEliminate = listTracks[trackPosition];
    for (int& listTrack : listTracks) {
      if (listTrack == toEliminate) {
        listTrack = 0;
      }
    }
  }

  // STANDARD FIT ALGORITHM
  /* This algorithm basically takes all the tracks coming from the Rest Of Events and send them to perform a multi-track fit
   The option of requestion PXD hits for the tracks can be chosen by the user.
   */
  bool TagVertexModule::getTagTracks_standardAlgorithm(Particle* Breco, int reqPXDHits)
  {
    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();
    if (!roe) return false;
    std::vector<const Track*> ROETracks = roe->getTracks(m_roeMaskName);
    if (ROETracks.size() == 0) return false;
    std::vector<const Track*> fitTracks;
    for (auto& ROETrack : ROETracks) {
      // TODO: this will always return something (so not nullptr) contrary to the previous method
      // used here. This line can be removed as soon as the multi hypothesis fitting method
      // has been properly established
      if (!ROETrack->getTrackFitResultWithClosestMass(Const::pion)) {
        continue;
      }
      HitPatternVXD roeTrackPattern = ROETrack->getTrackFitResultWithClosestMass(Const::pion)->getHitPatternVXD();

      if (roeTrackPattern.getNPXDHits() >= reqPXDHits) {
        fitTracks.push_back(ROETrack);

      }
    }
    if (fitTracks.size() == 0) return false;
    m_tagTracks = fitTracks;

    return true;
  }

  bool TagVertexModule::getTracksWithoutKS(vector<const Track*> const&  tagTracks, vector<TrackAndWeight>& trackAndWeights)
  {
    //clear the vector
    if (trackAndWeights.size() > 0)
      B2WARNING("In TagVertexModule::getTracksWithoutKS, trackAndWeights has non-zero size, risk of memory leak.");
    trackAndWeights.clear();

    TrackAndWeight trackAndWeight;
    trackAndWeight.mcParticle = 0;
    trackAndWeight.weight = -1111.;

    // Mpi &&  MKs
    const double mpi = Const::pionMass;
    const double mks = Const::K0Mass;
    double Mass = 0.0;
    // remove tracks from KS
    for (unsigned int i = 0; i < tagTracks.size(); i++) {
      const Track* trak1 = tagTracks[i];
      const TrackFitResult* trak1Res = nullptr;
      if (trak1) trak1Res = trak1->getTrackFitResultWithClosestMass(Const::pion);
      TVector3 mom1;
      if (trak1Res) mom1 = trak1Res->getMomentum();
      if (std::isinf(mom1.Mag2()) or std::isnan(mom1.Mag2())) continue;
      if (!trak1Res) continue;

      bool isKsDau = false;
      for (unsigned int j = 0; j < tagTracks.size() && !isKsDau; j++) {
        if (i != j) {
          const Track* trak2 = tagTracks[j];
          const TrackFitResult* trak2Res = nullptr;

          if (trak2) trak2Res = trak2->getTrackFitResultWithClosestMass(Const::pion);

          TVector3 mom2;
          if (trak2Res) mom2 = trak2Res->getMomentum();
          if (std::isinf(mom2.Mag2()) or std::isnan(mom2.Mag2())) continue;
          if (!trak2Res) continue;

          double Mass2 = TMath::Power(TMath::Sqrt(mom1.Mag2() + mpi * mpi) + TMath::Sqrt(mom2.Mag2() + mpi * mpi), 2)
                         - (mom1 + mom2).Mag2();
          Mass = TMath::Sqrt(Mass2);
          if (TMath::Abs(Mass - mks) < 0.01) isKsDau = true;
        }

      }
      if (!isKsDau) {
        trackAndWeight.track = trak1Res;

        if (m_useMCassociation == "breco" || m_useMCassociation == "internal")
          trackAndWeight.mcParticle = trak1->getRelatedTo<MCParticle>();

        trackAndWeights.push_back(trackAndWeight);
      }
    }

    return true;
  }

  bool TagVertexModule::makeGeneralFit()
  {
    if (m_fitAlgo == "Rave") return  makeGeneralFitRave();
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

    vector<TrackAndWeight> trackAndWeights;
    getTracksWithoutKS(m_tagTracks, trackAndWeights);

    for (unsigned int i(0); i < trackAndWeights.size(); ++i) {
      try {
        rFit.addTrack(trackAndWeights.at(i).track); // Temporal fix: some mom go to Inf
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

    unsigned int n(trackAndWeights.size());
    for (unsigned int i(0); i < n && isGoodFit >= 1; ++i)
      trackAndWeights.at(i).weight = rFit.getWeight(i);


    sort(trackAndWeights.begin(), trackAndWeights.end(), compare);

    m_raveTracks.resize(n);
    m_raveWeights.resize(n);
    m_raveTracksMCParticles.resize(n);

    for (unsigned int i(0); i < n; ++i) {
      m_raveTracks.at(i) = trackAndWeights.at(i).track;
      m_raveTracksMCParticles.at(i) = trackAndWeights.at(i).mcParticle;
      m_raveWeights.at(i) = trackAndWeights.at(i).weight;
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
    //For this, we need to construct a particle from the trackfit result as
    //KFit needs the complete 7X7 cov matrix (maybe there is a nicer solution)

    vector<TrackAndWeight> trackAndWeights;
    getTracksWithoutKS(m_tagTracks, trackAndWeights);
    const int dummyIndex(0);
    int nTracksAdded(0);

    for (unsigned int i(0); i < trackAndWeights.size(); ++i) {
      const TrackFitResult* trackRes(NULL);
      trackRes = trackAndWeights.at(i).track;

      int addedOK;
      Particle particle(dummyIndex, trackRes, Const::ChargedStable(211), Const::ChargedStable(211));
      addedOK = kFit.addParticle(&particle);

      if (addedOK != 0) {
        B2WARNING("TagVertexModule::makeGeneralFitKFit: failed to add a track");
        trackAndWeights.at(i).weight = 0.;
      }

      if (addedOK == 0) {
        nTracksAdded++;
        trackAndWeights.at(i).weight = 1.;
      }
    }

    //perform fit if there are enough tracks

    if ((nTracksAdded < 2 && m_constraintType == "noConstraint") || nTracksAdded < 1)
      return false;

    int isGoodFit(-1);

    isGoodFit = kFit.doFit();

    //save the track info for later use
    //Tracks are sorted by weight, ie pushing the tracks with 0 weight (from KS) to the end of the list

    unsigned int n(trackAndWeights.size());
    sort(trackAndWeights.begin(), trackAndWeights.end(), compare);

    m_raveTracks.resize(n);
    m_raveWeights.resize(n);
    m_raveTracksMCParticles.resize(n);

    for (unsigned int i(0); i < n; ++i) {
      m_raveTracks.at(i) = trackAndWeights.at(i).track;
      m_raveTracksMCParticles.at(i) = trackAndWeights.at(i).mcParticle;
      m_raveWeights.at(i) = trackAndWeights.at(i).weight;
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
    m_MCdeltaTapprox = MCdt;

    // MCdeltaT=tauRec-tauTag
    m_MCdeltaT = m_MCLifeTimeReco - m_MCtagLifeTime;
    if (m_MCLifeTimeReco  == -1 || m_MCtagLifeTime == -1)
      m_MCdeltaT =  std::numeric_limits<double>::quiet_NaN();

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
