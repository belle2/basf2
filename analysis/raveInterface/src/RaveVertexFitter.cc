/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler, Luigi Li Gioi                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/raveInterface/RaveVertexFitter.h>
#include <analysis/raveInterface/RaveSetup.h>
#include <mdst/dataobjects/Track.h>

#include <genfit/GFRaveVertexFactory.h>
#include <genfit/RKTrackRep.h>

//root
#include <Math/ProbFunc.h>
//stl
using std::string;
#include <vector>
using std::vector;

using namespace Belle2;
using namespace analysis;



RaveVertexFitter::RaveVertexFitter(): m_useBeamSpot(false)
{
  //std::cerr << "RaveVertexFitter::RaveVertexFitter()" << std::endl;
  if (RaveSetup::getRawInstance() == NULL) {
    B2FATAL("RaveSetup::initialize was not called. It has to be called before RaveSetup or RaveVertexFitter are used");
  }
  //std::cerr << "m_useBeamSpot " << m_useBeamSpot << std::endl;
  m_useBeamSpot = RaveSetup::getRawInstance()->m_useBeamSpot;
  //std::cerr << "m_useBeamSpot " << m_useBeamSpot << std::endl;
}

void RaveVertexFitter::initBeamSpotMember()
{
  m_useBeamSpot = false;
  if (RaveSetup::getRawInstance() == NULL) {
    B2FATAL("RaveSetup::initialize was not called. It has to be called before RaveSetup or RaveVertexFitter are used");
  }
  m_useBeamSpot = RaveSetup::getRawInstance()->m_useBeamSpot;
}



RaveVertexFitter::~RaveVertexFitter()
{

}


void RaveVertexFitter::addTrack(const genfit::Track& aGFTrack)
{
  const genfit::MeasuredStateOnPlane& fittedState = aGFTrack.getFittedState();
  m_raveTracks.push_back(GFMeasuredStateToRaveTrack(fittedState));
}


void RaveVertexFitter::addTrack(const genfit::Track* aGFTrackPtr)
{
  const genfit::MeasuredStateOnPlane& fittedState = aGFTrackPtr->getFittedState();
  m_raveTracks.push_back(GFMeasuredStateToRaveTrack(fittedState));
}

void RaveVertexFitter::addTrack(const TrackFitResult* const aTrackPtr)
{
  m_raveTracks.push_back(TrackFitResultToRaveTrack(aTrackPtr));
}



rave::Track RaveVertexFitter::GFMeasuredStateToRaveTrack(const genfit::MeasuredStateOnPlane& aGFState) const
{
  const int id = m_raveTracks.size();
  TVector3 pos;
  TVector3 mom;
  TMatrixDSym cov;

  aGFState.getPosMomCov(pos, mom, cov);

  // state
  rave::Vector6D ravestate(pos.X(), pos.Y(), pos.Z(),
                           mom.X(), mom.Y(), mom.Z());

  rave::Covariance6D ravecov(cov(0, 0), cov(1, 0), cov(2, 0),
                             cov(1, 1), cov(2, 1), cov(2, 2),
                             cov(3, 0), cov(4, 0), cov(5, 0),
                             cov(3, 1), cov(4, 1), cov(5, 1),
                             cov(3, 2), cov(4, 2), cov(5, 2),
                             cov(3, 3), cov(4, 3), cov(5, 3),
                             cov(4, 4), cov(5, 4), cov(5, 5));

  return rave::Track(id, ravestate, ravecov, rave::Charge(aGFState.getCharge() + 0.1), 1, 1); //the two 1s are just dummy values. They are not used by Rave anyway

}

rave::Track RaveVertexFitter::TrackFitResultToRaveTrack(const TrackFitResult* const aTrackPtr) const
{
  const int id = m_raveTracks.size();

  TVector3 pos = aTrackPtr->getPosition();
  TVector3 mom = aTrackPtr->getMomentum();
  TMatrixF cov(aTrackPtr->getCovariance6());


  // state
  rave::Vector6D ravestate(pos.X(), pos.Y(), pos.Z(),
                           mom.X(), mom.Y(), mom.Z());

  rave::Covariance6D ravecov(cov(0, 0), cov(1, 0), cov(2, 0),
                             cov(1, 1), cov(2, 1), cov(2, 2),
                             cov(3, 0), cov(4, 0), cov(5, 0),
                             cov(3, 1), cov(4, 1), cov(5, 1),
                             cov(3, 2), cov(4, 2), cov(5, 2),
                             cov(3, 3), cov(4, 3), cov(5, 3),
                             cov(4, 4), cov(5, 4), cov(5, 5));

  return rave::Track(id, ravestate, ravecov, rave::Charge(aTrackPtr->getChargeSign()), 1, 1); //the two 1s are just dummy values. They are not used by Rave anyway

}


void RaveVertexFitter::addTrack(const Particle* aParticlePtr)
{
  const int id = m_raveTracks.size();
  const TVector3& pos = aParticlePtr->getVertex();
  const TVector3& mom = aParticlePtr->getMomentum();
  rave::Vector6D ravestate(pos.X(), pos.Y(), pos.Z(), mom.X(), mom.Y(), mom.Z());

  const TMatrixFSym& cov = aParticlePtr->getMomentumVertexErrorMatrix();

  rave::Covariance6D ravecov(cov(4, 4), cov(4, 5), cov(4, 6),
                             cov(5, 5), cov(5, 6), cov(6, 6),
                             cov(0, 4), cov(1, 4), cov(2, 4),
                             cov(0, 5), cov(1, 5), cov(2, 5),
                             cov(0, 6), cov(1, 6), cov(2, 6),
                             cov(0, 0), cov(0, 1), cov(0, 2),
                             cov(1, 1), cov(1, 2), cov(2, 2));

  m_raveTracks.push_back(rave::Track(id, ravestate, ravecov, rave::Charge(aParticlePtr->getCharge() + 0.1), 1, 1)); // 1 and 1 are dummy values for chi2 and ndf. the are not used for the vertex fit
}

void RaveVertexFitter::addMother(const Particle* aMotherParticlePtr)
{
  vector<Particle*> daughters = aMotherParticlePtr->getDaughters();

  int nDaughters = daughters.size();
  for (int i = 0; i not_eq nDaughters; ++i) {
    addTrack(daughters[i]);
  }

}

int RaveVertexFitter::fit(string options)
{
  //std::cerr << "RaveVertexFitter::fit(string options)" << std::endl;
  //std::cerr << "m_useBeamSpot " << m_useBeamSpot << std::endl;
  if (options == "default") {
    options = "kalman";
  }
  int ndf = 0;

  ndf = 2 * m_raveTracks.size();

  if (m_useBeamSpot == true) {
    ndf += 3;
  }
  if (ndf < 4) {
    return -1;
  }
  int nOfVertices = -100;

  if (m_useBeamSpot == true) {
    const TVector3& bsPos = RaveSetup::getRawInstance()->m_beamSpot;
    const TMatrixDSym& bsCov = RaveSetup::getRawInstance()->m_beamSpotCov;
    const rave::Covariance3D bsCovRave(bsCov(0, 0), bsCov(0, 1), bsCov(0, 2), bsCov(1, 1), bsCov(1, 2), bsCov(2, 2));
    RaveSetup::getRawInstance()->m_raveVertexFactory->setBeamSpot(rave::Ellipsoid3D(rave::Point3D(bsPos.X(), bsPos.Y(), bsPos.Z()), bsCovRave));
  }
  //std::cerr << "now fitting with m_raveVertexFactory" << std::endl;
  RaveSetup::getRawInstance()->m_raveVertexFactory->setDefaultMethod(options);
  m_raveVertices = RaveSetup::getRawInstance()->m_raveVertexFactory->create(m_raveTracks, m_useBeamSpot);
  nOfVertices = m_raveVertices.size();

  return nOfVertices;
}

void RaveVertexFitter::isVertexIdValid(const VecSize vertexId) const
{

  if (m_raveVertices.empty()) {
    B2ERROR("There is no fitted Vertex. Maybe you did not call fit() or maybe the fit was not successful");
    throw;
  }
  if (vertexId >= m_raveVertices.size()) {
    B2ERROR("The Vertex id " << vertexId << " does not correspond to any fitted vertex");
    throw;
  }

}

TVector3 RaveVertexFitter::getPos(VecSize vertexId) const
{
  isVertexIdValid(vertexId);

  return TVector3(m_raveVertices[vertexId].position().x(), m_raveVertices[vertexId].position().y(), m_raveVertices[vertexId].position().z());

}

double RaveVertexFitter::getWeight(int trackId, VecSize vertexId)const
{
  isVertexIdValid(vertexId);


  const std::vector < std::pair < float, rave::Track > >& weightedTracks = m_raveVertices[vertexId].weightedTracks();
  for (unsigned int i = 0; i not_eq weightedTracks.size(); ++i) {
    if (weightedTracks[i].second.id() == trackId) {
//          std::cerr << "returing weight for track with x coord: " <<weightedTracks[i].second.state().x() << std::endl;
      return weightedTracks[i].first;
    }
  }
  return -1;

}

std::vector<int> RaveVertexFitter::getTrackIdsForOneVertex(VecSize vertexId) const
{

  const std::vector < std::pair < float, rave::Track > >& weightedTracks = m_raveVertices[vertexId].weightedTracks();
  const int n = weightedTracks.size();
  vector<int> trackIds(n);
  for (int i = 0; i not_eq n; ++i) {
    trackIds[i] = weightedTracks[i].second.id();
  }
  return trackIds;
}

double RaveVertexFitter::getPValue(VecSize vertexId) const
{
  isVertexIdValid(vertexId);

  return ROOT::Math::chisquared_cdf_c(m_raveVertices[vertexId].chiSquared(), m_raveVertices[vertexId].ndf());

}

double RaveVertexFitter::getNdf(VecSize vertexId) const
{
  isVertexIdValid(vertexId);

  return m_raveVertices[vertexId].ndf();

}

double RaveVertexFitter::getChi2(VecSize vertexId) const
{
  isVertexIdValid(vertexId);

  return m_raveVertices[vertexId].chiSquared();

}

TMatrixDSym RaveVertexFitter::getCov(VecSize vertexId) const
{
  isVertexIdValid(vertexId);

  TMatrixDSym Cov(3);
  Cov(0, 0) = m_raveVertices[vertexId].error().dxx();
  Cov(0, 1) = m_raveVertices[vertexId].error().dxy();
  Cov(0, 2) = m_raveVertices[vertexId].error().dxz();
  Cov(1, 1) = m_raveVertices[vertexId].error().dyy();
  Cov(1, 2) = m_raveVertices[vertexId].error().dyz();
  Cov(2, 2) = m_raveVertices[vertexId].error().dzz();
  Cov(1, 0) = Cov(0, 1);
  Cov(2, 1) = Cov(1, 2);
  Cov(2, 0) = Cov(0, 2);

  return Cov;

}
