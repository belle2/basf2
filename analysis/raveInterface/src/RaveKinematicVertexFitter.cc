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

#include <GFRaveVertexFactory.h>
#include <RKTrackRep.h>
//root
#include <Math/ProbFunc.h>

#include <rave/TransientTrackKinematicParticle.h>
#include <rave/impl/RaveBase/Converters/interface/RaveStreamers.h>

//#include <rave/KinematicTreeFactory.h>
#include <rave/KinematicConstraint.h>
#include <rave/KinematicConstraintBuilder.h>


//#include <analysis/dataobjects/Particle.h>

//c++ std lib
using std::string;
#include <vector>
using std::vector;
#include <iostream>
using std::cout; using std::endl; using std::cerr;
using namespace Belle2;
using namespace analysis;



RaveKinematicVertexFitter::RaveKinematicVertexFitter(): m_useBeamSpot(false), m_motherParticlePtr(NULL), m_raveAlgorithm(""), m_massConstFit(false), m_vertFit(true)
{
  if (RaveSetup::s_instance == NULL) {
    B2FATAL("RaveSetup::initialize was not called. It has to be called before RaveSetup or RaveKinematicVertexFitter are used");
  }
  m_useBeamSpot = RaveSetup::s_instance->m_useBeamSpot;
}

RaveKinematicVertexFitter::RaveKinematicVertexFitter(string howToInterfaceRave)
{
  // if ( howToInterfaceRave == "GFRave" ) {
  //   m_gfRave = true;
  //   RaveKinematicVertexFitter();
  // } else if(howToInterfaceRave == "Rave" ) {
  if (howToInterfaceRave == "Rave") {
    // m_gfRave = false;
    RaveKinematicVertexFitter();
  } else {
    //B2FATAL("RaveKinematicVertexFitter: \"GFRave\" and \"Rave\" are the only 2 valid options");
    B2FATAL("RaveKinematicVertexFitter: \"Rave\" is the only valid option");
  }
}


RaveKinematicVertexFitter::~RaveKinematicVertexFitter()
{
  //destroy all the temporary trackreps created in this class but do not destroy the pointers coming from outside
  // int n = m_ownGfTrackReps.size();
  // for (int i = 0; i not_eq n; ++i) {
  //   delete m_ownGfTrackReps[i];
  // }

  // n = m_GFRaveVertices.size();
  // for (int i = 0; i not_eq n; ++i) {
  //   delete m_GFRaveVertices[i];
  // }
}


void RaveKinematicVertexFitter::setMassConstFit(bool isConstFit)
{
  m_massConstFit = isConstFit;
}

void RaveKinematicVertexFitter::setVertFit(bool isVertFit)
{
  m_vertFit = isVertFit;
}

// void RaveKinematicVertexFitter::addTrack(GFTrack& aGFTrack)
// {
//  GFAbsTrackRep* const aGFTrackRepPtr = aGFTrack.getCardinalRep();
//  if (m_gfRave == true) {
//    m_gfTrackReps.push_back(aGFTrackRepPtr);
//  } else {
//    m_raveTracks.push_back(GFTrackRepToRaveTrack(aGFTrackRepPtr));
//  }
// }


// void RaveKinematicVertexFitter::addTrack(GFTrack* const aGFTrackPtr)
// {
//  GFAbsTrackRep* const aGFTrackRepPtr = aGFTrackPtr->getCardinalRep();
//  if (m_gfRave == true) {
//    m_gfTrackReps.push_back(aGFTrackRepPtr);
//  } else {
//    m_raveTracks.push_back(GFTrackRepToRaveTrack(aGFTrackRepPtr));
//  }
// }

// void RaveKinematicVertexFitter::addTrack(const TrackFitResult* aTrackPtr)
// {
//  if (m_gfRave == true) {
//    //maybe there is a smarter way to convert TMatrixF to TMatrixDSym. If you know one feel free to change it.
//    TMatrixF cov(aTrackPtr->getCovariance6());
//    TMatrixDSym temp(6);
//    for (int i = 0; i not_eq 6; ++i) {
//      for (int j = 0; j not_eq 6; ++j) {
//        temp(i, j) = cov(i, j);
//      }
//    }
//    RKTrackRep* aTrackRepPtr = new RKTrackRep(aTrackPtr->getPosition(), aTrackPtr->getMomentum(), temp, aTrackPtr->getPDGCode());
//    m_ownGfTrackReps.push_back(static_cast<GFAbsTrackRep*>(aTrackRepPtr));
//    m_gfTrackReps.push_back(static_cast<GFAbsTrackRep*>(aTrackRepPtr));
//  } else {
//    m_raveTracks.push_back(TrackFitResultToRaveTrack(aTrackPtr));
//  }
// }

// void RaveKinematicVertexFitter::addTrack(GFAbsTrackRep* const aTrackRepPtr)
// {
//  if (m_gfRave == true) {
//    m_gfTrackReps.push_back(aTrackRepPtr);
//  } else {
//    m_raveTracks.push_back(GFTrackRepToRaveTrack(aTrackRepPtr));
//  }

// }

// rave::Track RaveKinematicVertexFitter::GFTrackRepToRaveTrack(GFAbsTrackRep* const aGFTrackRepPtr) const
// {
//  const int id = m_raveTracks.size();

//  const GFDetPlane& refPlane(aGFTrackRepPtr->getReferencePlane());
//  TVector3 pos;
//  TVector3 mom;
//  TMatrixDSym cov;

//  aGFTrackRepPtr->getPosMomCov(refPlane, pos, mom, cov);

//  // state
//  rave::Vector6D ravestate(pos.X(), pos.Y(), pos.Z(),
//                           mom.X(), mom.Y(), mom.Z());

//  rave::Covariance6D ravecov(cov(0, 0), cov(1, 0), cov(2, 0),
//                             cov(1, 1), cov(2, 1), cov(2, 2),
//                             cov(3, 0), cov(4, 0), cov(5, 0),
//                             cov(3, 1), cov(4, 1), cov(5, 1),
//                             cov(3, 2), cov(4, 2), cov(5, 2),
//                             cov(3, 3), cov(4, 3), cov(5, 3),
//                             cov(4, 4), cov(5, 4), cov(5, 5));

//  return rave::Track(id, ravestate, ravecov, rave::Charge(aGFTrackRepPtr->getCharge() + 0.1), aGFTrackRepPtr->getChiSqu(), aGFTrackRepPtr->getNDF());

// }

// rave::Track RaveKinematicVertexFitter::TrackFitResultToRaveTrack(const TrackFitResult* aTrackPtr) const
// {
//  const int id = m_raveTracks.size();

//  TVector3 pos = aTrackPtr->getPosition();
//  TVector3 mom = aTrackPtr->getMomentum();
//  TMatrixF cov(aTrackPtr->getCovariance6());


//  // state
//  rave::Vector6D ravestate(pos.X(), pos.Y(), pos.Z(),
//                           mom.X(), mom.Y(), mom.Z());

//  rave::Covariance6D ravecov(cov(0, 0), cov(1, 0), cov(2, 0),
//                             cov(1, 1), cov(2, 1), cov(2, 2),
//                             cov(3, 0), cov(4, 0), cov(5, 0),
//                             cov(3, 1), cov(4, 1), cov(5, 1),
//                             cov(3, 2), cov(4, 2), cov(5, 2),
//                             cov(3, 3), cov(4, 3), cov(5, 3),
//                             cov(4, 4), cov(5, 4), cov(5, 5));

//  return rave::Track(id, ravestate, ravecov, rave::Charge(aTrackPtr->getCharge()), 1, 1); //the two 1s are just dummy values. They are not used by Rave anyway

// }


void RaveKinematicVertexFitter::addTrack(const Particle* aParticlePtr)
{

  rave::Vector7D raveState(aParticlePtr->getX(), aParticlePtr->getY(), aParticlePtr->getZ(), aParticlePtr->getPx(), aParticlePtr->getPy(), aParticlePtr->getPz(), aParticlePtr->getMass());
  TMatrixFSym cov = aParticlePtr->getMomentumVertexErrorMatrix();

  rave::Covariance7D raveCov(cov(4, 4), cov(4, 5), cov(4, 6), // x x , x y, x z
                             cov(5, 5), cov(5, 6), cov(6, 6), // y y , y z, z z
                             cov(0, 4), cov(1, 4), cov(2, 4), // x px , x py, x pz
                             cov(0, 5), cov(1, 5), cov(2, 5), // y px , y py, y pz
                             cov(0, 6), cov(1, 6), cov(2, 6), // z px , z py, z pz
                             cov(0, 0), cov(1, 0), cov(2, 0), // px px , px py, px pz
                             cov(1, 1), cov(1, 2), cov(2, 2), // py py , py pz, pz pz
                             cov(4, 3), cov(5, 3), cov(6, 3), // x m , y m, z m
                             cov(0, 3), cov(1, 3), cov(2, 3), // px m, py m, pz m
                             cov(3, 3)); // mm

  rave::TransientTrackKinematicParticle aRaveParticle(raveState, raveCov, rave::Charge(aParticlePtr->getCharge()), 1, 1);
  // 1 and 1 are dummy values for chi2 and ndf. the are not used for the vertex fit

  m_inputParticles.push_back(aRaveParticle);

  //cov.Print();

}

void RaveKinematicVertexFitter::addMother(const Particle* aMotherParticlePtr)
{
  vector<Particle*> daughters = aMotherParticlePtr->getDaughters();

  if (m_vertFit) {
    int nDaughters = daughters.size();
    for (int i = 0; i not_eq nDaughters; ++i) {
      addTrack(daughters[i]);
    }
  } else {
    addTrack(aMotherParticlePtr);
  }

  //store input pointer so fit results can be written to the mother particle after the fit
  //m_motherParticlePtr = aMotherParticlePtr;
  Particle* tmp = const_cast<Particle*>(aMotherParticlePtr);

  m_motherParticlePtr = tmp;

}




int RaveKinematicVertexFitter::fit(string options)
{

  if (options == "default") {
    m_raveAlgorithm = "kalman";
  } else {
    m_raveAlgorithm = options;
  }
  int ndf = 0;

  ndf = 2 * m_inputParticles.size();

  if (m_useBeamSpot == true) {
    ndf += 3;
  }
  //if (ndf < 4) {
  if (ndf < 4 && m_vertFit) {
    return -1;
  }
  int nOfVertices = -100;
  if (m_useBeamSpot == true) {
    const TVector3& bsPos = RaveSetup::s_instance->m_beamSpot;
    const TMatrixDSym& bsCov = RaveSetup::s_instance->m_beamSpotCov;
    const rave::Covariance3D bsCovRave(bsCov(0, 0), bsCov(0, 1), bsCov(0, 2), bsCov(1, 1), bsCov(1, 2), bsCov(2, 2));
    RaveSetup::s_instance->m_raveVertexFactory->setBeamSpot(rave::Ellipsoid3D(rave::Point3D(bsPos.X(), bsPos.Y(), bsPos.Z()), bsCovRave));
  }

  rave::KinematicConstraint cs = rave::KinematicConstraintBuilder().createMassKinematicConstraint(m_motherParticlePtr->getPDGMass(), 0.);


  if (m_vertFit) {
    if (!m_massConstFit) {
      try {
        m_fittedResult = RaveSetup::s_instance->m_raveKinematicTreeFactory->useVertexFitter(m_inputParticles);
        m_fittedParticle = m_fittedResult.topParticle();
      } catch (...) {
        nOfVertices = 0;
      }
    }

    if (m_massConstFit) {
      try {
        m_fittedResult = RaveSetup::s_instance->m_raveKinematicTreeFactory->useVertexFitter(m_inputParticles);
        std::vector< rave::KinematicParticle > parts; parts.push_back(m_fittedResult.topParticle());
        std::vector< rave::KinematicParticle > m_fittedResult2 = RaveSetup::s_instance->m_raveKinematicTreeFactory->useParticleFitter(parts, cs, "ppf:lppf");
        m_fittedParticle = m_fittedResult2[0];
      } catch (...) {
        nOfVertices = 0;
      }
    }

  }

  if (!m_vertFit && m_massConstFit) {

    try {
      //rave::KinematicConstraint cs2 = rave::KinematicConstraintBuilder().createMassKinematicConstraint(m_motherParticlePtr->getPDGMass(),0.);

      if (m_motherParticlePtr->getMomentumVertexErrorMatrix().Determinant() != 0) {

        std::vector< rave::KinematicParticle > m_fittedResult2 = RaveSetup::s_instance->m_raveKinematicTreeFactory->useParticleFitter(m_inputParticles, cs, "ppf:lppf");
        m_fittedParticle = m_fittedResult2[0];
      } else {
        B2ERROR("[RaveKinematicVertexFitter]: VertexException saying ParentParticleFitter::error inverting covariance matrix occured");
        nOfVertices = 0;
      }
    } catch (...) {
      nOfVertices = 0;
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


  if (nOfVertices == 0) { // vertex fit not successful
    return 0;
  }


  if (m_vertFit) {
    rave::KinematicVertex fittedVertex = m_fittedResult.currentDecayVertex();


    m_fittedNdf = fittedVertex.ndf();
    m_fittedChi2 = fittedVertex.chiSquared();
    m_fittedPValue = ROOT::Math::chisquared_cdf_c(m_fittedChi2, m_fittedNdf);
    m_fittedPos.SetXYZ(fittedVertex.position().x(), fittedVertex.position().y(), fittedVertex.position().z());

    m_raveVertices = RaveSetup::s_instance->m_raveVertexFactory->create(m_raveTracks, m_useBeamSpot);
    nOfVertices = m_raveVertices.size();

  } else {
    m_fittedNdf = m_fittedParticle.ndof();
    m_fittedChi2 = m_fittedParticle.chi2();
    m_fittedPValue = ROOT::Math::chisquared_cdf_c(m_fittedChi2, m_fittedNdf);
    m_fittedPos = m_motherParticlePtr->getVertex();
  }


  m_fitted4Vector.SetXYZT(fittedState.p4().p3().x(), fittedState.p4().p3().y(), fittedState.p4().p3().z(), fittedState.p4().energy());

  m_fitted7Cov.ResizeTo(7, 7);

  m_fitted7Cov(3, 0) = fittedCov.dpxm();  m_fitted7Cov(0, 3) = m_fitted7Cov(3, 0);
  m_fitted7Cov(3, 1) = fittedCov.dpym();  m_fitted7Cov(1, 3) = m_fitted7Cov(3, 1);
  m_fitted7Cov(3, 2) = fittedCov.dpzm();  m_fitted7Cov(2, 3) = m_fitted7Cov(3, 2);
  m_fitted7Cov(3, 3) = fittedCov.dmm();   m_fitted7Cov(3, 3) = m_fitted7Cov(3, 3);
  m_fitted7Cov(3, 4) = fittedCov.dxm();   m_fitted7Cov(4, 3) = m_fitted7Cov(3, 4);
  m_fitted7Cov(3, 5) = fittedCov.dym();   m_fitted7Cov(5, 3) = m_fitted7Cov(3, 5);
  m_fitted7Cov(3, 6) = fittedCov.dzm();   m_fitted7Cov(6, 3) = m_fitted7Cov(3, 6);

  m_fitted7Cov(0, 0) = fittedCov.dpxpx(); m_fitted7Cov(0, 0) = m_fitted7Cov(0, 0);
  m_fitted7Cov(0, 1) = fittedCov.dpxpy(); m_fitted7Cov(1, 0) = m_fitted7Cov(0, 1);
  m_fitted7Cov(0, 2) = fittedCov.dpxpz(); m_fitted7Cov(2, 0) = m_fitted7Cov(0, 2);
  m_fitted7Cov(0, 4) = fittedCov.dxpx();  m_fitted7Cov(4, 0) = m_fitted7Cov(0, 4);
  m_fitted7Cov(0, 5) = fittedCov.dypx();  m_fitted7Cov(5, 0) = m_fitted7Cov(0, 5);
  m_fitted7Cov(0, 6) = fittedCov.dzpx();  m_fitted7Cov(6, 0) = m_fitted7Cov(0, 6);

  m_fitted7Cov(1, 1) = fittedCov.dpypy(); m_fitted7Cov(1, 1) = m_fitted7Cov(1, 1);
  m_fitted7Cov(1, 2) = fittedCov.dpypz(); m_fitted7Cov(2, 1) = m_fitted7Cov(1, 2);
  m_fitted7Cov(1, 4) = fittedCov.dxpy();  m_fitted7Cov(4, 1) = m_fitted7Cov(1, 4);
  m_fitted7Cov(1, 5) = fittedCov.dypy();  m_fitted7Cov(5, 1) = m_fitted7Cov(1, 5);
  m_fitted7Cov(1, 6) = fittedCov.dzpy();  m_fitted7Cov(6, 1) = m_fitted7Cov(1, 6);

  m_fitted7Cov(2, 2) = fittedCov.dpzpz(); m_fitted7Cov(2, 2) = m_fitted7Cov(2, 2);
  m_fitted7Cov(2, 4) = fittedCov.dxpz();  m_fitted7Cov(4, 2) = m_fitted7Cov(2, 4);
  m_fitted7Cov(2, 5) = fittedCov.dypz();  m_fitted7Cov(5, 2) = m_fitted7Cov(2, 5);
  m_fitted7Cov(2, 6) = fittedCov.dzpz();  m_fitted7Cov(6, 2) = m_fitted7Cov(2, 6);

  m_fitted7Cov(4, 4) = fittedCov.dxx();   m_fitted7Cov(4, 4) = m_fitted7Cov(4, 4);
  m_fitted7Cov(4, 5) = fittedCov.dxy();   m_fitted7Cov(5, 4) = m_fitted7Cov(4, 5);
  m_fitted7Cov(4, 6) = fittedCov.dxz();   m_fitted7Cov(6, 4) = m_fitted7Cov(4, 6);

  m_fitted7Cov(5, 5) = fittedCov.dyy();   m_fitted7Cov(5, 5) = m_fitted7Cov(5, 5);
  m_fitted7Cov(5, 6) = fittedCov.dyz();   m_fitted7Cov(6, 5) = m_fitted7Cov(5, 6);

  m_fitted7Cov(6, 6) = fittedCov.dzz();   m_fitted7Cov(6, 6) = m_fitted7Cov(6, 6);


  m_motherParticlePtr->updateMomentum(m_fitted4Vector, m_fittedPos, m_fitted7Cov, m_fittedPValue);

  //cout<<"nOfVertices END = "<<nOfVertices<<endl;
  //m_fitted7Cov.Print();
  //m_fitted4Vector.Print();
  //m_fittedPos.Print();
  //cout<<m_fittedPValue<<endl;
  return 1;
}


Particle* RaveKinematicVertexFitter::getMother()
{

  return m_motherParticlePtr;

}

// int RaveKinematicVertexFitter::updateDauthers()
// {
//   std::vector< rave::KinematicParticle > finalStateParticles();

// }
