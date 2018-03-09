/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler, Luigi Li Gioi                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/raveInterface/RaveKinematicVertexFitter.h>
#include <analysis/raveInterface/RaveSetup.h>

//root
#include <Math/ProbFunc.h>

#include <rave/TransientTrackKinematicParticle.h>
#include <rave/impl/RaveBase/Converters/interface/RaveStreamers.h>

#include <rave/KinematicTreeFactory.h>
#include <rave/KinematicConstraint.h>
#include <rave/KinematicConstraintBuilder.h>
#include <rave/VertexFactory.h>


//#include <analysis/dataobjects/Particle.h>

//c++ std lib
using std::string;
#include <vector>
using std::vector;
#include <iostream>
using std::cout; using std::endl; using std::cerr;
using namespace Belle2;
using namespace analysis;



RaveKinematicVertexFitter::RaveKinematicVertexFitter(): m_useBeamSpot(false), m_motherParticlePtr(NULL), m_raveAlgorithm(""),
  m_massConstFit(false), m_vertFit(true)
{
  if (RaveSetup::getRawInstance() == NULL) {
    B2FATAL("RaveSetup::initialize was not called. It has to be called before RaveSetup or RaveKinematicVertexFitter are used");
  }
  m_useBeamSpot = RaveSetup::getRawInstance()->m_useBeamSpot;
}

IOIntercept::InterceptorScopeGuard<IOIntercept::OutputToLogMessages> RaveKinematicVertexFitter::captureOutput()
{
  static IOIntercept::OutputToLogMessages s_captureOutput("Rave", LogConfig::c_Debug, LogConfig::c_Debug);
  return IOIntercept::start_intercept(s_captureOutput);
}


RaveKinematicVertexFitter::~RaveKinematicVertexFitter()
{
}


void RaveKinematicVertexFitter::setMassConstFit(bool isConstFit)
{
  m_massConstFit = isConstFit;
}

void RaveKinematicVertexFitter::setVertFit(bool isVertFit)
{
  m_vertFit = isVertFit;
}



void RaveKinematicVertexFitter::addTrack(const Particle* aParticlePtr)
{
  rave::Vector7D raveState(aParticlePtr->getX(), aParticlePtr->getY(), aParticlePtr->getZ(), aParticlePtr->getPx(),
                           aParticlePtr->getPy(), aParticlePtr->getPz(), aParticlePtr->getMass());
  TMatrixDSym covP = aParticlePtr->getMomentumVertexErrorMatrix();

  TMatrixDSym covE(7);
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 7; j++) {
      if (i < 3 && j < 3) covE(i, j) = covP(i + 4, j + 4);
      if (i > 2 && j > 2) covE(i, j) = covP(i - 3, j - 3);
      if (i < 3 && j > 2) covE(i, j) = covP(i + 4, j - 3);
      if (i > 2 && j < 3) covE(i, j) = covP(i - 3, j + 4);
    }
  }

  TMatrixDSym cov = ErrorMatrixEnergyToMass(aParticlePtr->get4Vector(), covE);

  rave::Covariance7D raveCov(cov(0, 0), cov(0, 1), cov(0, 2), // x x , x y, x z
                             cov(1, 1), cov(1, 2), cov(2, 2), // y y , y z, z z
                             cov(0, 3), cov(0, 4), cov(0, 5), // x px , x py, x pz
                             cov(1, 3), cov(1, 4), cov(1, 5), // y px , y py, y pz
                             cov(2, 3), cov(2, 4), cov(2, 5), // z px , z py, z pz
                             cov(3, 3), cov(3, 4), cov(3, 5), // px px , px py, px pz
                             cov(4, 4), cov(4, 5), cov(5, 5), // py py , py pz, pz pz
                             cov(0, 6), cov(1, 6), cov(2, 6), // x m , y m, z m
                             cov(3, 6), cov(4, 6), cov(5, 6), // px m, py m, pz m
                             cov(6, 6)); // mm

  rave::TransientTrackKinematicParticle aRaveParticle(raveState, raveCov, rave::Charge(aParticlePtr->getCharge()), 1, 1);
  // 1 and 1 are dummy values for chi2 and ndf. the are not used for the vertex fit

  m_inputParticles.push_back(aRaveParticle);
  m_belleDaughters.push_back(const_cast<Particle*>(aParticlePtr));

}

void RaveKinematicVertexFitter::addMother(const Particle* aMotherParticlePtr)
{
  vector<Particle*> daughters = aMotherParticlePtr->getDaughters();

  int nDaughters = daughters.size();
  for (int i = 0; i not_eq nDaughters; ++i) {
    addTrack(daughters[i]);
  }

  //store input pointer so fit results can be written to the mother particle after the fit
  Particle* tmp = const_cast<Particle*>(aMotherParticlePtr);
  m_motherParticlePtr = tmp;

}


void RaveKinematicVertexFitter::setMother(const Particle* aMotherParticlePtr)
{
  Particle* tmp = const_cast<Particle*>(aMotherParticlePtr);
  m_motherParticlePtr = tmp;
}



int RaveKinematicVertexFitter::fit()
{
  // make sure all output in this function is converted to log messages
  auto output_capture = captureOutput();

  if (m_inputParticles.size() < 2 && m_vertFit) {
    return -1;
  }
  int nOfVertices = -100;
  if (m_useBeamSpot == true) {
    const TVector3& bsPos = RaveSetup::getRawInstance()->m_beamSpot;
    const TMatrixDSym& bsCov = RaveSetup::getRawInstance()->m_beamSpotCov;
    const rave::Covariance3D bsCovRave(bsCov(0, 0), bsCov(0, 1), bsCov(0, 2), bsCov(1, 1), bsCov(1, 2), bsCov(2, 2));
    RaveSetup::getRawInstance()->m_raveVertexFactory->setBeamSpot(rave::Ellipsoid3D(rave::Point3D(bsPos.X(), bsPos.Y(), bsPos.Z()),
        bsCovRave));
  }

  if (m_vertFit && m_massConstFit) {

    rave::KinematicConstraint cs2 = rave::KinematicConstraintBuilder().createMultiTrackMassKinematicConstraint(
                                      m_motherParticlePtr->getPDGMass(), m_inputParticles.size());
    try {
      m_fittedResult = RaveSetup::getRawInstance()->m_raveKinematicTreeFactory->useVertexFitter(m_inputParticles, cs2, "", m_useBeamSpot);
      m_fittedParticle = m_fittedResult.topParticle();
    } catch (...) {
      nOfVertices = 0;
    }
  } else {

    if (m_vertFit) {
      if (!m_massConstFit) {
        try {
          m_fittedResult = RaveSetup::getRawInstance()->m_raveKinematicTreeFactory->useVertexFitter(m_inputParticles, "", m_useBeamSpot);
          m_fittedParticle = m_fittedResult.topParticle();
        } catch (...) {
          nOfVertices = 0;
        }
      }

    }

    if (!m_vertFit && m_massConstFit) {

      try {

        Particle* aParticlePtr = m_motherParticlePtr;

        rave::Vector7D raveState(aParticlePtr->getX(), aParticlePtr->getY(), aParticlePtr->getZ(), aParticlePtr->getPx(),
                                 aParticlePtr->getPy(), aParticlePtr->getPz(), aParticlePtr->getMass());
        TMatrixFSym covP = aParticlePtr->getMomentumVertexErrorMatrix();

        float chi2 = 1;
        float ndf = 1.;

        TMatrixDSym covE(7);
        for (int i = 0; i < 7; i++) {
          for (int j = 0; j < 7; j++) {
            if (i < 3 && j < 3) covE(i, j) = covP(i + 4, j + 4);
            if (i > 2 && j > 2) covE(i, j) = covP(i - 3, j - 3);
            if (i < 3 && j > 2) covE(i, j) = covP(i + 4, j - 3);
            if (i > 2 && j < 3) covE(i, j) = covP(i - 3, j + 4);
          }
        }

        TMatrixDSym cov = ErrorMatrixEnergyToMass(aParticlePtr->get4Vector(), covE);

        rave::Covariance7D raveCov(cov(0, 0), cov(0, 1), cov(0, 2), // x x , x y, x z
                                   cov(1, 1), cov(1, 2), cov(2, 2), // y y , y z, z z
                                   cov(0, 3), cov(0, 4), cov(0, 5), // x px , x py, x pz
                                   cov(1, 3), cov(1, 4), cov(1, 5), // y px , y py, y pz
                                   cov(2, 3), cov(2, 4), cov(2, 5), // z px , z py, z pz
                                   cov(3, 3), cov(3, 4), cov(3, 5), // px px , px py, px pz
                                   cov(4, 4), cov(4, 5), cov(5, 5), // py py , py pz, pz pz
                                   cov(0, 6), cov(1, 6), cov(2, 6), // x m , y m, z m
                                   cov(3, 6), cov(4, 6), cov(5, 6), // px m, py m, pz m
                                   cov(6, 6)); // mm

        rave::TransientTrackKinematicParticle aRaveParticle(raveState, raveCov, rave::Charge(aParticlePtr->getCharge()), chi2, ndf);

        std::vector< rave::KinematicParticle > parts; parts.push_back(aRaveParticle);

        rave::KinematicConstraint constraint = rave::KinematicConstraintBuilder().createMassKinematicConstraint(
                                                 m_motherParticlePtr->getPDGMass(), 0.);

        if (m_motherParticlePtr->getMomentumVertexErrorMatrix().Determinant() != 0) {

          std::vector< rave::KinematicParticle > refitted = RaveSetup::getRawInstance()->m_raveKinematicTreeFactory->useParticleFitter(parts,
                                                            constraint, "ppf:lppf");

          m_fittedParticle = refitted[0];

        } else {
          B2ERROR("[RaveKinematicVertexFitter]: VertexException saying ParentParticleFitter::error inverting covariance matrix occured");
          nOfVertices = 0;
        }


      } catch (...) {
        nOfVertices = 0;
      }
    }

  }


  rave::Vector7D fittedState;
  rave::Covariance7D fittedCov;

  try {
    fittedState = m_fittedParticle.fullstate();
    fittedCov = m_fittedParticle.fullerror();
  } catch (...) {
    nOfVertices = 0;
  }


  for (auto& i : m_inputParticles) i.unlink();
  m_inputParticles.clear();
  //

  if (nOfVertices == 0) { // vertex fit not successful
    return 0;
  }

  if (m_vertFit) {
    rave::KinematicVertex fittedVertex = m_fittedResult.currentDecayVertex();

    m_fittedNdf = fittedVertex.ndf();
    m_fittedChi2 = fittedVertex.chiSquared();
    m_fittedPValue = ROOT::Math::chisquared_cdf_c(m_fittedChi2, m_fittedNdf);
    m_fittedPos.SetXYZ(fittedVertex.position().x(), fittedVertex.position().y(), fittedVertex.position().z());

  } else {
    m_fittedNdf = m_fittedParticle.ndof();
    m_fittedChi2 = m_fittedParticle.chi2();
    m_fittedPValue = ROOT::Math::chisquared_cdf_c(m_fittedChi2, m_fittedNdf);
    m_fittedPos = m_motherParticlePtr->getVertex();
  }


  m_fitted4Vector.SetXYZT(fittedState.p4().p3().x(), fittedState.p4().p3().y(), fittedState.p4().p3().z(), fittedState.p4().energy());

  m_fitted7Cov.ResizeTo(7, 7);

  TMatrixDSym fitted7CovM(7);

  fitted7CovM(3, 6) = fittedCov.dpxm();  fitted7CovM(6, 3) = fitted7CovM(3, 6);
  fitted7CovM(4, 6) = fittedCov.dpym();  fitted7CovM(6, 4) = fitted7CovM(4, 6);
  fitted7CovM(5, 6) = fittedCov.dpzm();  fitted7CovM(6, 5) = fitted7CovM(5, 6);
  fitted7CovM(6, 6) = fittedCov.dmm();   fitted7CovM(6, 6) = fitted7CovM(6, 6);
  fitted7CovM(0, 6) = fittedCov.dxm();   fitted7CovM(6, 0) = fitted7CovM(0, 6);
  fitted7CovM(1, 6) = fittedCov.dym();   fitted7CovM(6, 1) = fitted7CovM(1, 6);
  fitted7CovM(2, 6) = fittedCov.dzm();   fitted7CovM(6, 2) = fitted7CovM(2, 6);

  fitted7CovM(3, 3) = fittedCov.dpxpx(); fitted7CovM(3, 3) = fitted7CovM(3, 3);
  fitted7CovM(3, 4) = fittedCov.dpxpy(); fitted7CovM(4, 3) = fitted7CovM(3, 4);
  fitted7CovM(3, 5) = fittedCov.dpxpz(); fitted7CovM(5, 3) = fitted7CovM(3, 5);
  fitted7CovM(3, 0) = fittedCov.dxpx();  fitted7CovM(0, 3) = fitted7CovM(3, 0);
  fitted7CovM(3, 1) = fittedCov.dypx();  fitted7CovM(1, 3) = fitted7CovM(3, 1);
  fitted7CovM(3, 2) = fittedCov.dzpx();  fitted7CovM(2, 3) = fitted7CovM(3, 2);

  fitted7CovM(4, 4) = fittedCov.dpypy(); fitted7CovM(4, 4) = fitted7CovM(4, 4);
  fitted7CovM(4, 5) = fittedCov.dpypz(); fitted7CovM(5, 4) = fitted7CovM(4, 5);
  fitted7CovM(4, 0) = fittedCov.dxpy();  fitted7CovM(0, 4) = fitted7CovM(4, 0);
  fitted7CovM(4, 1) = fittedCov.dypy();  fitted7CovM(1, 4) = fitted7CovM(4, 1);
  fitted7CovM(4, 2) = fittedCov.dzpy();  fitted7CovM(2, 4) = fitted7CovM(4, 2);

  fitted7CovM(5, 5) = fittedCov.dpzpz(); fitted7CovM(5, 5) = fitted7CovM(5, 5);
  fitted7CovM(5, 0) = fittedCov.dxpz();  fitted7CovM(0, 5) = fitted7CovM(5, 0);
  fitted7CovM(5, 1) = fittedCov.dypz();  fitted7CovM(1, 5) = fitted7CovM(5, 1);
  fitted7CovM(5, 2) = fittedCov.dzpz();  fitted7CovM(2, 5) = fitted7CovM(5, 2);

  fitted7CovM(0, 0) = fittedCov.dxx();   fitted7CovM(0, 0) = fitted7CovM(0, 0);
  fitted7CovM(0, 1) = fittedCov.dxy();   fitted7CovM(1, 0) = fitted7CovM(0, 1);
  fitted7CovM(0, 2) = fittedCov.dxz();   fitted7CovM(2, 0) = fitted7CovM(0, 2);

  fitted7CovM(1, 1) = fittedCov.dyy();   fitted7CovM(1, 1) = fitted7CovM(1, 1);
  fitted7CovM(1, 2) = fittedCov.dyz();   fitted7CovM(2, 1) = fitted7CovM(1, 2);

  fitted7CovM(2, 2) = fittedCov.dzz();   fitted7CovM(2, 2) = fitted7CovM(2, 2);

  TMatrixDSym fitted7CovE = ErrorMatrixMassToEnergy(m_fitted4Vector, fitted7CovM);

  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 7; j++) {
      if (i < 4 && j < 4) m_fitted7Cov(i, j) = fitted7CovE(i + 3, j + 3);
      if (i > 3 && j > 3) m_fitted7Cov(i, j) = fitted7CovE(i - 4, j - 4);
      if (i < 4 && j > 3) m_fitted7Cov(i, j) = fitted7CovE(i + 3, j - 4);
      if (i > 3 && j < 4) m_fitted7Cov(i, j) = fitted7CovE(i - 4, j + 3);
    }
  }

  return 1;
}

void RaveKinematicVertexFitter::updateMother()
{
  m_motherParticlePtr->updateMomentum(m_fitted4Vector, m_fittedPos, m_fitted7Cov, m_fittedPValue);
}

void RaveKinematicVertexFitter::updateDaughters()
{

  m_fittedResult.topParticle();
  std::vector< rave::KinematicParticle > rDau = m_fittedResult.daughterParticles();
  std::vector<Belle2::Particle*> bDau = m_belleDaughters;
  if (rDau.size() == bDau.size()) {
    for (unsigned ii = 0; ii < bDau.size(); ii++) {
      rave::Vector7D fittedState;
      rave::Covariance7D fittedCov;
      fittedState = rDau[ii].fullstate();
      fittedCov = rDau[ii].fullerror();

      TLorentzVector p4;
      p4.SetXYZT(fittedState.p4().p3().x(), fittedState.p4().p3().y(), fittedState.p4().p3().z(), fittedState.p4().energy());

      TVector3 x3(fittedState.x(), fittedState.y(), fittedState.z());


      TMatrixDSym fitted7CovM(7);
      fitted7CovM(3, 6) = fittedCov.dpxm();  fitted7CovM(6, 3) = fitted7CovM(3, 6);
      fitted7CovM(4, 6) = fittedCov.dpym();  fitted7CovM(6, 4) = fitted7CovM(4, 6);
      fitted7CovM(5, 6) = fittedCov.dpzm();  fitted7CovM(6, 5) = fitted7CovM(5, 6);
      fitted7CovM(6, 6) = fittedCov.dmm();   fitted7CovM(6, 6) = fitted7CovM(6, 6);
      fitted7CovM(0, 6) = fittedCov.dxm();   fitted7CovM(6, 0) = fitted7CovM(0, 6);
      fitted7CovM(1, 6) = fittedCov.dym();   fitted7CovM(6, 1) = fitted7CovM(1, 6);
      fitted7CovM(2, 6) = fittedCov.dzm();   fitted7CovM(6, 2) = fitted7CovM(2, 6);

      fitted7CovM(3, 3) = fittedCov.dpxpx(); fitted7CovM(3, 3) = fitted7CovM(3, 3);
      fitted7CovM(3, 4) = fittedCov.dpxpy(); fitted7CovM(4, 3) = fitted7CovM(3, 4);
      fitted7CovM(3, 5) = fittedCov.dpxpz(); fitted7CovM(5, 3) = fitted7CovM(3, 5);
      fitted7CovM(3, 0) = fittedCov.dxpx();  fitted7CovM(0, 3) = fitted7CovM(3, 0);
      fitted7CovM(3, 1) = fittedCov.dypx();  fitted7CovM(1, 3) = fitted7CovM(3, 1);
      fitted7CovM(3, 2) = fittedCov.dzpx();  fitted7CovM(2, 3) = fitted7CovM(3, 2);

      fitted7CovM(4, 4) = fittedCov.dpypy(); fitted7CovM(4, 4) = fitted7CovM(4, 4);
      fitted7CovM(4, 5) = fittedCov.dpypz(); fitted7CovM(5, 4) = fitted7CovM(4, 5);
      fitted7CovM(4, 0) = fittedCov.dxpy();  fitted7CovM(0, 4) = fitted7CovM(4, 0);
      fitted7CovM(4, 1) = fittedCov.dypy();  fitted7CovM(1, 4) = fitted7CovM(4, 1);
      fitted7CovM(4, 2) = fittedCov.dzpy();  fitted7CovM(2, 4) = fitted7CovM(4, 2);

      fitted7CovM(5, 5) = fittedCov.dpzpz(); fitted7CovM(5, 5) = fitted7CovM(5, 5);
      fitted7CovM(5, 0) = fittedCov.dxpz();  fitted7CovM(0, 5) = fitted7CovM(5, 0);
      fitted7CovM(5, 1) = fittedCov.dypz();  fitted7CovM(1, 5) = fitted7CovM(5, 1);
      fitted7CovM(5, 2) = fittedCov.dzpz();  fitted7CovM(2, 5) = fitted7CovM(5, 2);

      fitted7CovM(0, 0) = fittedCov.dxx();   fitted7CovM(0, 0) = fitted7CovM(0, 0);
      fitted7CovM(0, 1) = fittedCov.dxy();   fitted7CovM(1, 0) = fitted7CovM(0, 1);
      fitted7CovM(0, 2) = fittedCov.dxz();   fitted7CovM(2, 0) = fitted7CovM(0, 2);

      fitted7CovM(1, 1) = fittedCov.dyy();   fitted7CovM(1, 1) = fitted7CovM(1, 1);
      fitted7CovM(1, 2) = fittedCov.dyz();   fitted7CovM(2, 1) = fitted7CovM(1, 2);

      fitted7CovM(2, 2) = fittedCov.dzz();   fitted7CovM(2, 2) = fitted7CovM(2, 2);

      TMatrixDSym fitted7CovE = ErrorMatrixMassToEnergy(m_fitted4Vector, fitted7CovM);

      TMatrixDSym fitted7CovDauM(7);
      for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
          if (i < 4 && j < 4) fitted7CovDauM(i, j) = fitted7CovE(i + 3, j + 3);
          if (i > 3 && j > 3) fitted7CovDauM(i, j) = fitted7CovE(i - 4, j - 4);
          if (i < 4 && j > 3) fitted7CovDauM(i, j) = fitted7CovE(i + 3, j - 4);
          if (i > 3 && j < 4) fitted7CovDauM(i, j) = fitted7CovE(i - 4, j + 3);
        }
      }

      float pValDau = rDau[ii].chi2();

      bDau[ii]->updateMomentum(p4, x3, fitted7CovDauM, pValDau);

    }

  } else B2ERROR("Error in Daughters update");

}

Particle* RaveKinematicVertexFitter::getMother()
{
  return m_motherParticlePtr;
}

TVector3 RaveKinematicVertexFitter::getPos()
{
  return m_fittedPos;
}

double RaveKinematicVertexFitter::getPValue()
{
  return m_fittedPValue;
}

double RaveKinematicVertexFitter::getNdf()
{
  return m_fittedNdf;
}

double RaveKinematicVertexFitter::getChi2()
{
  return m_fittedChi2;
}

TMatrixDSym RaveKinematicVertexFitter::getCov()
{
  return m_fitted7Cov;
}

TMatrixDSym RaveKinematicVertexFitter::getVertexErrorMatrix()
{
  TMatrixDSym posErr(3);
  posErr(0, 0) = m_fitted7Cov(4, 4);
  posErr(0, 1) = m_fitted7Cov(4, 5);
  posErr(0, 2) = m_fitted7Cov(4, 6);
  posErr(1, 0) = m_fitted7Cov(5, 4);
  posErr(1, 1) = m_fitted7Cov(5, 5);
  posErr(1, 2) = m_fitted7Cov(5, 6);
  posErr(2, 0) = m_fitted7Cov(6, 4);
  posErr(2, 1) = m_fitted7Cov(6, 5);
  posErr(2, 2) = m_fitted7Cov(6, 6);

  return posErr;
}


TMatrixDSym RaveKinematicVertexFitter::ErrorMatrixMassToEnergy(TLorentzVector p4, TMatrixDSym MassErr)
{

  TMatrix jac(7, 7);
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 7; j++) {
      if (i == j) jac(i, j) = 1;
      else jac(i, j) = 0;
    }
  }
  jac(6, 3) = p4.Px() / p4.E();
  jac(6, 4) = p4.Py() / p4.E();
  jac(6, 5) = p4.Pz() / p4.E();
  jac(6, 6) = p4.M() / p4.E();


  TMatrix jact(7, 7); jact.Transpose(jac);
  TMatrix EnergyErrPart(7, 7); EnergyErrPart.Mult(jac, MassErr);
  TMatrix EnergyErrTemp(7, 7); EnergyErrTemp.Mult(EnergyErrPart, jact);

  TMatrixDSym EnergyErr(7);
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 7; j++) {
      EnergyErr(i, j) = EnergyErrTemp(i, j);
    }
  }

  return EnergyErr;
}

TMatrixDSym RaveKinematicVertexFitter::ErrorMatrixEnergyToMass(TLorentzVector p4, TMatrixDSym EnergyErr)
{

  TMatrix jac(7, 7);
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 7; j++) {
      if (i == j) jac(i, j) = 1;
      else jac(i, j) = 0;
    }
  }
  jac(6, 3) = -1 * p4.Px() / p4.M();
  jac(6, 4) = -1 * p4.Py() / p4.M();
  jac(6, 5) = -1 * p4.Pz() / p4.M();
  jac(6, 6) = p4.E() / p4.M();

  TMatrix jact(7, 7); jact.Transpose(jac);
  TMatrix MassErrPart(7, 7); MassErrPart.Mult(jac, EnergyErr);
  TMatrix MassErrTemp(7, 7); MassErrTemp.Mult(MassErrPart, jact);

  TMatrixDSym MassErr(7);
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 7; j++) {
      MassErr(i, j) = MassErrTemp(i, j);
    }
  }

  return MassErr;
}

