/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/MCMatchParticle.h"

using namespace std;
using namespace Belle2;

ClassImp(MCMatchParticle)

MCMatchParticle::MCMatchParticle()
{
}

MCMatchParticle::MCMatchParticle(MCParticle *mcp)
{
  //copy some variables from MCParticle
  m_pdg = mcp->getPDG();
  m_mass = mcp->getMass();
  m_energy = mcp->getEnergy();
  m_momentum = mcp->getMomentum();

  //calculate some additional variables from MCParticle information
  m_momentumValue = sqrt((mcp->getMomentum().X() * mcp->getMomentum().X()) + (mcp->getMomentum().Y() * mcp->getMomentum().Y()) + (mcp->getMomentum().Z() * mcp->getMomentum().Z()));
  if (mcp->getMother()) m_primary = false;
  else m_primary = true;

  //set some initial values for other member variables
  m_nHits = 0;
  m_efficiency = 0;
  m_trackCandId = 0;

  m_recoMomentum.SetXYZ(0., 0., 0.);
  m_fitRecoMomentum.SetXYZ(0., 0., 0.);
  m_fitMCMomentum.SetXYZ(0., 0., 0.);


}

MCMatchParticle::~MCMatchParticle()
{
}

void MCMatchParticle::setNHits(int nHits)
{
  m_nHits = nHits ;
}


void MCMatchParticle::addTrackCandidate(int Id)
{

  bool already = false;
  for (unsigned int i = 0; i < m_trackCands.size(); i++) { //loop over trackCandidates which are already there

    if (m_trackCands.at(i).first == Id) { //if there is already an entry for this TrackCandidateId
      m_trackCands.at(i).second = m_trackCands.at(i).second + 1;  //increase the number of hits in this TrackCandidate by 1
      already = true;
    }
  }// end loop over trackCandidates

  if (already == false) {                 //if this is the first entry for this TrackCandidateId, create new entry
    pair <int, int> newEntry(Id, 1);
    m_trackCands.push_back(newEntry);
  }
}


void MCMatchParticle::evaluateMC()
{

  double max = 0;     //variable to mark the highest number of hits
  int indexMax = 0;   //variable to mart the index of the trackCandidate with the highest number of hits

  //Search for maximum, but only if there were some entries
  if (m_trackCands.size() != 0) {
    for (unsigned int i = 0; i < m_trackCands.size(); i++) {
      if (m_trackCands.at(i).second > max) {
        max = m_trackCands.at(i).second;
        indexMax = i;
      }
    }

    //Assign results
    double fraction = double(max) / double(m_nHits) * 100;
    m_efficiency = fraction;
    m_trackCandId = m_trackCands.at(indexMax).first;
  }

}

void MCMatchParticle::setRecoMomentum(TVector3 momentum)
{
  m_recoMomentum = momentum ;
}

void MCMatchParticle::setFitMCMomentum(TVector3 momentum)
{
  m_fitMCMomentum = momentum ;
}

void MCMatchParticle::setFitRecoMomentum(TVector3 momentum)
{
  m_fitRecoMomentum = momentum ;
}

void MCMatchParticle::setChi2(double chi2)
{
  m_fitChi2 = chi2 ;
}



