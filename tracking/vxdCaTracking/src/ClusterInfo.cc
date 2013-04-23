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


bool ClusterInfo::isSameTC(const VXDTFTrackCandidate* a1, const VXDTFTrackCandidate* a2)
{
  if (a1 == a2) { return true; } else { return false; }
}

void ClusterInfo::addTrackCandidate(VXDTFTrackCandidate* aTC)
{
  int ctr = 0;
  BOOST_FOREACH(VXDTFTrackCandidate * anotherTC, m_attachedTCs) {
    if (isSameTC(aTC, anotherTC) == true) { ++ctr; }
  }
  if (ctr == 0) { m_attachedTCs.push_back(aTC); }
}

// void ClusterInfo::deleteTrackCandidate ( VXDTFTrackCandidate& aTC ) {
//  int ctr = 0;
//  BOOST_FOREACH (VXDTFTrackCandidate& anotherTC, m_attachedTCs) {
//    if ( isSameTC(aTC, anotherTC) == true ) {
//
//    }
//  }
//  if ( ctr != 0 ) { m_attachedTCs.push_back(aTC); }
// }

bool ClusterInfo::isOverbooked()
{
  int ctr = 0;
  vector<VXDTFTrackCandidate*> overbookedOnes;
  BOOST_FOREACH(VXDTFTrackCandidate * aTC, m_attachedTCs) {
    if (aTC->getCondition() == true) {
      ++ctr;
      overbookedOnes.push_back(aTC);
    }
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
