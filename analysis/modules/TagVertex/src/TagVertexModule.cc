/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/TagVertex/TagVertexModule.h>

//to help printing out stuff
#include<sstream>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/RestOfEvent.h>
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

// mdst dataobject
#include <mdst/dataobjects/HitPatternVXD.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

#include <TVector.h>
#include <TRotation.h>


using namespace std;

namespace Belle2 {

  /** shortcut for NaN of double type */
  static const double    realNaN = std::numeric_limits<double>::quiet_NaN();
  /** vector with NaN entries */
  static const TVector3  vecNaN(realNaN, realNaN, realNaN);
  /** 3x3 matrix with NaN entries */
  static const TMatrixDSym matNaN(3, (double [])
  {
    realNaN, realNaN, realNaN,
             realNaN, realNaN, realNaN,
             realNaN, realNaN, realNaN
  });

  // import tools from RotationTools.h
  using RotationTools::rotateTensor;
  using RotationTools::rotateTensorInv;
  using RotationTools::toSymMatrix;
  using RotationTools::toVec;
  using RotationTools::getUnitOrthogonal;

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------
  REG_MODULE(TagVertex)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TagVertexModule::TagVertexModule() : Module(),
    m_Bfield(0), m_fitTruthStatus(0), m_rollbackStatus(0), m_fitPval(0), m_mcTagLifeTime(-1), m_mcPDG(0), m_mcLifeTimeReco(-1),
    m_deltaT(0), m_deltaTErr(0), m_mcDeltaTau(0), m_mcDeltaT(0),
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
    addParam("askMCInformation", m_mcInfo,
             "TRUE when requesting MC Information from the tracks performing the vertex fit", false);
    addParam("reqPXDHits", m_reqPXDHits,
             "Minimum number of PXD hits for a track to be used in the vertex fit", 0);
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

    //input
    StoreArray<Particle>().isRequired();
    m_plist.isRequired(m_listName);
    // output
    m_verArray.registerInDataStore();
    StoreArray<Particle>().registerRelationTo(m_verArray);
    //check if the fitting algorithm name is set correctly
    if (m_fitAlgo != "Rave" && m_fitAlgo != "KFit")
      B2FATAL("TagVertexModule: invalid fitting algorithm (must be set to either Rave or KFit).");
    if (m_useRollBack && m_useTruthInFit)
      B2FATAL("TagVertexModule: invalid fitting option (useRollBack and useTruthInFit cannot be simultaneously set to true).");
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
    if (!m_plist) {
      B2ERROR("TagVertexModule: ParticleList " << m_listName << " not found");
      return;
    }

    // output
    analysis::RaveSetup::initialize(1, m_Bfield);

    std::vector<unsigned int> toRemove;

    for (unsigned i = 0; i < m_plist->getListSize(); ++i) {
      resetReturnParams();

      Particle* particle =  m_plist->getParticle(i);
      if (m_useMCassociation == "breco" || m_useMCassociation == "internal") BtagMCVertex(particle);
      bool ok = doVertexFit(particle);
      if (ok) deltaT(particle);

      if ((m_fitPval < m_confidenceLevel && m_confidenceLevel != 0)
          || (m_fitPval <= m_confidenceLevel && m_confidenceLevel == 0)) {
        toRemove.push_back(particle->getArrayIndex());
      } else {
        // save information in the Vertex StoreArray
        TagVertex* ver = m_verArray.appendNew();
        // create relation: Particle <-> Vertex
        particle->addRelationTo(ver);
        // fill Vertex with content
        if (ok) {
          ver->setTagVertex(m_tagV);
          ver->setTagVertexErrMatrix(m_tagVErrMatrix);
          ver->setTagVertexPval(m_fitPval);
          ver->setDeltaT(m_deltaT);
          ver->setDeltaTErr(m_deltaTErr);
          ver->setMCTagVertex(m_mcTagV);
          ver->setMCTagBFlavor(m_mcPDG);
          ver->setMCDeltaTau(m_mcDeltaTau);
          ver->setMCDeltaT(m_mcDeltaT);
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
          ver->setMCTagVertex(m_mcTagV);
          ver->setMCTagBFlavor(0.);
          ver->setMCDeltaTau(m_mcDeltaTau);
          ver->setMCDeltaT(m_mcDeltaT);
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
    m_plist->removeParticles(toRemove);

    //free memory allocated by rave. initialize() would be enough, except that we must clean things up before program end...
    //
    analysis::RaveSetup::getInstance()->reset();
  }

  bool TagVertexModule::doVertexFit(const Particle* Breco)
  {
    //reset the fit truth status in case it was set to 2 in a previous fit

    if (m_useTruthInFit) m_fitTruthStatus = 1;

    //reset the roll back status in case it was set to 2 in a previous fit

    if (m_useRollBack) m_rollbackStatus = 1;

    //set constraint type, reset pVal and B field

    m_fitPval = 1;

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

    double beta = PCmsLabTransform().getBoostVector().Mag();
    double bg = beta / sqrt(1 - beta * beta);

    //TODO: What's the origin of these numbers?
    double cut = 8.717575e-02 * bg;
    m_shiftZ = 4.184436e+02 * bg *  0.0001;

    //tube length here set to 20 * 2 * c tau beta gamma ~= 0.5 cm, should be enough to not bias the decay
    //time but should still help getting rid of some pions from kshorts
    m_constraintCov.ResizeTo(3, 3);
    if (m_constraintType == "IP")         tie(m_constraintCenter, m_constraintCov) = findConstraintBoost(cut);
    else if (m_constraintType == "tube")  tie(m_constraintCenter, m_constraintCov) = findConstraintBTube(Breco, 1000 * cut);
    else if (m_constraintType == "boost") tie(m_constraintCenter, m_constraintCov) = findConstraintBoost(cut * 200000.);
    else if (m_constraintType == "breco") tie(m_constraintCenter, m_constraintCov) = findConstraint(Breco, cut * 2000.);
    else if (m_constraintType == "noConstraint") m_constraintCenter = TVector3(); //zero vector
    else  {
      B2ERROR("TagVertex: Invalid constraintType selected");
      return false;
    }

    if (m_constraintCenter == vecNaN) {
      B2ERROR("TagVertex: No correct fit constraint");
      return false;
    }

    /* Depending on the user's choice, one of the possible algorithms is chosen for the fit. In case the algorithm does not converge, in order to assure
       high efficiency, the next algorithm less restrictive is used. I.e, if standard_PXD does not work, the program tries with standard.
    */

    m_FitType = 0;
    double minPVal = (m_fitAlgo != "KFit") ? 0.001 : 0.;
    bool ok = false;

    if (m_trackFindingType == "standard_PXD") {
      m_tagParticles = getTagTracks_standardAlgorithm(Breco, 1);
      if (m_tagParticles.size() > 0) {
        ok = makeGeneralFit();
        m_FitType = 3;
      }
    }

    if (ok == false || m_fitPval < minPVal || m_trackFindingType == "standard") {
      m_tagParticles = getTagTracks_standardAlgorithm(Breco, m_reqPXDHits);
      ok = m_tagParticles.size() > 0;
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 4;
      }
    }

    if ((ok == false || (m_fitPval <= 0. && m_fitAlgo == "Rave")) && m_constraintType != "noConstraint") {
      tie(m_constraintCenter, m_constraintCov) = findConstraintBoost(cut * 200000.);
      ok = (m_constraintCenter != vecNaN);
      if (ok) {
        m_tagParticles = getTagTracks_standardAlgorithm(Breco, m_reqPXDHits);
        ok = (m_tagParticles.size() > 0);
      }
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 5;
      }
    }

    return ok;
  }


  /**
   * Get vector which is opposite to vIn in CMS ref frame
   * Be aware that input vIn and output are in the Lab frame
   */
  static TLorentzVector flipVector(TLorentzVector vIn)
  {
    TLorentzVector vCMS = PCmsLabTransform::labToCms(vIn);
    vCMS.SetVect(-vCMS.Vect());
    return PCmsLabTransform::cmsToLab(vCMS);
  }


  pair<TVector3, TMatrixDSym> TagVertexModule::findConstraint(const Particle* Breco, double cut) const
  {
    if (Breco->getPValue() < 0.) return make_pair(vecNaN, matNaN);

    TMatrixDSym beamSpotCov(3);
    beamSpotCov = m_beamSpotDB->getCovVertex();

    analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, beamSpotCov);

    double pmag = Breco->getMomentumMagnitude();
    double xmag = (Breco->getVertex() - m_BeamSpotCenter).Mag();


    TMatrixDSym TerrMatrix = Breco->getMomentumVertexErrorMatrix();
    TMatrixDSym PerrMatrix(7);

    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        if (i == j) {
          PerrMatrix(i, j) = (beamSpotCov(i, j) + TerrMatrix(i, j)) * pmag / xmag;
        } else {
          PerrMatrix(i, j) = TerrMatrix(i, j);
        }
        PerrMatrix(i + 4, j + 4) = TerrMatrix(i + 4, j + 4);
      }
    }

    PerrMatrix(3, 3) = 0.;

    //Copy Breco, but use errors as are in PerrMatrix
    Particle* Breco2 = ParticleCopy::copyParticle(Breco);
    Breco2->setMomentumVertexErrorMatrix(PerrMatrix);


    const Particle* BRecoRes = doVertexFitForBTube(Breco2, "kalman");
    if (BRecoRes->getPValue() < 0) return make_pair(vecNaN, matNaN); //problems

    // Overall error matrix
    TMatrixDSym errFinal = TMatrixDSym(Breco->getVertexErrorMatrix() + BRecoRes->getVertexErrorMatrix());

    // TODO : to be developed the extraction of the momentum from the rave fitted track

    // Get expected pBtag 4-momentum using transverse-momentum conservation
    TVector3 BvertDiff = Breco->getVertex() - BRecoRes->getVertex();
    TLorentzVector pBrecEstimate(pmag / BvertDiff.Mag() *  BvertDiff, Breco->getPDGMass());
    TLorentzVector pBtagEstimate = flipVector(pBrecEstimate);

    // rotate err-matrix such that pBrecEstimate goes to eZ
    TMatrixD TubeZ = rotateTensorInv(pBrecEstimate.Vect(), errFinal);

    TubeZ(2, 2) = cut * cut;
    TubeZ(2, 0) = 0; TubeZ(0, 2) = 0;
    TubeZ(2, 1) = 0; TubeZ(1, 2) = 0;


    // rotate err-matrix such that eZ goes to pBtagEstimate
    TMatrixD Tube = rotateTensor(pBtagEstimate.Vect(), TubeZ);

    // Standard algorithm needs no shift
    return make_pair(m_BeamSpotCenter, toSymMatrix(Tube));
  }

  pair<TVector3, TMatrixDSym> TagVertexModule::findConstraintBTube(const Particle* Breco, double cut)
  {
    //Use Breco as the creator of the B tube.
    if ((Breco->getVertexErrorMatrix()(2, 2)) == 0.0) {
      B2WARNING("In TagVertexModule::findConstraintBTube: cannot get a proper vertex for BReco. BTube constraint replaced by Boost.");
      return findConstraintBoost(cut);
    }


    //vertex fit will give the intersection between the beam spot and the trajectory of the B
    //(base of the BTube, or primary vtx cov matrix)
    const Particle* tubecreatorBCopy = doVertexFitForBTube(Breco, "avf");
    if (tubecreatorBCopy->getPValue() < 0) return make_pair(vecNaN, matNaN); //if problems


    //get direction of B tag = opposite direction of B rec in CMF
    TLorentzVector pBrec = tubecreatorBCopy->get4Vector();

    //if we want the true info, replace the 4vector by the true one
    if (m_useTruthInFit) {
      const MCParticle* mcBr = Breco->getRelated<MCParticle>();
      if (mcBr)
        pBrec = mcBr->get4Vector();
      else
        m_fitTruthStatus = 2;
    }
    TLorentzVector pBtag = flipVector(pBrec);

    //To create the B tube, strategy is: take the primary vtx cov matrix, and add to it a cov
    //matrix corresponding to an very big error in the direction of the B tag
    TMatrixDSym pv = tubecreatorBCopy->getVertexErrorMatrix();

    //print some stuff if wanted
    if (m_verbose) {
      B2DEBUG(10, "Brec decay vertex before fit: " << printVector(Breco->getVertex()));
      B2DEBUG(10, "Brec decay vertex after fit: " << printVector(tubecreatorBCopy->getVertex()));
      B2DEBUG(10, "Brec direction before fit: " << printVector((1. / Breco->getP()) * Breco->getMomentum()));
      B2DEBUG(10, "Brec direction after fit: " << printVector((1. / tubecreatorBCopy->getP()) * tubecreatorBCopy->getMomentum()));
      B2DEBUG(10, "IP position: " << printVector(m_BeamSpotCenter));
      B2DEBUG(10, "IP covariance: " << printMatrix(m_BeamSpotCov));
      B2DEBUG(10, "Brec primary vertex: " << printVector(tubecreatorBCopy->getVertex()));
      B2DEBUG(10, "Brec PV covariance: " << printMatrix(pv));
      B2DEBUG(10, "BTag direction: " << printVector((1. / pBtag.P())*pBtag.Vect()));
    }

    //make a long error matrix along BTag direction
    TMatrixD longerror(3, 3); longerror(2, 2) = cut * cut;


    // make rotation matrix from z axis to BTag line of flight
    TMatrixD longerrorRotated = rotateTensor(pBtag.Vect(), longerror);

    //pvNew will correspond to the covariance matrix of the B tube
    TMatrixD pvNew = TMatrixD(pv) + longerrorRotated;

    //set the constraint
    TVector3 constraintCenter = tubecreatorBCopy->getVertex();

    //if we want the true info, set the centre of the constraint to the primary vertex
    if (m_useTruthInFit) {
      const MCParticle* mcBr = Breco->getRelated<MCParticle>();
      if (mcBr) {
        constraintCenter = mcBr->getProductionVertex();
      }
    }

    if (m_verbose) {
      B2DEBUG(10, "IPTube covariance: " << printMatrix(pvNew));
    }

    //The following is done to do the BTube constraint with a virtual track
    //(ie KFit way)

    m_tagMomentum = pBtag;

    m_pvCov.ResizeTo(pv);
    m_pvCov = pv;

    return make_pair(constraintCenter, toSymMatrix(pvNew));
  }

  pair<TVector3, TMatrixDSym> TagVertexModule::findConstraintBoost(double cut, double shiftAlongBoost) const
  {
    //make a long error matrix along boost direction
    TMatrixD longerror(3, 3); longerror(2, 2) = cut * cut;
    TVector3 boostDir = PCmsLabTransform().getBoostVector().Unit();
    TMatrixD longerrorRotated = rotateTensor(boostDir, longerror);

    //Extend error of BeamSpotCov matrix in the boost direction
    TMatrixDSym beamSpotCov = m_beamSpotDB->getCovVertex();
    TMatrixD Tube = TMatrixD(beamSpotCov) + longerrorRotated;

    // Standard algorithm needs no shift
    TVector3 constraintCenter = m_BeamSpotCenter;

    // The constraint used in the Single Track Fit needs to be shifted in the boost direction.
    if (shiftAlongBoost > -1000) {
      constraintCenter +=  shiftAlongBoost * boostDir;
    }

    return make_pair(constraintCenter,   toSymMatrix(Tube));
  }

  /** proper life time, i.e. in the rest system (in ps) */
  static double getProperLifeTime(const MCParticle* mc)
  {
    double beta = mc->getMomentum().Mag() / mc->getEnergy();
    return 1e3 * mc->getLifetime() * sqrt(1 - pow(beta, 2));
  }

  void TagVertexModule::BtagMCVertex(const Particle* Breco)
  {
    //fill vector with mcB (intended order: Reco, Tag)
    vector<const MCParticle*> mcBs;
    for (const MCParticle& mc : m_mcParticles) {
      if (abs(mc.getPDG()) == abs(Breco->getPDGCode()))
        mcBs.push_back(&mc);
    }
    //too few Bs
    if (mcBs.size() < 2) return;

    if (mcBs.size() > 2) {
      B2WARNING("TagVertexModule:: Too many Bs found in MC");
    }

    auto isReco = [&](const MCParticle * mc) {
      return (m_useMCassociation == "breco") ? (mc == Breco->getRelated<MCParticle>())
             : compBrecoBgen(Breco, mc);  //internal association
    };

    //nothing matched?
    if (!isReco(mcBs[0]) && !isReco(mcBs[1])) {
      return;
    }

    //first is Tag, second Reco -> swap the order
    if (!isReco(mcBs[0]) && isReco(mcBs[1]))
      swap(mcBs[0], mcBs[1]);

    //both matched -> use closest vertex dist as Reco
    if (isReco(mcBs[0]) && isReco(mcBs[1])) {
      double dist0 = (mcBs[0]->getDecayVertex() - Breco->getVertex()).Mag2();
      double dist1 = (mcBs[1]->getDecayVertex() - Breco->getVertex()).Mag2();
      if (dist0 > dist1)
        swap(mcBs[0], mcBs[1]);
    }

    m_mcVertReco = mcBs[0]->getDecayVertex();
    m_mcLifeTimeReco  = getProperLifeTime(mcBs[0]);
    m_mcTagV = mcBs[1]->getDecayVertex();
    m_mcTagLifeTime = getProperLifeTime(mcBs[1]);
    m_mcPDG = mcBs[1]->getPDG();
  }

  // static
  bool TagVertexModule::compBrecoBgen(const Particle* Breco, const MCParticle* Bgen)
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
   The option to request PXD hits for the tracks can be chosen by the user.
   */
  std::vector<const Particle*> TagVertexModule::getTagTracks_standardAlgorithm(const Particle* Breco, int reqPXDHits) const
  {
    std::vector<const Particle*> fitParticles;
    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();
    if (!roe) return fitParticles;
    //load all particles from the ROE
    std::vector<const Particle*> ROEParticles = roe->getChargedParticles(m_roeMaskName, 0 , false);
    if (ROEParticles.size() == 0) return fitParticles;

    for (auto& ROEParticle : ROEParticles) {
      HitPatternVXD roeTrackPattern = ROEParticle->getTrackFitResult()->getHitPatternVXD();

      if (roeTrackPattern.getNPXDHits() >= reqPXDHits) {
        fitParticles.push_back(ROEParticle);
      }
    }
    return fitParticles;
  }

  vector<ParticleAndWeight> TagVertexModule::getParticlesWithoutKS(const vector<const Particle*>& tagParticles,
      double massWindowWidth) const
  {
    vector<ParticleAndWeight> particleAndWeights;

    ParticleAndWeight particleAndWeight;
    particleAndWeight.mcParticle = 0;
    particleAndWeight.weight = -1111.;

    // remove tracks from KS
    for (unsigned i = 0; i < tagParticles.size(); ++i) {
      const Particle* particle1 = tagParticles.at(i);
      if (!particle1) continue;
      TLorentzVector mom1 = particle1->get4Vector();
      if (!isfinite(mom1.Mag2())) continue;

      //is from Ks decay?
      bool isKsDau = false;
      for (unsigned j = 0; j < tagParticles.size(); ++j) {
        if (i == j) continue;
        const Particle* particle2 = tagParticles.at(j);
        if (!particle2) continue;
        TLorentzVector mom2 = particle2->get4Vector();
        if (!isfinite(mom2.Mag2())) continue;
        double mass = (mom1 + mom2).M();
        if (abs(mass - Const::K0Mass) < massWindowWidth) {
          isKsDau = true;
          break;
        }
      }
      //if from Ks decay, skip
      if (isKsDau) continue;

      particleAndWeight.particle = particle1;

      if (m_useMCassociation == "breco" || m_useMCassociation == "internal")
        particleAndWeight.mcParticle = particle1->getRelatedTo<MCParticle>();

      particleAndWeights.push_back(particleAndWeight);

    }

    return particleAndWeights;
  }

  bool TagVertexModule::makeGeneralFit()
  {
    if (m_fitAlgo == "Rave") return makeGeneralFitRave();
    else if (m_fitAlgo == "KFit") return makeGeneralFitKFit();
    return false;
  }

  void TagVertexModule::fillParticles(vector<ParticleAndWeight>& particleAndWeights)
  {
    unsigned n = particleAndWeights.size();
    sort(particleAndWeights.begin(), particleAndWeights.end(),
    [](const ParticleAndWeight & a, const ParticleAndWeight & b) { return a.weight > b.weight; });

    m_raveParticles.resize(n);
    m_raveWeights.resize(n);
    m_raveMCParticles.resize(n);

    for (unsigned i = 0; i < n; ++i) {
      m_raveParticles.at(i) = particleAndWeights.at(i).particle;
      m_raveMCParticles.at(i) = particleAndWeights.at(i).mcParticle;
      m_raveWeights.at(i) = particleAndWeights.at(i).weight;
    }
  }

  void TagVertexModule::fillTagVinfo(const TVector3& tagVpos, const TMatrixDSym& tagVposErr)
  {
    m_tagV = tagVpos;

    if (m_constraintType != "noConstraint") {
      TMatrixDSym tubeInv = m_constraintCov;
      tubeInv.Invert();
      TVectorD dV = toVec(m_tagV - m_BeamSpotCenter);
      m_tagVChi2IP = tubeInv.Similarity(dV);
    }

    m_tagVErrMatrix.ResizeTo(tagVposErr);
    m_tagVErrMatrix = tagVposErr;
  }

  bool TagVertexModule::makeGeneralFitRave()
  {
    // apply constraint
    analysis::RaveSetup::getInstance()->unsetBeamSpot();
    if (m_constraintType != "noConstraint")
      analysis::RaveSetup::getInstance()->setBeamSpot(m_constraintCenter, m_constraintCov);
    analysis::RaveVertexFitter rFit;

    //feed rave with tracks without Kshorts
    vector<ParticleAndWeight> particleAndWeights = getParticlesWithoutKS(m_tagParticles);

    for (const auto& pw : particleAndWeights) {
      try {
        if (m_useTruthInFit) {
          if (pw.mcParticle) {
            TrackFitResult tfr(getTrackWithTrueCoordinates(pw));
            rFit.addTrack(&tfr);
          } else
            m_fitTruthStatus = 2;
        } else if (m_useRollBack) {
          if (pw.mcParticle) {
            TrackFitResult tfr(getTrackWithRollBackCoordinates(pw));
            rFit.addTrack(&tfr);
          } else
            m_rollbackStatus = 2;
        } else {
          rFit.addTrack(pw.particle->getTrackFitResult());
        }
      } catch (const rave::CheckedFloatException&) {
        B2ERROR("Exception caught in TagVertexModule::makeGeneralFitRave(): Invalid inputs (nan/inf)?");
      }
    }

    //perform fit

    int isGoodFit(-1);

    try {
      isGoodFit = rFit.fit("avf");
      // if problems
      if (isGoodFit < 1) return false;
    } catch (const rave::CheckedFloatException&) {
      B2ERROR("Exception caught in TagVertexModule::makeGeneralFitRave(): Invalid inputs (nan/inf)?");
      return false;
    }

    //save the track info for later use

    for (unsigned int i(0); i < particleAndWeights.size() && isGoodFit >= 1; ++i)
      particleAndWeights.at(i).weight = rFit.getWeight(i);

    //Tracks are sorted from highest rave weight to lowest

    fillParticles(particleAndWeights);

    //if the fit is good, save the infos related to the vertex
    fillTagVinfo(rFit.getPos(0), rFit.getCov(0));

    //fill quality variables
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
    if (m_constraintType != "noConstraint") {
      if (m_constraintType == "tube") {
        CLHEP::HepSymMatrix err(7, 0);
        //copy m_pvCov to the end of err matrix
        err.sub(5, ROOTToCLHEP::getHepSymMatrix(m_pvCov));
        kFit.setIpTubeProfile(
          ROOTToCLHEP::getHepLorentzVector(m_tagMomentum),
          ROOTToCLHEP::getPoint3D(m_constraintCenter),
          err,
          0.);
      } else {
        kFit.setIpProfile(ROOTToCLHEP::getPoint3D(m_constraintCenter),
                          ROOTToCLHEP::getHepSymMatrix(m_constraintCov));
      }
    }

    //feed KFit with tracks without Kshorts
    vector<ParticleAndWeight> particleAndWeights = getParticlesWithoutKS(m_tagParticles);

    int nTracksAdded = 0;
    for (auto& pawi : particleAndWeights) {
      int addedOK = 1;
      if (m_useTruthInFit) {
        if (pawi.mcParticle) {
          addedOK = kFit.addTrack(
                      ROOTToCLHEP::getHepLorentzVector(pawi.mcParticle->get4Vector()),
                      ROOTToCLHEP::getPoint3D(getTruePoca(pawi)),
                      ROOTToCLHEP::getHepSymMatrix(pawi.particle->getMomentumVertexErrorMatrix()),
                      pawi.particle->getCharge());
        } else {
          m_fitTruthStatus = 2;
        }
      } else if (m_useRollBack) {
        if (pawi.mcParticle) {
          addedOK = kFit.addTrack(
                      ROOTToCLHEP::getHepLorentzVector(pawi.mcParticle->get4Vector()),
                      ROOTToCLHEP::getPoint3D(getRollBackPoca(pawi)),
                      ROOTToCLHEP::getHepSymMatrix(pawi.particle->getMomentumVertexErrorMatrix()),
                      pawi.particle->getCharge());
        } else {
          m_rollbackStatus = 2;
        }
      } else {
        addedOK = kFit.addParticle(pawi.particle);
      }

      if (addedOK == 0) {
        ++nTracksAdded;
        pawi.weight = 1.;
      } else {
        B2WARNING("TagVertexModule::makeGeneralFitKFit: failed to add a track");
        pawi.weight = 0.;
      }
    }

    //perform fit if there are enough tracks
    if ((nTracksAdded < 2 && m_constraintType == "noConstraint") || nTracksAdded < 1)
      return false;

    int isGoodFit = kFit.doFit();
    if (isGoodFit != 0) return false;

    //save the track info for later use
    //Tracks are sorted by weight, ie pushing the tracks with 0 weight (from KS) to the end of the list
    fillParticles(particleAndWeights);

    //Save the infos related to the vertex
    fillTagVinfo(CLHEPToROOT::getTVector3(kFit.getVertex()),
                 CLHEPToROOT::getTMatrixDSym(kFit.getVertexError()));

    m_tagVNDF = kFit.getNDF();
    m_tagVChi2 = kFit.getCHIsq();
    m_fitPval = TMath::Prob(m_tagVChi2, m_tagVNDF);

    return true;
  }

  void TagVertexModule::deltaT(const Particle* Breco)
  {

    TVector3 boost = PCmsLabTransform().getBoostVector();
    TVector3 boostDir = boost.Unit();
    double bg = boost.Mag() / sqrt(1 - boost.Mag2());
    double c = Const::speedOfLight / 1000.; // cm ps-1

    //Reconstructed DeltaL & DeltaT in the boost direction
    TVector3 dVert = Breco->getVertex() - m_tagV; //reconstructed vtxReco - vtxTag
    double dl = dVert.Dot(boostDir);
    m_deltaT  = dl / (bg * c);

    //Truth DeltaL & approx DeltaT in the boost direction
    TVector3 MCdVert = m_mcVertReco - m_mcTagV;   //truth vtxReco - vtxTag
    double MCdl = MCdVert.Dot(boostDir);
    m_mcDeltaT = MCdl / (bg * c);

    // MCdeltaTau=tauRec-tauTag
    m_mcDeltaTau = m_mcLifeTimeReco - m_mcTagLifeTime;
    if (m_mcLifeTimeReco  == -1 || m_mcTagLifeTime == -1)
      m_mcDeltaTau =  realNaN;

    TVectorD bVec = toVec(boostDir);

    //TagVertex error in boost dir
    m_tagVlErr = sqrt(m_tagVErrMatrix.Similarity(bVec));

    //bReco error in boost dir
    double bRecoErrL = sqrt(Breco->getVertexErrorMatrix().Similarity(bVec));

    //Delta t error
    m_deltaTErr =  hypot(m_tagVlErr, bRecoErrL) / (bg * c);

    m_tagVl = m_tagV.Dot(boostDir);
    m_truthTagVl = m_mcTagV.Dot(boostDir);

    // calculate tagV component and error in the direction orthogonal to the boost
    TVector3 oboost = getUnitOrthogonal(boostDir);
    TVectorD oVec = toVec(oboost);

    //TagVertex error in boost-orthogonal dir
    m_tagVolErr = sqrt(m_tagVErrMatrix.Similarity(oVec));

    m_tagVol = m_tagV.Dot(oboost);
    m_truthTagVol = m_mcTagV.Dot(oboost);
  }

  Particle* TagVertexModule::doVertexFitForBTube(const Particle* motherIn, std::string fitType) const
  {
    //make a copy of motherIn to not modify the original object
    Particle* mother = ParticleCopy::copyParticle(motherIn);

    //Here rave is used to find the upsilon(4S) vtx as the intersection
    //between the mother B trajectory and the beam spot
    analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_BeamSpotCov);

    analysis::RaveVertexFitter rsg;
    rsg.addTrack(mother);
    int nvert = rsg.fit(fitType);
    if (nvert != 1) {
      mother->setPValue(-1); //error
      return mother;
    } else {
      rsg.updateDaughters();
      return mother;
    }
  }



  TrackFitResult TagVertexModule::getTrackWithTrueCoordinates(ParticleAndWeight const& paw) const
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
                          m_Bfield, 0, 0, tfr->getNDF());
  }

  // static
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
                          m_Bfield, 0, 0, tfr->getNDF());
  }

  TVector3 TagVertexModule::getRollBackPoca(ParticleAndWeight const& paw)
  {
    if (!paw.mcParticle) {
      B2ERROR("In TagVertexModule::getTruePoca: no MC particle set");
      return TVector3(0., 0., 0.);
    }

    return paw.particle->getTrackFitResult()->getPosition() - paw.mcParticle->getProductionVertex() + m_mcTagV;
  }

  void TagVertexModule::resetReturnParams()
  {
    m_raveParticles.resize(0);
    m_raveMCParticles.resize(0);
    m_tagParticles.resize(0);
    m_raveWeights.resize(0);

    m_fitPval = realNaN;
    m_tagV = vecNaN;
    m_tagVErrMatrix.ResizeTo(matNaN);
    m_tagVErrMatrix = matNaN;
    m_mcTagV = vecNaN;
    m_mcVertReco = vecNaN;
    m_deltaT = realNaN;
    m_deltaTErr = realNaN;
    m_mcDeltaTau = realNaN;
    m_constraintCov.ResizeTo(matNaN);
    m_constraintCov = matNaN;
    m_constraintCenter = vecNaN;
    m_tagVl = realNaN;
    m_truthTagVl = realNaN;
    m_tagVlErr = realNaN;
    m_tagVol = realNaN;
    m_truthTagVol = realNaN;
    m_tagVolErr = realNaN;
    m_tagVNDF = realNaN;
    m_tagVChi2 = realNaN;
    m_tagVChi2IP = realNaN;
    m_pvCov.ResizeTo(matNaN);
    m_pvCov = matNaN;
    m_tagMomentum = TLorentzVector(vecNaN, realNaN);
  }

  //The following functions are just here to help printing stuff

  // static
  std::string TagVertexModule::printVector(const TVector3& vec)
  {
    std::ostringstream oss;
    int w = 14;
    oss << "(" << std::setw(w) << vec[0] << ", " << std::setw(w) << vec[1] << ", " << std::setw(w) << vec[2] << ")" << std::endl;
    return oss.str();
  }

  // static
  std::string TagVertexModule::printMatrix(const TMatrixD& mat)
  {
    std::ostringstream oss;
    int w = 14;
    for (int i = 0; i < mat.GetNrows(); ++i) {
      for (int j = 0; j < mat.GetNcols(); ++j) {
        oss << std::setw(w) << mat(i, j) << " ";
      }
      oss << endl;
    }
    return oss.str();
  }

  // static
  std::string TagVertexModule::printMatrix(const TMatrixDSym& mat)
  {
    std::ostringstream oss;
    int w = 14;
    for (int i = 0; i < mat.GetNrows(); ++i) {
      for (int j = 0; j < mat.GetNcols(); ++j) {
        oss << std::setw(w) << mat(i, j) << " ";
      }
      oss << endl;
    }
    return oss.str();
  }
} // end Belle2 namespace
