/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/PartialUpdateDaughters/PartialUpdateDaughtersModule.h>

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

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PartialUpdateDaughters)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PartialUpdateDaughtersModule::PartialUpdateDaughtersModule() : Module(),
  m_Bfield(0)
{
  // Set module properties
  setDescription("Partial Update Daughters");

  // Parameter definitions
  addParam("listName", m_listName, "name of particle list", string(""));
  addParam("confidenceLevel", m_confidenceLevel,
           "required confidence level of fit to keep particles in the list. Note that even with confidenceLevel == 0.0, errors during the fit might discard Particles in the list. confidenceLevel = -1 if an error occurs during the fit",
           0.001);
  addParam("withConstraint", m_withConstraint, "additional constraint on vertex: ipprofile, iptube, mother, iptubecut", string(""));
  addParam("decayString", m_decayString, "specifies which daughter particles are included in the kinematic fit", string(""));
}

PartialUpdateDaughtersModule::~PartialUpdateDaughtersModule()
{
}

void PartialUpdateDaughtersModule::initialize()
{
  // magnetic field
  m_Bfield = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Unit::T;

  // RAVE setup
  analysis::RaveSetup::initialize(1, m_Bfield);

  B2INFO("PartialUpdateDaughters : magnetic field = " << m_Bfield);

  if (m_decayString != "")
    m_decaydescriptor.init(m_decayString);
  if (m_decayString == "")
    B2FATAL("PartialUpdateDaughter: empty decayString, use ParticleVertexFitter");

}

void PartialUpdateDaughtersModule::beginRun()
{
}

void PartialUpdateDaughtersModule::event()
{
  //cout<<"DECAY STRING = "<<m_decayString<<endl;

  StoreObjPtr<ParticleList> plist(m_listName);
  if (!plist) {
    B2ERROR("ParticleList " << m_listName << " not found");
    return;
  }

  analysis::RaveSetup::initialize(1, m_Bfield);

  m_BeamSpotCenter = m_beamParams->getVertex();
  m_beamSpotCov.ResizeTo(3, 3);
  TMatrixDSym beamSpotCov(3);
  if (m_withConstraint == "ipprofile") m_beamSpotCov = m_beamParams->getCovVertex();
  if (m_withConstraint == "iptube") PartialUpdateDaughtersModule::findConstraintBoost(2.);

  if (m_withConstraint == "ipprofile" || m_withConstraint == "iptube"  || m_withConstraint == "mother") {
    analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_beamSpotCov);
  } else {
    if (m_withConstraint != "") B2FATAL("PartialUpdateDaughtersModule: worng constraint");
  }

  std::vector<unsigned int> toRemove;
  unsigned int n = plist->getListSize();
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = plist->getParticle(i);
    ParticleCopy::copyDaughters(particle);

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
  analysis::RaveSetup::getInstance()->reset();
}

bool PartialUpdateDaughtersModule::doVertexFit(Particle* mother)
{

  std::vector<const Particle*> tracksVertex = m_decaydescriptor.getSelectionParticles(mother);
  std::vector<std::string> tracksName = m_decaydescriptor.getSelectionNames();

  int nvert = 0;

  if (tracksVertex.size() == 0) return false;

  if (tracksVertex.size() > 1) {


    analysis::RaveKinematicVertexFitter rsf;
    for (unsigned itrack = 0; itrack < tracksVertex.size(); itrack++) {
      if (tracksVertex[itrack] != mother) {
        rsf.addTrack(tracksVertex[itrack]);
        B2DEBUG(1, "PartialUpdateDaughtersModule: Adding particle " << tracksName[itrack] << " to vertex fit ");
      }
      if (tracksVertex[itrack] == mother) B2WARNING("PartialUpdateDaughtersModule: Selected Mother not used in the fit");
    }

    TVector3 pos; TMatrixDSym RerrMatrix(7);

    nvert = rsf.fit();

    if (nvert > 0) {
      pos = rsf.getPos();
      RerrMatrix = rsf.getCov();
      double prob = rsf.getPValue();
      TLorentzVector mom(mother->getMomentum(), mother->getEnergy());
      TMatrixDSym errMatrix(7);

      for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
          if (i < 4 && j < 4) errMatrix(i, j) = RerrMatrix(i + 3, j + 3);
          if (i > 3 && j > 3) errMatrix(i, j) = RerrMatrix(i - 4, j - 4);
          if (i < 4 && j > 3) errMatrix(i, j) = RerrMatrix(i + 3, j - 4);
          if (i > 3 && j < 4) errMatrix(i, j) = RerrMatrix(i - 4, j + 3);
        }
      }

      mother->updateMomentum(mom, pos, errMatrix, prob);
      rsf.updateDaughters();

    } else {return false;}
  }


  if (tracksVertex.size() == 1) {
    if (m_withConstraint == "") B2FATAL("PartialUpdateDaughtersModule: sigle track fit needs a valid constraint");

    // One track fit cannot be kinematic
    analysis::RaveVertexFitter rsg;
    rsg.addTrack(tracksVertex[0]);
    B2DEBUG(1, "PartialUpdateDaughtersModule: Adding particle " << tracksName[0] << " to vertex fit ");
    if (tracksVertex[0] == mother) B2FATAL("PartialUpdateDaughtersModule: Selected Mother not used in sigle track fit");

    TVector3 pos; TMatrixDSym RerrMatrix(3);

    //nvert = rsg.fit("kalman");
    nvert = rsg.fit("avf");

    if (nvert > 0) {
      pos = rsg.getPos(0);
      RerrMatrix = rsg.getCov(0);
      double prob = rsg.getPValue(0);
      TLorentzVector mom(mother->getMomentum(), mother->getEnergy());
      TMatrixDSym errMatrix(7);
      for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
          if (i > 3 && j > 3) {errMatrix[i][j] = RerrMatrix[i - 4][j - 4];}
          else {errMatrix[i][j] = 0;} // da correggere: lasciare i valori precedenti
        }
      }
      mother->updateMomentum(mom, pos, errMatrix, prob);
      rsg.updateDaughters();

    } else {return false;}
  }

  return true;
}



void PartialUpdateDaughtersModule::endRun()
{
}

void PartialUpdateDaughtersModule::terminate()
{
}


void PartialUpdateDaughtersModule::findConstraintBoost(double cut)
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
