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
#include <framework/logging/Logger.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::Tracking;



void Belle2::Tracking::ClusterInfo::addTrackCandidate(VXDTFTrackCandidate* aTC)
{
  BOOST_FOREACH(VXDTFTrackCandidate * anotherTC, m_attachedTCs) {
    if (isSameTC(aTC, anotherTC) == true) { return; }
  }
  m_attachedTCs.push_back(aTC);
}


bool Belle2::Tracking::ClusterInfo::isOverbooked()
{
  int nTCs = m_attachedTCs.size(), countIterations = 0;
  TcContainer::iterator tcIta = m_attachedTCs.begin();
  bool isOverbooked = false;
  for (; tcIta != m_attachedTCs.end(); ++tcIta) {
    if ((*tcIta)->getCondition() == false) { continue; }
    TcContainer::iterator tcItb = tcIta;
    for (++tcItb; tcItb != m_attachedTCs.end(); ++tcItb) {
      if ((*tcItb)->getCondition() == false) { continue; }

      if (isSameTC((*tcIta), (*tcItb)) == false) {
        (*tcIta)->addBookingRival((*tcItb));
        (*tcItb)->addBookingRival((*tcIta));
        isOverbooked = true;
      }
    }
    countIterations++;
    if (countIterations > nTCs) { B2FATAL("ClusterInfo::isOverbooked(): iterator crash! nIterations: " << countIterations << ", nTCs: " << nTCs) }
  }
  B2DEBUG(150, " Cluster at ownIndex " << m_ownPositionInIndex << " has got total/alive" << nTCs << "/" << countIterations << " track candidates stored...")
  if (isOverbooked == true && m_reserved == true) {
    B2WARNING(" Cluster at ownIndex " << m_ownPositionInIndex << " has got total/alive" << nTCs << "/" << countIterations << " track candidates stored... and is reserved: " << m_reserved)
  }
  return isOverbooked;
}


bool Belle2::Tracking::ClusterInfo::setReserved(VXDTFTrackCandidate* newBossTC)
{
  if (newBossTC == NULL) { B2WARNING("ClusterInfo::setReserved: you gave me a NULL pointer! ") return false; }
  if (newBossTC->getCondition() == false) {
    B2WARNING("ClusterInfo::setReserved: the new boss you sent is already dead! It had the tracknumber: " << newBossTC->getTrackNumber())
    return false;
  }
  if (m_bossTC != NULL) {
    if (newBossTC != m_bossTC) {
      B2WARNING("ClusterInfo::setReserved: the cluster withOwnID (" << m_ownPositionInIndex << ") is already reserved by: " << m_bossTC->getTrackNumber() << ", new Boss has " << newBossTC->getTrackNumber() << " and is not the same!")
      return false;
    }
    B2INFO("ClusterInfo::setReserved: the cluster withOwnID (" << m_ownPositionInIndex << ") was already reserved by that TC: " << m_bossTC->getTrackNumber() << ", you have sent now " << newBossTC->getTrackNumber() << " which is the same, old reserved-state(" << m_reserved << ")!")
    m_reserved = true;
    return true;
  }
  BOOST_FOREACH(VXDTFTrackCandidate * aTC, m_attachedTCs) {
    if (aTC->getCondition() == false) { continue; }
    if (isSameTC(aTC, newBossTC) == false) { aTC->setCondition(false); }
  }
  m_bossTC = newBossTC;
  m_reserved = true;
  return true;
}


bool ClusterInfo::checkReserved(VXDTFTrackCandidate* newBossTC) const
{
  if (newBossTC == NULL) { B2WARNING("ClusterInfo::checkReserved: you gave me a NULL pointer! ") return false; }
  if ((m_reserved == false) && (m_bossTC == NULL)) { return false; }
  if (m_reserved == false && m_bossTC != NULL) {
    if (newBossTC == m_bossTC) {
      if (newBossTC->getCondition() == false) {
        B2WARNING("ClusterInfo::checkReserved: he new boss you sent with trackNum " << m_bossTC->getTrackNumber() << " is already dead! You head it already reserved per pointer and m_reserved is " << m_reserved)
        return false;
      }
      B2INFO("ClusterInfo::checkReserved: the Cluster withOwnID (" << m_ownPositionInIndex << ") was already pointing to new boss (trackNumber: " << m_bossTC->getTrackNumber() << ") but reserved was false, you gave me number " << newBossTC->getTrackNumber() << " which is the same as before, please start again with setReserved()")
      return false;
    }
    B2WARNING("ClusterInfo::checkReserved: the new boss you sent is already dead! It had the tracknumber: " << newBossTC->getTrackNumber())
  }
  // reserved == true:

  if (newBossTC != m_bossTC && m_bossTC != NULL) {
    B2INFO("ClusterInfo::checkReserved: the cluster withOwnID (" << m_ownPositionInIndex << ") is already reserved by: " << m_bossTC->getTrackNumber() << ", new Boss has " << newBossTC->getTrackNumber() << " and is not the same")
    return false;
  }
  return true;

}