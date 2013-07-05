/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// #include "../include/VXDTFTrackCandidate.h"
#include "../include/ClusterInfo.h"

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;



void Belle2::Tracking::ClusterInfo::addTrackCandidate(VXDTFTrackCandidate* aTC)
{
  int ctr = 0;
  BOOST_FOREACH(VXDTFTrackCandidate * anotherTC, m_attachedTCs) {
    if (isSameTC(aTC, anotherTC) == true) { ++ctr; break; }
  }
  if (ctr == 0) { m_attachedTCs.push_back(aTC); }
}


bool Belle2::Tracking::ClusterInfo::isOverbooked()
{
  int ctr = 0;
  vector<VXDTFTrackCandidate*> overbookedOnes;
  BOOST_FOREACH(VXDTFTrackCandidate * aTC, m_attachedTCs) {
    if (aTC->getCondition() == false) { continue; }
    ++ctr;
    overbookedOnes.push_back(aTC);
  }
  if (ctr > 1) {
    BOOST_FOREACH(VXDTFTrackCandidate * aTC, overbookedOnes) {
//      aTC->clearRivals();
      BOOST_FOREACH(VXDTFTrackCandidate * bTC, overbookedOnes) {
        if (isSameTC(aTC, bTC) == false) { aTC->addBookingRival(bTC); }
      }
    }
    return true;
  } else { return false; }
}


void Belle2::Tracking::ClusterInfo::setReserved(VXDTFTrackCandidate* newBossTC)
{
  BOOST_FOREACH(VXDTFTrackCandidate * aTC, m_attachedTCs) {
    if (aTC->getCondition() == false) { continue; }
    if (isSameTC(aTC, newBossTC) == false) { aTC->setCondition(false); }
  }
  m_reserved = true;
}
