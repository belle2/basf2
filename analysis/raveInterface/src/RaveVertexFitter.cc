/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/raveInterface/RaveVertexFitter.h>
#include <analysis/raveInterface/RaveSetup.h>

#include <GFRaveVertexFactory.h>
#include <RKTrackRep.h>

//#include <CLHEP/Matrix/SymMatrix.h>

using std::string;

#include <vector>
using std::vector;

using namespace Belle2;
using namespace analysis;



RaveVertexFitter::RaveVertexFitter(): m_chi2(-1.0), m_ndf(-1), m_fitStatus(-1), m_useBeamSpot(false), m_raveAlgorithm("")
{
  if (RaveSetup::s_instance == NULL) {
    B2FATAL("RaveSetup::initialize was not called. It has to be called before RaveSetup or RaveVertexFitter are used");
  }
  m_useBeamSpot = RaveSetup::s_instance->m_useBeamSpot;
}

RaveVertexFitter::~RaveVertexFitter()
{
  //destroy all the temporary trackreps created in this class but do not destroy the pointers coming from outside
  int n = m_ownGfTrackReps.size();
  for (int i = 0; i not_eq n; ++i) {
    delete m_ownGfTrackReps[i];
  }

  n = m_GFRaveVertices.size();
  for (int i = 0; i not_eq n; ++i) {
    delete m_GFRaveVertices[i];
  }
}


void RaveVertexFitter::addTrack(GFTrack& aGFTrack)
{
  GFAbsTrackRep* const aGFTrackRepPtr = aGFTrack.getCardinalRep();
  if (RaveSetup::s_instance->m_gfRave == true) {
    m_gfTrackReps.push_back(aGFTrackRepPtr);
  } else {
    m_raveTracks.push_back(GFTrackRepToRaveTrack(aGFTrackRepPtr));
  }

}


void RaveVertexFitter::addTrack(GFTrack* const aGFTrackPtr)
{
  GFAbsTrackRep* const aGFTrackRepPtr = aGFTrackPtr->getCardinalRep();
  if (RaveSetup::s_instance->m_gfRave == true) {
    m_gfTrackReps.push_back(aGFTrackRepPtr);
  } else {
    m_raveTracks.push_back(GFTrackRepToRaveTrack(aGFTrackRepPtr));
  }

}

void RaveVertexFitter::addTrack(GFAbsTrackRep* const aTrackRepPtr)
{
  if (RaveSetup::s_instance->m_gfRave == true) {
    m_gfTrackReps.push_back(aTrackRepPtr);
  } else {
    m_raveTracks.push_back(GFTrackRepToRaveTrack(aTrackRepPtr));
  }

}

rave::Track RaveVertexFitter::GFTrackRepToRaveTrack(GFAbsTrackRep* const aGFTrackRepPtr) const
{
  const int id = m_raveTracks.size();

  const GFDetPlane& refPlane(aGFTrackRepPtr->getReferencePlane());
  TVector3 pos, mom;
  TMatrixDSym cov;

  aGFTrackRepPtr->getPosMomCov(refPlane, pos, mom, cov);

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

  return rave::Track(id, ravestate, ravecov, aGFTrackRepPtr->getCharge(), aGFTrackRepPtr->getChiSqu(), aGFTrackRepPtr->getNDF());

}

//void RaveVertexFitter::addTrack( const Belle1::Particle& aParticle){
//  Hep3Vector pos = aParticle.momentum().x();
//  TVector3 posRoot(pos.x(),pos.y(),pos.z());
//  HepLorentzVector mom = aParticle.momentum().p();
//  TVector3 momRoot(mom.px(), mom.py(), mom.pz());
//  //std::cerr << "RaveVertexFitter::addTrack\n"; momRoot.Print();
//  CLHEP::HepSymMatrix cov = aParticle.momentum().dpx(); // format of dpx is px,py,pz,E,x,y,z
//  TMatrixDSym covRoot(6);
//  covRoot(0,0) = cov[4][4]; //x
//  covRoot(1,1) = cov[5][5]; //y
//  covRoot(2,2) = cov[6][6]; //z
//  covRoot(3,3) = cov[0][0]; //px
//  covRoot(4,4) = cov[1][1]; //py
//  covRoot(5,5) = cov[2][2]; //pz
//  covRoot(0,1) = cov[4][5]; //x,y
//  covRoot(1,0) = covRoot(0,1); //x,y
//  covRoot(0,2) = cov[4][6]; //x,z
//  covRoot(2,0) = covRoot(0,2); //x,z
//  covRoot(1,2) = cov[5][6]; //y,z
//  covRoot(2,1) = covRoot(1,2); //y,z
//  covRoot(0,3) = cov[0][4]; //x,px
//  covRoot(3,0) = covRoot(0,3); //x,px
//  covRoot(0,4) = cov[1][4]; //x,py
//  covRoot(4,0) = covRoot(0,4); //x,py
//  covRoot(0,5) = cov[2][4]; //x,pz
//  covRoot(5,0) = covRoot(0,5); //x,pz
//  covRoot(1,3) = cov[0][5]; //y,px
//  covRoot(3,1) = covRoot(1,3); //y,px
//  covRoot(1,4) = cov[1][5]; //y,py
//  covRoot(4,1) = covRoot(1,4); //y,py
//  covRoot(1,5) = cov[2][5]; //y,pz
//  covRoot(5,1) = covRoot(1,5); //y,pz
//  covRoot(2,3) = cov[0][6]; //z,px
//  covRoot(3,2) = covRoot(2,3); //z,px
//  covRoot(2,4) = cov[1][6]; //z,py
//  covRoot(4,2) = covRoot(2,4); //z,py
//  covRoot(2,5) = cov[2][6]; //z,pz
//  covRoot(5,2) = covRoot(2,5); //z,py
//  covRoot(3,4) = cov[0][1]; //px,py
//  covRoot(4,3) = covRoot(3,4); //px,py
//  covRoot(3,5) = cov[0][2]; //px,pz
//  covRoot(5,3) = covRoot(3,5); //px,pz
//  covRoot(4,5) = cov[1][2]; //py,pz
//  covRoot(5,4) = covRoot(4,5); //py,pz
//
//
//  RKTrackRep* aTrackRepPtr = new RKTrackRep(posRoot,momRoot,covRoot,aParticle.lund());
//  m_ownGfTrackReps.push_back(static_cast<GFAbsTrackRep*>(aTrackRepPtr));
//  m_gfTrackReps.push_back(static_cast<GFAbsTrackRep*>(aTrackRepPtr));
//}


void RaveVertexFitter::addTrack(const Particle& aParticle)
{
  if (RaveSetup::s_instance->m_gfRave == true) {
    TMatrixFSym cov = aParticle.getMomentumVertexErrorMatrix();

    TMatrixDSym covRoot(6);
    covRoot(0, 0) = cov[4][4]; //x
    covRoot(1, 1) = cov[5][5]; //y
    covRoot(2, 2) = cov[6][6]; //z
    covRoot(3, 3) = cov[0][0]; //px
    covRoot(4, 4) = cov[1][1]; //py
    covRoot(5, 5) = cov[2][2]; //pz
    covRoot(0, 1) = cov[4][5]; //x,y
    covRoot(1, 0) = covRoot(0, 1); //x,y
    covRoot(0, 2) = cov[4][6]; //x,z
    covRoot(2, 0) = covRoot(0, 2); //x,z
    covRoot(1, 2) = cov[5][6]; //y,z
    covRoot(2, 1) = covRoot(1, 2); //y,z
    covRoot(0, 3) = cov[0][4]; //x,px
    covRoot(3, 0) = covRoot(0, 3); //x,px
    covRoot(0, 4) = cov[1][4]; //x,py
    covRoot(4, 0) = covRoot(0, 4); //x,py
    covRoot(0, 5) = cov[2][4]; //x,pz
    covRoot(5, 0) = covRoot(0, 5); //x,pz
    covRoot(1, 3) = cov[0][5]; //y,px
    covRoot(3, 1) = covRoot(1, 3); //y,px
    covRoot(1, 4) = cov[1][5]; //y,py
    covRoot(4, 1) = covRoot(1, 4); //y,py
    covRoot(1, 5) = cov[2][5]; //y,pz
    covRoot(5, 1) = covRoot(1, 5); //y,pz
    covRoot(2, 3) = cov[0][6]; //z,px
    covRoot(3, 2) = covRoot(2, 3); //z,px
    covRoot(2, 4) = cov[1][6]; //z,py
    covRoot(4, 2) = covRoot(2, 4); //z,py
    covRoot(2, 5) = cov[2][6]; //z,pz
    covRoot(5, 2) = covRoot(2, 5); //z,py
    covRoot(3, 4) = cov[0][1]; //px,py
    covRoot(4, 3) = covRoot(3, 4); //px,py
    covRoot(3, 5) = cov[0][2]; //px,pz
    covRoot(5, 3) = covRoot(3, 5); //px,pz
    covRoot(4, 5) = cov[1][2]; //py,pz
    covRoot(5, 4) = covRoot(4, 5); //py,pz
    RKTrackRep* aTrackRepPtr = new RKTrackRep(aParticle.getVertex(), aParticle.getMomentum(), covRoot, aParticle.getPDGCode());
    m_ownGfTrackReps.push_back(static_cast<GFAbsTrackRep*>(aTrackRepPtr));
    m_gfTrackReps.push_back(static_cast<GFAbsTrackRep*>(aTrackRepPtr));
  } else {

  }


}

//void RaveVertexFitter::addTrack( const Track& aTrack){
//
//
//  RKTrackRep* aTrackRepPtr = new RKTrackRep();
//  aTrackRepPtr->setPosMomCov(aTrack,aTrack,aTrack.getErrorMatrix());
//  aTrackRepPtr->setPDG(aTrack.getPDG());
//  m_ownGfTrackReps.push_back(static_cast<GFAbsTrackRep*>(aTrackRepPtr));
//}


int RaveVertexFitter::fit(string options)
{

  if (options == "default") {
    m_raveAlgorithm = "kalman";
  } else {
    m_raveAlgorithm = options;
  }
  if (RaveSetup::s_instance->m_gfRave == true) {
    m_ndf = 2 * m_gfTrackReps.size();
  } else {
    m_ndf = 2 * m_raveTracks.size();
  }



  if (m_useBeamSpot == true) {
    m_ndf += 3;
  }

  if (m_ndf < 4) {
    return -1;
  }
  int nOfVertices = -100;
  if (RaveSetup::s_instance->m_gfRave == true) {

    //    for( int i = 0; i not_eq m_gfTrackReps.size(); ++i){
    //      m_gfTrackReps[i]->Print();
    //    }

    GFRaveVertexFactory vertexFactory(RaveSetup::s_instance->m_raveVerbosity, RaveSetup::s_instance->m_gfPropagation);
    vertexFactory.setMethod(m_raveAlgorithm);
    if (m_useBeamSpot == true) {
      vertexFactory.setBeamspot(RaveSetup::s_instance->m_beamSpot, RaveSetup::s_instance->m_beamSpotCov);
    }

    vertexFactory.findVertices(&m_GFRaveVertices, m_gfTrackReps, m_useBeamSpot);
    nOfVertices = m_GFRaveVertices.size();
  } else {

  }

  return nOfVertices;
}
