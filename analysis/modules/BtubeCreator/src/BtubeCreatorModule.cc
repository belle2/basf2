/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Dey                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/BtubeCreator/BtubeCreatorModule.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/dbobjects/BeamParameters.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/DecayDescriptor/ParticleListName.h>

#include <analysis/dataobjects/Btube.h>
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
REG_MODULE(BtubeCreator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BtubeCreatorModule::BtubeCreatorModule() : Module(),
  m_Bfield(0)
{
  // Set module properties
  setDescription("BtubeCreator");

  // Parameter definitions
  addParam("listName", m_listName, "name of mother particle list", string(""));
  addParam("confidenceLevel", m_confidenceLevel,
           "required confidence level of fit to keep particles in the list. Note that even with confidenceLevel == 0.0, errors during the fit might discard Particles in the list. confidenceLevel = -1 if an error occurs during the fit",
           0.);
  addParam("verbosity", m_verbose, "print statements", false);
}

BtubeCreatorModule::~BtubeCreatorModule()
{
}

void BtubeCreatorModule::initialize()
{
  // magnetic field
  m_Bfield = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Unit::T;

  //  analysis::RaveSetup::initialize(1, m_Bfield);
  m_BeamSpotCenter = m_beamParams->getVertex();
  m_beamSpotCov.ResizeTo(3, 3);
  m_beamSpotCov = m_beamParams->getCovVertex();
  B2INFO("BtubeCreator : magnetic field = " << m_Bfield);

  StoreArray<Particle> PARTICLES;
  //  PARTICLES.isRequired();


  StoreArray<Btube> tubeconstraint;
  tubeconstraint.registerInDataStore();
  PARTICLES.registerRelationTo(tubeconstraint);
}


void BtubeCreatorModule::event()
{

  StoreObjPtr<ParticleList> plist(m_listName);
  if (!plist) {
    B2ERROR("ParticleList " << m_listName << " not found");
    return;
  }
  StoreArray<Btube> tubeArray;
  analysis::RaveSetup::initialize(1, m_Bfield);

  std::vector<unsigned int> toRemove;
  unsigned int n = plist->getListSize();

  for (unsigned i = 0; i < n; i++) {
    Particle* particle = plist->getParticle(i);

    Particle* child0 = const_cast<Particle*>(particle->getDaughter(0));
    Particle* child1 = const_cast<Particle*>(particle->getDaughter(1));

    //make a copy of child0 aka Btag

    Particle BtagCopy(child0->get4Vector(), child0->getPDGCode());
    BtagCopy.setVertex(child0->getVertex());
    BtagCopy.setMomentumVertexErrorMatrix(child0->getMomentumVertexErrorMatrix());

    Particle* dummyP;
    dummyP  = &BtagCopy;

    bool ok = 0;
    TVector3 tagdecaypos(dummyP->getVertex()[0], dummyP->getVertex()[1], dummyP->getVertex()[2]);

    if (m_verbose) {
      cout << "B tag decay  " << endl;
      cout << "{" << std::fixed << std::setprecision(20) << dummyP->getVertex()[0] << "," << std::fixed << std::setprecision(
             20) << dummyP->getVertex()[1] << "," << std::fixed << std::setprecision(20) << dummyP->getVertex()[2] << "}" << endl;
    }
    bool ok0 = doVertexFit(dummyP);

    if (ok0) {
      child0->writeExtraInfo("avf_fitted_ellipsoid00", dummyP->getVertexErrorMatrix()(0, 0));
      child0->writeExtraInfo("avf_fitted_ellipsoid01", dummyP->getVertexErrorMatrix()(0, 1));
      child0->writeExtraInfo("avf_fitted_ellipsoid02", dummyP->getVertexErrorMatrix()(0, 2));
      child0->writeExtraInfo("avf_fitted_ellipsoid10", dummyP->getVertexErrorMatrix()(1, 0));
      child0->writeExtraInfo("avf_fitted_ellipsoid11", dummyP->getVertexErrorMatrix()(1, 1));
      child0->writeExtraInfo("avf_fitted_ellipsoid12", dummyP->getVertexErrorMatrix()(1, 2));
      child0->writeExtraInfo("avf_fitted_ellipsoid20", dummyP->getVertexErrorMatrix()(2, 0));
      child0->writeExtraInfo("avf_fitted_ellipsoid21", dummyP->getVertexErrorMatrix()(2, 1));
      child0->writeExtraInfo("avf_fitted_ellipsoid22", dummyP->getVertexErrorMatrix()(2, 2));

      child0->writeExtraInfo("Px_after_avf", (dummyP->get4Vector()).Px());
      child0->writeExtraInfo("Py_after_avf", (dummyP->get4Vector()).Py());
      child0->writeExtraInfo("Pz_after_avf", (dummyP->get4Vector()).Pz());
      child0->writeExtraInfo("E_after_avf", (dummyP->get4Vector()).E());
      TVector3 tagOriginpos(dummyP->getVertex()[0], dummyP->getVertex()[1], dummyP->getVertex()[2]);
      TLorentzVector v4Final = dummyP->get4Vector();
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * v4Final;
      //--//            TLorentzVector v4Mother = particle->get4Vector();
      //TLorentzVector v4Mother_cms = T.rotateLabToCms() * v4Mother;
      //TLorentzVector vecNew = v4Mother_cms - vec;
      TLorentzVector vecNew(-1 * vec.Px(), -1 * vec.Py(), -1 * vec.Pz(), vec.E());
      TLorentzVector v4FinalNew = T.rotateCmsToLab() * vecNew;
      //--//    TLorentzVector v4FinalNew = v4Mother - v4Final;
      if (m_verbose) {
        cout << "beamspot center :" << endl;
        cout << "{" << std::fixed << std::setprecision(20) << m_BeamSpotCenter.X() << "," << std::fixed << std::setprecision(
               20) << m_BeamSpotCenter.Y() << "," << std::fixed << std::setprecision(20) << m_BeamSpotCenter.Z() << "}" << endl;
        cout << "beamspot cov" << endl;

        cout << "{" << std::fixed << std::setprecision(20) <<  m_beamSpotCov(0,
             0) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(0,
                 1) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(0, 2) << "}," << endl;
        cout << "{" << std::fixed << std::setprecision(20) <<  m_beamSpotCov(1,
             0) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(1,
                 1) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(1, 2) << "}," << endl;
        cout << "{" << std::fixed << std::setprecision(20) << m_beamSpotCov(2,
             0) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(2,
                 1) << "," << std::fixed << std::setprecision(20) << m_beamSpotCov(2, 2) << "}" << endl;
      }
      TMatrixFSym pp = dummyP->getMomentumErrorMatrix().GetSub(0, 2, 0, 2, "S");
      double pe = dummyP->getMomentumErrorMatrix()(2, 2);
      TMatrixFSym pv = dummyP->getVertexErrorMatrix();

      // start rotation

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


      TMatrix longerror(3, 3); longerror(2, 2) = 1;
      TMatrix longerror_temp(3, 3); longerror_temp.Mult(r2, longerror);
      TMatrix longerrorRotated(3, 3); longerrorRotated.Mult(longerror_temp, r2t);

      //      TMatrix xFat(3, 3);
      // xFat(0,0) = 4 * pv(0,0);

      TMatrix pvNew(3, 3);
      pvNew += pv;
      pvNew += longerrorRotated;
      //      pvNew += xFat;

      TMatrixFSym errNew(7);
      errNew.SetSub(0, 0, pp);
      errNew.SetSub(4, 4, pvNew);
      errNew(3, 3) = pe;

      TMatrixFSym tubeMat(3);
      tubeMat.SetSub(0, 0, pvNew);

      if (m_verbose) {
        cout << "B origin error matrix  :  " << endl;
        cout << "{" << std::fixed << std::setprecision(20) <<  pv(0, 0) << "," << std::fixed << std::setprecision(20) << pv(0,
             1) << "," << std::fixed << std::setprecision(20) << pv(0, 2) << "}," << endl;
        cout << "{" << std::fixed << std::setprecision(20) << pv(1, 0) << "," << std::fixed << std::setprecision(20) << pv(1,
             1) << "," << std::fixed << std::setprecision(20) << pv(1, 2) << "}," << endl;
        cout << "{" << std::fixed << std::setprecision(20) <<  pv(2, 0) << "," << std::fixed << std::setprecision(20) << pv(2,
             1) << "," << std::fixed << std::setprecision(20) << pv(2, 2) << "}" << endl;
      }
      cout << "B tube error matrix  :  " << endl;
      cout << "{" << std::fixed << std::setprecision(20) <<  pvNew(0, 0) << "," << std::fixed << std::setprecision(20) << pvNew(0,
           1) << "," << std::fixed << std::setprecision(20) << pvNew(0, 2) << "}," << endl;
      cout << "{" << std::fixed << std::setprecision(20) <<  pvNew(1, 0) << "," << std::fixed << std::setprecision(20) << pvNew(1,
           1) << "," << std::fixed << std::setprecision(20) << pvNew(1, 2) << "}," << endl;
      cout << "{" << std::fixed << std::setprecision(20) << pvNew(2, 0) << "," << std::fixed << std::setprecision(20) << pvNew(2,
           1) << "," << std::fixed << std::setprecision(20) << pvNew(2, 2) << "}" << endl;
      //      }
      cout << "B origin  " << endl;
      cout << "{" << std::fixed << std::setprecision(20) << dummyP->getVertex()[0] << "," << std::fixed << std::setprecision(
             20) << dummyP->getVertex()[1] << "," << std::fixed << std::setprecision(20) << dummyP->getVertex()[2] << "}" << endl;
      //      }

      dummyP->setMomentumVertexErrorMatrix(errNew);

      Btube* tubeconstraint = tubeArray.appendNew(
                                Btube());//tagOriginpos,tubeMat));
      particle->addRelationTo(tubeconstraint);
      tubeconstraint->setTubeCenter(tagOriginpos);
      tubeconstraint->setTubeMatrix(tubeMat);

      ok = doRaveFit(child1, particle);

      child1->writeExtraInfo("TubeX", dummyP->getVertex()[0]);
      child1->writeExtraInfo("TubeY", dummyP->getVertex()[1]);
      child1->writeExtraInfo("TubeZ", dummyP->getVertex()[2]);

      child1->writeExtraInfo("Tube00", pvNew(0, 0));
      child1->writeExtraInfo("Tube01", pvNew(0, 1));
      child1->writeExtraInfo("Tube02", pvNew(0, 2));
      child1->writeExtraInfo("Tube10", pvNew(1, 0));
      child1->writeExtraInfo("Tube11", pvNew(1, 1));
      child1->writeExtraInfo("Tube12", pvNew(1, 2));
      child1->writeExtraInfo("Tube20", pvNew(2, 0));
      child1->writeExtraInfo("Tube21", pvNew(2, 1));
      child1->writeExtraInfo("Tube22", pvNew(2, 2));

      child1->writeExtraInfo("tubedirX", v4FinalNew.Px());
      child1->writeExtraInfo("tubedirY", v4FinalNew.Py());
      child1->writeExtraInfo("tubedirZ", v4FinalNew.Pz());

    }

    if (ok) {
      if (m_verbose) {
        cout << "B sig decay  " << endl;
        cout << "{" << std::fixed << std::setprecision(20) << child1->getVertex()[0] << "," << std::fixed << std::setprecision(
               20) << child1->getVertex()[1] << "," << std::fixed << std::setprecision(20) << child1->getVertex()[2] << "}" << endl;
        cout << "B sig error matrix  " << endl;
      }
      TMatrixFSym sigE = child1->getVertexErrorMatrix();
      if (m_verbose) {
        cout << "{" << std::fixed << std::setprecision(20) <<  sigE(0, 0) << "," << std::fixed << std::setprecision(20) << sigE(0,
             1) << "," << std::fixed << std::setprecision(20) << sigE(0, 2) << "}," << endl;
        cout << "{" << std::fixed << std::setprecision(20) <<  sigE(1, 0) << "," << std::fixed << std::setprecision(20) << sigE(1,
             1) << "," << std::fixed << std::setprecision(20) << sigE(1, 2) << "}," << endl;
        cout << "{" << std::fixed << std::setprecision(20) <<  sigE(2, 0) << "," << std::fixed << std::setprecision(20) << sigE(2,
             1) << "," << std::fixed << std::setprecision(20) << sigE(2, 2) << "}" << endl;
      }


    }
    if (!ok || !ok0) toRemove.push_back(particle->getArrayIndex());

  }
  plist->removeParticles(toRemove);

  //free memory allocated by rave. initialize() would be enough, except that we must clean things up before program end...
  analysis::RaveSetup::getInstance()->reset();
}

bool BtubeCreatorModule::doVertexFit(Particle* mother)
{
  //  analysis::RaveSetup::initialize(1, m_Bfield);
  analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_beamSpotCov);

  analysis::RaveVertexFitter rsg;
  rsg.addTrack(mother);
  int nvert = rsg.fit("avf");

  if (nvert == 1) {
    rsg.updateDaughters();
  } else {return false;}
  return true;
  //analysis::RaveSetup::getInstance()->reset();
}

bool BtubeCreatorModule::doRaveFit(Particle* mother2, Particle* constraintP)
{
  //  analysis::RaveSetup::initialize(1, m_Bfield); //added
  //              analysis::RaveSetup::getInstance()->setBeamSpot(m_BeamSpotCenter, m_beamSpotCov);
  //            analysis::RaveSetup::getInstance()->setBeamSpot(constraintP->getVertex(), constraintP->getVertexErrorMatrix());
  //               analysis::RaveSetup::getInstance()->unsetBeamSpot();
  auto* Ver = constraintP->getRelatedTo<Btube>();
  cout << "  object X " << (Ver->getTubeCenter()).X() << "  object Y " << (Ver->getTubeCenter()).Y() << "  object Z " <<
       (Ver->getTubeCenter()).Z() << endl;
  analysis::RaveSetup::getInstance()->setBeamSpot(Ver->getTubeCenter(), Ver->getTubeMatrix());

  analysis::RaveKinematicVertexFitter rf;
  rf.addMother(mother2);

  int nVert2 = rf.fit();

  if (nVert2 == 1) {
    rf.updateMother();
    rf.updateDaughters();
  } else {return false;}
  return true;
  //analysis::RaveSetup::getInstance()->reset();//added
}
