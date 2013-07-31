/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// This file is intended for for anybody continuing the work on Rave integration into basf2. It shows how to do mass constrained fits of basf2 objects with Rave.
// While this code compiled at some point it was never tested and therefore is completely commented out. It might still be easier for somebody to start with this code instead from scratch.

//#include <analysis/raveInterface/RaveKinematicVertexFitter.h>
//#include <analysis/raveInterface/RaveSetup.h>
//
//#include <GFRaveVertexFactory.h>
//#include <RKTrackRep.h>
////root
//#include <Math/ProbFunc.h>
//
//#include <rave/TransientTrackKinematicParticle.h>
//#include <rave/impl/RaveBase/Converters/interface/RaveStreamers.h>
//
////c++ std lib
//using std::string;
//#include <vector>
//using std::vector;
//#include <iostream>
//using std::cout; using std::endl; using std::cerr;
//using namespace Belle2;
//using namespace analysis;
//
//
//
//RaveKinematicVertexFitter::RaveKinematicVertexFitter(): m_useBeamSpot(false),m_motherParticlePtr(NULL), m_raveAlgorithm("")
//{
//  if (RaveSetup::s_instance == NULL) {
//    B2FATAL("RaveSetup::initialize was not called. It has to be called before RaveSetup or RaveKinematicVertexFitter are used");
//  }
//  m_useBeamSpot = RaveSetup::s_instance->m_useBeamSpot;
//}
//
////RaveKinematicVertexFitter::RaveKinematicVertexFitter(string howToInterfaceRave){
////  if ( howToInterfaceRave == "GFRave" ) {
////    m_gfRave = true;
////    RaveKinematicVertexFitter();
////  } else if(howToInterfaceRave == "Rave" ) {
////    m_gfRave = false;
////    RaveKinematicVertexFitter();
////  } else {
////    B2FATAL("RaveKinematicVertexFitter: \"GFRave\" and \"Rave\" are the only 2 valid options");
////  }
////}
//
//
////RaveKinematicVertexFitter::~RaveKinematicVertexFitter()
////{
////  //destroy all the temporary trackreps created in this class but do not destroy the pointers coming from outside
////  int n = m_ownGfTrackReps.size();
////  for (int i = 0; i not_eq n; ++i) {
////    delete m_ownGfTrackReps[i];
////  }
////
////  n = m_GFRaveVertices.size();
////  for (int i = 0; i not_eq n; ++i) {
////    delete m_GFRaveVertices[i];
////  }
////}
//
//
////void RaveKinematicVertexFitter::addTrack(GFTrack& aGFTrack)
////{
////  GFAbsTrackRep* const aGFTrackRepPtr = aGFTrack.getCardinalRep();
////  if (m_gfRave == true) {
////    m_gfTrackReps.push_back(aGFTrackRepPtr);
////  } else {
////    m_raveTracks.push_back(GFTrackRepToRaveTrack(aGFTrackRepPtr));
////  }
////}
////
////
////void RaveKinematicVertexFitter::addTrack(GFTrack* const aGFTrackPtr)
////{
////  GFAbsTrackRep* const aGFTrackRepPtr = aGFTrackPtr->getCardinalRep();
////  if (m_gfRave == true) {
////    m_gfTrackReps.push_back(aGFTrackRepPtr);
////  } else {
////    m_raveTracks.push_back(GFTrackRepToRaveTrack(aGFTrackRepPtr));
////  }
////}
//
////void RaveKinematicVertexFitter::addTrack(const TrackFitResult* aTrackPtr)
////{
////  if (m_gfRave == true) {
////    //maybe there is a smarter way to convert TMatrixF to TMatrixDSym. If you know one feel free to change it.
////    TMatrixF cov(aTrackPtr->getCovariance6());
////    TMatrixDSym temp(6);
////    for (int i = 0; i not_eq 6; ++i) {
////      for (int j = 0; j not_eq 6; ++j) {
////        temp(i, j) = cov(i, j);
////      }
////    }
////    RKTrackRep* aTrackRepPtr = new RKTrackRep(aTrackPtr->getPosition(), aTrackPtr->getMomentum(), temp, aTrackPtr->getPDGCode());
////    m_ownGfTrackReps.push_back(static_cast<GFAbsTrackRep*>(aTrackRepPtr));
////    m_gfTrackReps.push_back(static_cast<GFAbsTrackRep*>(aTrackRepPtr));
////  } else {
////    m_raveTracks.push_back(TrackFitResultToRaveTrack(aTrackPtr));
////  }
////}
//
////void RaveKinematicVertexFitter::addTrack(GFAbsTrackRep* const aTrackRepPtr)
////{
////  if (m_gfRave == true) {
////    m_gfTrackReps.push_back(aTrackRepPtr);
////  } else {
////    m_raveTracks.push_back(GFTrackRepToRaveTrack(aTrackRepPtr));
////  }
////
////}
//
////rave::Track RaveKinematicVertexFitter::GFTrackRepToRaveTrack(GFAbsTrackRep* const aGFTrackRepPtr) const
////{
////  const int id = m_raveTracks.size();
////
////  const GFDetPlane& refPlane(aGFTrackRepPtr->getReferencePlane());
////  TVector3 pos;
////  TVector3 mom;
////  TMatrixDSym cov;
////
////  aGFTrackRepPtr->getPosMomCov(refPlane, pos, mom, cov);
////
////  // state
////  rave::Vector6D ravestate(pos.X(), pos.Y(), pos.Z(),
////                           mom.X(), mom.Y(), mom.Z());
////
////  rave::Covariance6D ravecov(cov(0, 0), cov(1, 0), cov(2, 0),
////                             cov(1, 1), cov(2, 1), cov(2, 2),
////                             cov(3, 0), cov(4, 0), cov(5, 0),
////                             cov(3, 1), cov(4, 1), cov(5, 1),
////                             cov(3, 2), cov(4, 2), cov(5, 2),
////                             cov(3, 3), cov(4, 3), cov(5, 3),
////                             cov(4, 4), cov(5, 4), cov(5, 5));
////
////  return rave::Track(id, ravestate, ravecov, rave::Charge(aGFTrackRepPtr->getCharge() + 0.1), aGFTrackRepPtr->getChiSqu(), aGFTrackRepPtr->getNDF());
////
////}
//
////rave::Track RaveKinematicVertexFitter::TrackFitResultToRaveTrack(const TrackFitResult* aTrackPtr) const
////{
////  const int id = m_raveTracks.size();
////
////  TVector3 pos = aTrackPtr->getPosition();
////  TVector3 mom = aTrackPtr->getMomentum();
////  TMatrixF cov(aTrackPtr->getCovariance6());
////
////
////  // state
////  rave::Vector6D ravestate(pos.X(), pos.Y(), pos.Z(),
////                           mom.X(), mom.Y(), mom.Z());
////
////  rave::Covariance6D ravecov(cov(0, 0), cov(1, 0), cov(2, 0),
////                             cov(1, 1), cov(2, 1), cov(2, 2),
////                             cov(3, 0), cov(4, 0), cov(5, 0),
////                             cov(3, 1), cov(4, 1), cov(5, 1),
////                             cov(3, 2), cov(4, 2), cov(5, 2),
////                             cov(3, 3), cov(4, 3), cov(5, 3),
////                             cov(4, 4), cov(5, 4), cov(5, 5));
////
////  return rave::Track(id, ravestate, ravecov, rave::Charge(aTrackPtr->getCharge()), 1, 1); //the two 1s are just dummy values. They are not used by Rave anyway
////
////}
//
//
//void RaveKinematicVertexFitter::addTrack(const Particle* aParticlePtr)
//{
//
//  rave::Vector7D raveState(aParticlePtr->getX(),aParticlePtr->getY(),aParticlePtr->getZ(),aParticlePtr->getPx(),aParticlePtr->getPy(),aParticlePtr->getPz(),aParticlePtr->getMass());
//  TMatrixFSym cov = aParticlePtr->getMomentumVertexErrorMatrix();
//
//  rave::Covariance7D raveCov(cov(4,4),cov(4,5),cov(4,6), // x x , x y, x z
//                             cov(5,5),cov(5,6),cov(6,6), // y y , y z, z z
//                             cov(0,4),cov(1,4),cov(2,4), // x px , x py, x pz
//                             cov(0,5),cov(1,5),cov(2,5), // y px , y py, y pz
//                             cov(0,6),cov(1,6),cov(2,6), // z px , z py, z pz
//                             cov(0,0),cov(1,0),cov(2,0), // px px , px py, px pz
//                             cov(1,1),cov(1,2),cov(2,2), // py py , py pz, pz pz
//                             cov(4,3),cov(5,3),cov(6,3), // x m , y m, z m
//                             cov(0,3),cov(1,3),cov(2,3), // px m, py m, pz m
//                             cov(3,3)); // mm
//
//  rave::TransientTrackKinematicParticle aRaveParticle( raveState, raveCov, rave::Charge(aParticlePtr->getCharge()), 1, 1 );
//
//
//
//  m_inputParticles.push_back(aRaveParticle); // 1 and 1 are dummy values for chi2 and ndf. the are not used for the vertex fit
//
//}
//
//void RaveKinematicVertexFitter::addMother( Particle* aMotherParticlePtr)
//{
//  vector<Particle*> daughters = aMotherParticlePtr->getDaughters();
//
//  int nDaughters = daughters.size();
//  for (int i = 0; i not_eq nDaughters; ++i) {
//    addTrack(daughters[i]);
//  }
//
//  //store input pointer so fit results can be written to the mother particle after the fit
//  m_motherParticlePtr = aMotherParticlePtr;
//
//}
//
//int RaveKinematicVertexFitter::fit(string options)
//{
////
////  if (options == "default") {
////    m_raveAlgorithm = "kalman";
////  } else {
////    m_raveAlgorithm = options;
////  }
//  int ndf = 0;
//
//    ndf = 2 * m_inputParticles.size();
//
//  if (m_useBeamSpot == true) {
//    ndf += 3;
//  }
//  if (ndf < 4) {
//    return -1;
//  }
//  int nOfVertices = -100;
////    if (m_useBeamSpot == true) {
////      const TVector3& bsPos = RaveSetup::s_instance->m_beamSpot;
////      const TMatrixDSym& bsCov = RaveSetup::s_instance->m_beamSpotCov;
////      const rave::Covariance3D bsCovRave(bsCov(0, 0), bsCov(0, 1), bsCov(0, 2), bsCov(1, 1), bsCov(1, 2), bsCov(2, 2));
////      RaveSetup::s_instance->m_raveVertexFactory->setBeamSpot(rave::Ellipsoid3D(rave::Point3D(bsPos.X(), bsPos.Y(), bsPos.Z()), bsCovRave));
////    }
//
//  try {
//    m_fittedResult = RaveSetup::s_instance->m_raveKinematicTreeFactory->useVertexFitter(m_inputParticles);
//  } catch (...){
//    nOfVertices = 0;
//  }
//
//  if (nOfVertices == 0){ // vertex fit not successful
//    return 0;
//  }
//  const rave::KinematicParticle fittedParticle = m_fittedResult.topParticle();
//
//  rave::Vector7D fittedState = fittedParticle.fullstate();
//  rave::Covariance7D fittedCov = fittedParticle.fullerror();
//
//
//  cerr << "stuff in KinematicParticle (top particle)\n";
//  cerr << "fittedState\n";
//  cerr << fittedState;
//  cerr << "fittedCov\n";
//  cerr << fittedCov;
//
//  rave::KinematicVertex fittedVertex = m_fittedResult.currentProductionVertex();
//  m_fittedNdf = fittedVertex.ndf();
//  m_fittedChi2 = fittedVertex.chiSquared();
//  m_fittedPValue = ROOT::Math::chisquared_cdf_c(m_fittedChi2,m_fittedNdf);
//  m_fittedPos.SetXYZ(fittedVertex.position().x(),fittedVertex.position().y(),fittedVertex.position().z());
//
//  cerr << "stuff in kinematic vertex\n";
//  std::cerr << "m_fittedNdf " << m_fittedNdf <<" m_fittedChi2 "<< m_fittedChi2 <<" m_fittedPValue "<< m_fittedPValue << std::endl;
//  std::cerr << "pos/cov of kinematic vertex" << std::endl;
//  m_fittedPos.Print();
//
// //  m_fitted4Vector;
////  m_fitted7Cov;
//
//
////    m_raveVertices = RaveSetup::s_instance->m_raveVertexFactory->create(m_raveTracks, m_useBeamSpot);
////    nOfVertices = m_raveVertices.size();
////  }
//  return 1;
//}
