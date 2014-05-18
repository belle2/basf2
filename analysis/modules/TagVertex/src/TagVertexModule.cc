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

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/Vertex.h>

// utilities
#include <analysis/utility/PCmsLabTransform.h>

// MC particle
#include <mdst/dataobjects/MCParticle.h>

// Magnetic field
#include <geometry/bfieldmap/BFieldMap.h>

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
    m_Bfield(0), m_fitPval(0)
  {
    // Set module properties
    setDescription("Tag side Vertex Fitter for modular analysis");

    // Parameter definitions
    addParam("ListName", m_listName, "name of particle list", string(""));
    addParam("ConfidenceLevel", m_confidenceLevel,
             "required confidence level of fit to keep particles in the list", 0.001);
    addParam("MCAssociation", m_useBrecoMCassociation,
             "true: use standard Breco MC association. false: use internal MC association", false);
    addParam("UseConstraint", m_useConstraint,
             "Choose spatial constraint for the Tag Vertex fit: boostcut,boost,breco,no ", string("boostcut"));
    //addParam("EventType", m_EventType, "Btag decay type", std::string(""));

  }

  TagVertexModule::~TagVertexModule()
  {
  }

  void TagVertexModule::initialize()
  {

    // magnetic field
    m_Bfield = BFieldMap::Instance().getBField(TVector3(0, 0, 0)).Z();
    // RAVE setup
    analysis::RaveSetup::initialize(1, m_Bfield);
    B2INFO("TagVertexModule : magnetic field = " << m_Bfield);


    // output
    StoreArray<Vertex>::registerPersistent();
    RelationArray::registerPersistent<Particle, Vertex>();



  }

  void TagVertexModule::beginRun()
  {
    //TODO: set magnetic field for each run
    //m_Bfield = BFieldMap::Instance().getBField(TVector3(0,0,0)).Z();
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


    std::vector<unsigned int> toRemove;
    for (unsigned i = 0; i < plist->getListSize(); i++) {
      Particle* particle =  plist->getParticle(i);
      bool ok = doVertexFit(particle);
      if (ok) BtagMCVertex(particle);
      if (ok) deltaT(particle);

      if (!ok) toRemove.push_back(i);

      // save information in the Vertex StoreArray
      if (ok) {
        Vertex* ver = verArray.appendNew(Vertex());
        // create relation: Particle <-> Vertex
        particle->addRelationTo(ver);
        // fill Vertex with content
        ver->setTagVertex(m_tagV);
        ver->setTagVertexErrMatrix(m_tagVErrMatrix);
        ver->setTagVertexPval(m_fitPval);
        ver->setDeltaT(m_deltaT);
        ver->setMCTagVertex(m_MCtagV);
        ver->setMCTagBFlavor(m_mcPDG);
        ver->setMCDeltaT(m_MCdeltaT);

      }


    }
    plist->removeParticles(toRemove);
  }

  void TagVertexModule::endRun()
  {
  }

  void TagVertexModule::terminate()
  {
  }

  bool TagVertexModule::doVertexFit(Particle* Breco)
  {

    if (m_useConstraint.compare(std::string("breco")) == 0) {

    }


    m_fitPval = 0;
    bool ok = false;

    if (m_Bfield == 0) {
      B2ERROR("TagVertex: No magnetic field");
      return false;
    }

    ok = false;
    if (m_useConstraint.compare(std::string("breco")) == 0) ok = findConstraint(Breco);
    if (m_useConstraint.compare(std::string("boostcut")) == 0) ok = findConstraintBoost(0.03);
    if (m_useConstraint.compare(std::string("boost")) == 0) ok = findConstraintBoost(2);
    if (m_useConstraint.compare(std::string("no")) == 0) ok = true;
    if (!ok) {
      B2ERROR("TagVertex: No correct fit constraint");
      return false;
    }

    ok = getTagTracks(Breco);
    if (!ok) return false;
    ok = makeGeneralFit();

    if (!ok) return false;


    deltaT(Breco);


    if (m_fitPval < m_confidenceLevel) return false;
    return true;

  }

  bool TagVertexModule::findConstraint(Particle* Breco)
  {
    if (Breco->getPValue() < 0.) return false;

    TVector3 beamSpot(0, 0, 0);
    TMatrixDSym beamSpotCov(3);
    beamSpotCov(0, 0) = 1e-03 * 1e-03; beamSpotCov(1, 1) = 5.9e-06 * 5.9e-06; beamSpotCov(2, 2) = 1.9e-02 * 1.9e-02;
    analysis::RaveSetup::getInstance()->setBeamSpot(beamSpot, beamSpotCov);

    double pmag = Breco->getMomentumMagnitude();
    double xmag = (Breco->getVertex() - beamSpot).Mag();

    TVector3 Pmom = (pmag / xmag) * (Breco->getVertex() - beamSpot);

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

    TubeZ(2, 2) = 10;
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

    TVector3 beamSpot(0, 0, 0);
    TMatrix beamSpotCov(3, 3);
    beamSpotCov(0, 0) = 1e-03 * 1e-03; beamSpotCov(1, 1) = 5.9e-06 * 5.9e-06; beamSpotCov(2, 2) = cut * cut;
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

        if (m_useBrecoMCassociation) {
          StoreArray<MCParticle> mcParticles;
          const MCParticle* mcBr = Breco->getRelated<MCParticle>();
          m_MCVertReco = mcBr->getDecayVertex();
          if (mcBr == mc) {
            isBreco = true;
          } else {
            isBreco = false;
          }
        } else {
          isBreco = compBrecoBgen(Breco, mc);
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


  bool TagVertexModule::getTagTracks(Particle* Breco)
  {

    const RestOfEvent* roe = Breco->getRelatedTo<RestOfEvent>();

    if (roe) {
      tagTracks = roe->getTracks();
    } else {
      return false;
    }

    return true;
  }


  //bool TagVertexModule::makeSemileptonicFit(Particle *Breco){
  //  return true;
  //}


  bool TagVertexModule::makeGeneralFit()
  {

    // apply constraint
    analysis::RaveSetup::getInstance()->unsetBeamSpot();
    TVector3 beamSpot(0, 0, 0); // TODO: use measured value when it will be available
    if (m_useConstraint.compare(std::string("no")) != 0) analysis::RaveSetup::getInstance()->setBeamSpot(beamSpot, m_tube);


    analysis::RaveVertexFitter rFit;

    // Mpi and MKs
    const double mpi = Const::pionMass;
    const double mks = Const::K0Mass;

    // remove traks from KS
    for (unsigned int i = 0; i < tagTracks.size(); i++) {

      const Track* trak1 = tagTracks[i];

      const TrackFitResult* trak1Res = NULL;
      if (trak1) trak1Res = trak1->getTrackFitResult(Const::pion);

      TVector3 mom1;
      if (trak1Res) mom1 = trak1Res->getMomentum();
      if (!trak1Res) continue;

      bool isKsDau = false;
      for (unsigned int j = 0; j < tagTracks.size(); j++) {

        if (i != j) {
          const Track* trak2 = tagTracks[j];
          const TrackFitResult* trak2Res = NULL;
          if (trak2) trak2Res = trak2->getTrackFitResult(Const::pion);

          TVector3 mom2;
          if (trak2Res) mom2 = trak2Res->getMomentum();
          if (!trak2Res) continue;

          double Mass2 = TMath::Power(TMath::Sqrt(mom1.Mag2() + mpi * mpi) + TMath::Sqrt(mom2.Mag2() + mpi * mpi), 2)
                         - (mom1 + mom2).Mag2();
          double Mass = TMath::Sqrt(Mass2);
          if (TMath::Abs(Mass - mks) < 0.01) isKsDau = true;
        }

      }

      if (!isKsDau) rFit.addTrack(trak1Res);
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

  }



} // end Belle2 namespace
