/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/TagVertex/TagVertexModule.h>


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
#include <analysis/dataobjects/Vertex.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>

// utilities
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/VariableManager/TrackVariables.h>

// msdt dataobject
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/HitPatternVXD.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

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
    m_Bfield(0), m_fitPval(0), m_mcPDG(0), m_deltaT(0), m_MCdeltaT(0)
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
    addParam("useFitAlgorithm", m_useFitAlgorithm,
             "Choose the fit algorithm: boost,breco, standard, standard_pxd, singleTrack, singleTrack_pxd, no ", string("standard"));
    addParam("askMCInformation", m_MCInfo,
             "TRUE when requesting MC Information from the tracks performing the vertex fit", false);
    addParam("reqPXDHits", m_reqPXDHits,
             "Minium number of PXD hits for a track to be used in the vertex fit", 0);


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
    StoreArray<Vertex> verArray;
    verArray.registerInDataStore();
    particles.registerRelationTo(verArray);
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
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    // input
    StoreArray<Particle> Particles(plist->getParticleCollectionName());

    // output
    StoreArray<Vertex> verArray;
    analysis::RaveSetup::initialize(1, m_Bfield);

    std::vector<unsigned int> toRemove;
    for (unsigned i = 0; i < plist->getListSize(); i++) {
      Particle* particle =  plist->getParticle(i);
      if (m_useMCassociation == "breco" || m_useMCassociation == "internal") BtagMCVertex(particle);
      bool ok = doVertexFit(particle);
      if (ok) deltaT(particle);

      //if (m_fitPval < m_confidenceLevel) {
      if ((m_fitPval < m_confidenceLevel && m_confidenceLevel != 0)
          || (m_fitPval <= m_confidenceLevel && m_confidenceLevel == 0)) {
        toRemove.push_back(particle->getArrayIndex());
      } else {
        // save information in the Vertex StoreArray
        Vertex* ver = verArray.appendNew();
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
          ver->setNTracks(m_tagTracks.size());
          ver->setTagVl(m_tagVl);
          ver->setTruthTagVl(m_truthTagVl);
          ver->setTagVlErr(m_tagVlErr);
          ver->setTagVol(m_tagVol);
          ver->setTruthTagVol(m_truthTagVol);
          ver->setTagVolErr(m_tagVolErr);
        } else {
          ver->setTagVertex(m_tagV);
          ver->setTagVertexPval(-1.);
          ver->setDeltaT(-1111.);
          ver->setDeltaTErr(-1111.);
          ver->setMCTagBFlavor(0.);
          ver->setMCDeltaT(-1111.);
          ver->setTagVl(-1111.);
          ver->setTruthTagVl(-1111.);
          ver->setTagVlErr(-1111.);
          ver->setTagVol(-1111.);
          ver->setTruthTagVol(-1111.);
          ver->setTagVolErr(-1111.);
        }
      }

    }
    plist->removeParticles(toRemove);

    //free memory allocated by rave. initialize() would be enough, except that we must clean things up before program end...
    analysis::RaveSetup::getInstance()->reset();

  }

  bool TagVertexModule::doVertexFit(Particle* Breco)
  {

    m_fitPval = 1;
    bool ok = false;
    if (!(Breco->getRelatedTo<RestOfEvent>())) return false;

    if (m_Bfield == 0) {
      B2ERROR("TagVertex: No magnetic field");
      return false;
    }

    // The constraint used in the Single Track Fit needs to be shifted in the boost direction.
    PCmsLabTransform T;
    TVector3 boost = T.getBoostVector().BoostVector();
    TVector3 boostDir = boost.Unit();
    float boostAngle = TMath::ATan(float(boostDir[0]) / boostDir[2]); // boost angle with respect from Z
    double bg = boost.Mag() / TMath::Sqrt(1 - boost.Mag2());

    m_shiftZ = 4.184436e+02 * bg *  0.0001;   // shift of 120 um (Belle2) in the boost direction

    if (m_useFitAlgorithm == "singleTrack" || m_useFitAlgorithm == "singleTrack_PXD") {
      m_BeamSpotCenter = m_beamParams->getVertex() +
                         TVector3(m_shiftZ * TMath::Sin(boostAngle), 0., m_shiftZ * TMath::Cos(boostAngle)); // boost in the XZ plane
    } else {
      m_BeamSpotCenter = m_beamParams->getVertex(); // Standard algorithm needs no shift
    }

    double cut = 8.717575e-02 * bg;

    // Each fit algorithm has its own constraint. Therefore, depending on the user's choice, the constraint will change.
    if (m_useFitAlgorithm == "breco") ok = findConstraint(Breco, cut * 2000.);
    if (m_useFitAlgorithm == "singleTrack" || m_useFitAlgorithm == "singleTrack_PXD") {
      ok = findConstraintBoost(cut - m_shiftZ); // The constraint size is specially squeezzed when using the Single Track Algorithm
      if (ok && m_MCInfo) FlavorTaggerInfoMCMatch(
          Breco); // When using the STA, the user can ask for MC information from the tracks performing the fit
    }
    if (m_useFitAlgorithm == "standard" || m_useFitAlgorithm == "standard_PXD") ok = findConstraintBoost(cut);
    if (m_useFitAlgorithm == "boost") ok = findConstraintBoost(cut * 200000.);
    if (m_useFitAlgorithm == "noConstraint") ok = true;
    if (!ok) {
      B2ERROR("TagVertex: No correct fit constraint");
      return false;
    }

    /* Depending on the user's choice, one of the possible algorithms is chosen for the fit. In case the algorithm does not converge, in order to assure
       high efficiency, the next algorithm less restictive is used. I.e, if standard_PXD does not work, the program tries with standard.
    */
    m_FitType = 0;
    if (m_useFitAlgorithm == "singleTrack_PXD") {
      ok = getTagTracks_singleTrackAlgorithm(Breco, 1);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 1;
      }
    }
    if ((ok == false || m_fitPval < 0.001) || m_useFitAlgorithm == "singleTrack") {
      ok = getTagTracks_singleTrackAlgorithm(Breco, 0);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 2;
      }
    }
    if ((ok == false || m_fitPval < 0.001) || m_useFitAlgorithm == "standard_PXD") {
      ok = getTagTracks_standardAlgorithm(Breco, 1);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 3;
      }
    }
    if ((ok == false || m_fitPval < 0.001) || m_useFitAlgorithm == "standard" || m_useFitAlgorithm == "breco"
        || m_useFitAlgorithm == "boost" || m_useFitAlgorithm == "noConstraint") {
      ok = getTagTracks_standardAlgorithm(Breco, m_reqPXDHits);
      if (ok) {
        ok = makeGeneralFit();
        m_FitType = 4;
      }
    }

    if ((ok == false || m_fitPval <= 0.) && m_useFitAlgorithm != "noConstraint") {
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
    beamSpotCov = m_beamParams->getCovVertex();

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

    m_tube.ResizeTo(3, 3);

    m_tube(0, 0) = Tube(0, 0);  m_tube(0, 1) = Tube(0, 1);  m_tube(0, 2) = Tube(0, 2);
    m_tube(1, 0) = Tube(1, 0);  m_tube(1, 1) = Tube(1, 1);  m_tube(1, 2) = Tube(1, 2);
    m_tube(2, 0) = Tube(2, 0);  m_tube(2, 1) = Tube(2, 1);  m_tube(2, 2) = Tube(2, 2);

    return true;

  }


  bool TagVertexModule::findConstraintBoost(double cut)
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


    m_tube.ResizeTo(3, 3);

    m_tube(0, 0) = Tube(0, 0);  m_tube(0, 1) = Tube(0, 1);  m_tube(0, 2) = Tube(0, 2);
    m_tube(1, 0) = Tube(1, 0);  m_tube(1, 1) = Tube(1, 1);  m_tube(1, 2) = Tube(1, 2);
    m_tube(2, 0) = Tube(2, 0);  m_tube(2, 1) = Tube(2, 1);  m_tube(2, 2) = Tube(2, 2);

    return true;


  }


  void TagVertexModule::BtagMCVertex(Particle* Breco)
  {

    bool isBreco = false;
    int nReco = 0;

    TVector3 MCTagVert(-111, -111, -111);
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
      for (unsigned int i = 0; i < recDau.size(); i++) {
        bool isDau = false;
        Particle* dauRec = recDau[i];

        for (unsigned int j = 0; j < genDau.size(); j++) {
          MCParticle* dauGen = genDau[j];

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
  void TagVertexModule::FlavorTaggerInfoMCMatch(Particle* Breco)
  {

    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();
    FlavorTaggerInfo* flavorTagInfo = Breco->getRelatedTo<FlavorTaggerInfo>();

    if (!flavorTagInfo) return;


    std::vector<int> FTGoodTracks;
    std::vector<int> FTTotalTracks;

    std::vector<float> momentum = flavorTagInfo->getP();
    std::vector<Particle*> particle = flavorTagInfo->getParticle();
    std::vector<Belle2::Track*> tracksFT = flavorTagInfo->getTracks();


    // FLAVOR TAG MC MATCHING
    /* The loop runs through all the tracks stored in the FlavorTaggerInfo. For each one it tracks back the mother, grandmother, grand grand mother...
       The iteration will go on while the mother is an immediately decaying particle (PDG). The iteration will stop tracking back mothers once it reaches either the B0, or another particle coming from the B0 that does not decay immediately. In the later case it assumes that the correspondent track does not share its production point with the decaying point of the B0 */
    for (unsigned i = 0; i < tracksFT.size(); i++) {

      if (i == 6 || (tracksFT[i] == NULL)) { // Tracks belonging to the Lambda category or not well reconstructed are discarted
        flavorTagInfo->setIsFromB(0);
        flavorTagInfo->setProdPointResolutionZ(100);
        continue;
      }

      MCParticle* trackMCParticle = particle[i]->getRelatedTo<MCParticle>();
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
    std::vector<const Track*> ROETracks = roe->getTracks();
    int ROEGoodTracks = 0;
    bool exitROEWhile = false;
    int ROETotalTracks = roe->getNTracks();
    for (int i = 0; i < ROETotalTracks; i++) {
      MCParticle* roeTrackMCParticle = ROETracks[i]->getRelatedTo<MCParticle>();
      MCParticle* roeTrackMCParticleMother = roeTrackMCParticle->getMother();
      do {
        int PDG = TMath::Abs(roeTrackMCParticleMother->getPDG());
        std::string motherPDGString = std::to_string(PDG);
        if (PDG == 211 || PDG == 130 || PDG == 310 || PDG == 311 ||
            PDG == 321 || PDG == 411 || PDG == 421 || PDG == 431) break;
        if (motherPDGString.size() == 4 || motherPDGString.size() < 3) break;
        if (roeTrackMCParticleMother->getPDG() == -m_mcPDG) break;
        if (roeTrackMCParticleMother->getPDG() == m_mcPDG) {
          ROEGoodTracks++;
          break;
        }
        MCParticle* roeTrackMCParticleGrandMother = roeTrackMCParticleMother->getMother();
        if (motherPDGString[motherPDGString.size() - 3] == '1' && roeTrackMCParticleGrandMother->getPDG() == m_mcPDG) {
          ROEGoodTracks++;
          exitROEWhile = true;
        } else if (motherPDGString[motherPDGString.size() - 3] == '2' && roeTrackMCParticleGrandMother->getPDG() == m_mcPDG) {
          ROEGoodTracks++;
          exitROEWhile = true;
        } else if (motherPDGString[motherPDGString.size() - 3] == '3' && roeTrackMCParticleGrandMother->getPDG() == m_mcPDG) {
          ROEGoodTracks++;
          exitROEWhile = true;
        } else if (motherPDGString[motherPDGString.size() - 3] == '4' && roeTrackMCParticleGrandMother->getPDG() == m_mcPDG) {
          ROEGoodTracks++;
          exitROEWhile = true;

        } else {
          roeTrackMCParticle = roeTrackMCParticleMother;
          roeTrackMCParticleMother = roeTrackMCParticleGrandMother;
          exitROEWhile = false;
        }
      } while (exitROEWhile == false);
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

    FlavorTaggerInfo* flavorTagInfo = Breco->getRelatedTo<FlavorTaggerInfo>();
    if (!flavorTagInfo) return false;
    std::vector<const Track*> ROETracks = roe->getTracks();
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
      if (i ==  6 || (originalTracks[i] == NULL)) { // Skip Lambdas and non-reconstructed tracks

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
    bool repeatedTrack = false;
    for (unsigned i = 0; i < listTracks.size(); i++) {
      repeatedTrack = false;
      for (int j = i - 1; j >= 0; j--) {
        if (originalTracks[i] == originalTracks[j]) {
          repeatedTrack = true;
          listTracks[i] = listTracks[j]; // If repeated, assign the same number for both tracks
          break;
        }
      }
      if (repeatedTrack == true) continue;
      listTracks[i] = nonRepeated; // Assign different numbers for different tracks
      nonRepeated++;
    }


    // Basic cut. Impact parameter needs to be small.
    for (unsigned i = 0; i < listTracks.size(); i++) {
      if ((listZ0[i] > 0.1 || listD0[i] > 0.1) && listTracks[i] != 0) eliminateTrack(listTracks, i);
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
    for (unsigned i = 0; i < listTracks.size(); i++) {
      if (listTracks[i] == toEliminate) {
        listTracks.at(i) = 0;
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
    std::vector<const Track*> ROETracks = roe->getTracks();
    if (ROETracks.size() == 0) return false;
    std::vector<const Track*> fitTracks;
    for (unsigned i = 0; i < ROETracks.size(); i++) {
      // TODO: this will always return something (so not nullptr) contrary to the previous method
      // used here. This line can be removed as soon as the multi hypothesis fitting method
      // has been properly established
      if (!ROETracks[i]->getTrackFitResultWithClosestMass(Const::pion)) {
        continue;
      }
      HitPatternVXD roeTrackPattern = ROETracks[i]->getTrackFitResultWithClosestMass(Const::pion)->getHitPatternVXD();

      if (roeTrackPattern.getNPXDHits() >= reqPXDHits) {
        fitTracks.push_back(ROETracks[i]);
      }
    }
    if (fitTracks.size() == 0) return false;
    m_tagTracks = fitTracks;
    return true;
  }

  bool TagVertexModule::makeGeneralFit()
  {
    // apply constraint
    analysis::RaveSetup::getInstance()->unsetBeamSpot();
    if (m_useFitAlgorithm != "no") analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_tube);
    analysis::RaveVertexFitter rFit;

    // Mpi &&  MKs
    const double mpi = Const::pionMass;
    const double mks = Const::K0Mass;
    double Mass = 0.0;
    // remove traks from KS
    for (unsigned int i = 0; i < m_tagTracks.size(); i++) {
      const Track* trak1 = m_tagTracks[i];
      const TrackFitResult* trak1Res = NULL;
      if (trak1) trak1Res = trak1->getTrackFitResultWithClosestMass(Const::pion);
      TVector3 mom1;
      if (trak1Res) mom1 = trak1Res->getMomentum();
      if (std::isinf(mom1.Mag2()) == true || std::isnan(mom1.Mag2()) == true) {
        continue;
      }
      if (!trak1Res) continue;

      bool isKsDau = false;
      for (unsigned int j = 0; j < m_tagTracks.size(); j++) {
        if (i != j) {
          const Track* trak2 = m_tagTracks[j];
          const TrackFitResult* trak2Res = NULL;

          if (trak2) trak2Res = trak2->getTrackFitResultWithClosestMass(Const::pion);

          TVector3 mom2;
          if (trak2Res) mom2 = trak2Res->getMomentum();
          if (std::isinf(mom2.Mag2()) == true || std::isnan(mom2.Mag2()) == true) continue;
          if (!trak2Res) continue;

          double Mass2 = TMath::Power(TMath::Sqrt(mom1.Mag2() + mpi * mpi) + TMath::Sqrt(mom2.Mag2() + mpi * mpi), 2)
                         - (mom1 + mom2).Mag2();
          Mass = TMath::Sqrt(Mass2);
          if (TMath::Abs(Mass - mks) < 0.01) isKsDau = true;
        }

      }
      try {
        if (!isKsDau) rFit.addTrack(trak1Res); // Temporal fix: some mom go to Inf
      } catch (rave::CheckedFloatException) {
        B2ERROR("Exception caught in TagVertexModule::makeGeneralFit(): Invalid inputs (nan/inf)?");
      }
    }

    int isGoodFit = rFit.fit("avf");
    if (isGoodFit < 1) return false;

    m_tagV = rFit.getPos(0);
    m_tagVErrMatrix.ResizeTo(rFit.getCov(0));
    m_tagVErrMatrix = rFit.getCov(0);

    m_fitPval = rFit.getPValue();

    return true;

  }


  void TagVertexModule::deltaT(Particle* Breco)
  {

    PCmsLabTransform T;

    TVector3 boost = T.getBoostVector().BoostVector();

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



} // end Belle2 namespace
