/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/cdcLegendreTracking/filter/CDCLegendreFilterCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/filter/CDCLegendreSimpleFilter.h>

#include <cstdlib>
#include <list>
#include <map>

using namespace std;

using namespace Belle2;

CDCLegendreFilterCandidate::CDCLegendreFilterCandidate(CDCLegendreTrackCandidate* trackCandidate) :
  m_trackProbability(1.), m_trackCandidate(trackCandidate)
{

}


CDCLegendreFilterCandidate::~CDCLegendreFilterCandidate()
{
  m_trackHitsProbability.clear();
  m_trackHits.clear();
}

void CDCLegendreFilterCandidate::addHit(CDCLegendreTrackHit* hit, double prob)
{
  if (m_trackHitsProbability.count(hit) > 0) {
    B2WARNING("Attempt to push duplicate of CDCLegendreTrackHit object in CDCLegendreFilterCandidate object!")
  } else {
    m_trackHitsProbability.insert(std::make_pair(hit, prob));
    m_trackProbability *= prob;
    m_trackHits.push_back(hit);
  }

}


void CDCLegendreFilterCandidate::removeHit(CDCLegendreTrackHit* hit)
{
  if (m_trackHitsProbability.count(hit) == 0) {
    B2WARNING("Couldn't find instance of given CDCLegendreTrackHit object in CDCLegendreFilterCandidate object! Nothing to remove.")
  } else {
    m_trackProbability = m_trackProbability / m_trackHitsProbability.find(hit)->second;
    m_trackHitsProbability.erase(hit);
    m_trackHits.erase(remove(m_trackHits.begin(), m_trackHits.end(), hit), m_trackHits.end());
  }

}


void CDCLegendreFilterCandidate::setHitProbability(CDCLegendreTrackHit* hit, double prob)
{
  if (m_trackHitsProbability.count(hit) == 0) {
    B2WARNING("Couldn't find instance of given CDCLegendreTrackHit object in CDCLegendreFilterCandidate object! Nothing to change.")
  } else {
    m_trackHitsProbability.find(hit)->second = prob;
  }
}
