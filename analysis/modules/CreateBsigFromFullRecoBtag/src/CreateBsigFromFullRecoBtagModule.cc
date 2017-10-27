/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/CreateBsigFromFullRecoBtag/CreateBsigFromFullRecoBtagModule.h>

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
REG_MODULE(CreateBsigFromFullRecoBtag)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CreateBsigFromFullRecoBtagModule::CreateBsigFromFullRecoBtagModule() : Module(),
  m_Bfield(0)
{
  // Set module properties
  setDescription("CreateBsigFromFullRecoBtag");

  // Parameter definitions
  addParam("listName", m_listName, "name of particle list", string(""));
  addParam("confidenceLevel", m_confidenceLevel,
           "required confidence level of fit to keep particles in the list. Note that even with confidenceLevel == 0.0, errors during the fit might discard Particles in the list. confidenceLevel = -1 if an error occurs during the fit",
           0.);
  addParam("listOutput", m_listOutput, "Bsig particle list created", string(""));
  addParam("writeOut", m_writeOut,
           "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
}

CreateBsigFromFullRecoBtagModule::~CreateBsigFromFullRecoBtagModule()
{
}

void CreateBsigFromFullRecoBtagModule::initialize()
{
  // magnetic field
  m_Bfield = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Unit::T;

  // RAVE setup
  analysis::RaveSetup::initialize(1, m_Bfield);

  B2INFO("CreateBsigFromFullRecoBtag : magnetic field = " << m_Bfield);

  StoreObjPtr<ParticleList> particleList(m_listOutput);
  DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
  particleList.registerInDataStore(flags);
  if (!m_isSelfConjugatedParticle) {
    StoreObjPtr<ParticleList> antiParticleList(m_antiListOutput);
    antiParticleList.registerInDataStore(flags);
  }

}

void CreateBsigFromFullRecoBtagModule::beginRun()
{
}

void CreateBsigFromFullRecoBtagModule::event()
{

  StoreObjPtr<ParticleList> plist(m_listName);
  if (!plist) {
    B2ERROR("ParticleList " << m_listName << " not found");
    return;
  }

  analysis::RaveSetup::initialize(1, m_Bfield);

  m_BeamSpotCenter = m_beamParams->getVertex();
  m_beamSpotCov.ResizeTo(3, 3);
  m_beamSpotCov = m_beamParams->getCovVertex();


  std::vector<unsigned int> toRemove;
  unsigned int n = plist->getListSize();
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = plist->getParticle(i);
    bool ok = doVertexFit(particle);
    if (!ok) particle->setPValue(-1);
    if (m_confidenceLevel > 0. && particle->getPValue() == 0.) {
      toRemove.push_back(particle->getArrayIndex());
    } else {
      if (particle->getPValue() < m_confidenceLevel)toRemove.push_back(particle->getArrayIndex());
    }
  }
  plist->removeParticles(toRemove);


  StoreArray<Particle> particles;

  StoreObjPtr<ParticleList> outputList(m_listOutput);
  outputList.create();
  outputList->initialize(plist->getAntiParticlePDGCode(), m_listOutput);

  if (!m_isSelfConjugatedParticle) {
    StoreObjPtr<ParticleList> outputAntiList(m_antiListOutput);
    outputAntiList.create();
    outputAntiList->initialize(plist->getPDGCode(), m_antiListOutput);

    outputList->bindAntiParticleList(*(outputAntiList));
  }


  for (unsigned i = 0; i < plist->getListSize(); i++) {

    Particle* particle = plist->getParticle(i);

    TLorentzVector v4Final = particle->get4Vector();
    PCmsLabTransform T;
    TLorentzVector vec = T.rotateLabToCms() * v4Final;
    TLorentzVector vecNew(-1 * vec.Px(), -1 * vec.Py(), -1 * vec.Pz(), vec.E());
    TLorentzVector v4FinalNew = T.rotateCmsToLab() * vecNew;


    TMatrixFSym pp = particle->getMomentumErrorMatrix().GetSub(0, 2, 0, 2, "S");
    double pe = particle->getMomentumErrorMatrix()(2, 2);
    TMatrixFSym pv = particle->getVertexErrorMatrix();

    // start rotation
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


    TMatrix ppZPart(3, 3);  ppZPart.Mult(r1t, pp);
    TMatrix ppZ(3, 3); ppZ.Mult(ppZPart, r1);

    TMatrix pvZPart(3, 3);  pvZPart.Mult(r1t, pv);
    TMatrix pvZ(3, 3); pvZ.Mult(pvZPart, r1);

    //pvZ(2, 2) = 100; // set long Z error

    double theta = vecNew.Theta();
    double phi = vecNew.Phi();

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

    TMatrix ppNewPart(3, 3);  ppNewPart.Mult(r2t, ppZ);
    TMatrix ppNew(3, 3); ppNew.Mult(ppNewPart, r2);

    TMatrix pvNewPart(3, 3);  pvNewPart.Mult(r2t, pvZ);
    TMatrix pvNew(3, 3); pvNew.Mult(pvNewPart, r2);

    TMatrixFSym errNew(7);
    errNew.SetSub(0, 0, ppNew);
    errNew.SetSub(4, 4, pvNew);
    errNew(3, 3) = pe;

    int newPdg = -1 * particle->getPDGCode();

    Particle newParticle(v4FinalNew, newPdg);
    newParticle.setMomentumVertexErrorMatrix(errNew);

    particles.appendNew(newParticle);
    int iparticle = particles.getEntries() - 1;

    outputList->addParticle(iparticle, newPdg, newParticle.getFlavorType());


  }

  //free memory allocated by rave. initialize() would be enough, except that we must clean things up before program end...
  analysis::RaveSetup::getInstance()->reset();
}


bool CreateBsigFromFullRecoBtagModule::doVertexFit(Particle* mother)
{

  analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_beamSpotCov);


  std::vector<const Particle*> tracksVertex;

  analysis::RaveVertexFitter rsg;
  rsg.addTrack(mother);
  int nvert = rsg.fit("avf");

  if (nvert == 1) {
    rsg.updateDaughters();
  } else {return false;}


  return true;
}



void CreateBsigFromFullRecoBtagModule::endRun()
{
}

void CreateBsigFromFullRecoBtagModule::terminate()
{
}


