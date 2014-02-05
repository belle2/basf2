/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCObservations2D.h"

using namespace std;
using namespace Eigen;

using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCObservations2D)


CDCObservations2D::CDCObservations2D()
{
}


CDCObservations2D::~CDCObservations2D()
{
}


void CDCObservations2D::append(const Vector2D& pos2D)
{

  m_observations.push_back(pos2D.x());
  m_observations.push_back(pos2D.y());
  m_observations.push_back(0.0);

}

void CDCObservations2D::append(const CDCWireHit& wireHit)
{

  const Vector2D& pos2D = wireHit.getRefPos2D();

  m_observations.push_back(pos2D.x());
  m_observations.push_back(pos2D.y());
  m_observations.push_back(wireHit.getRefDriftLength());

}

void CDCObservations2D::append(const CDCRLWireHit& rlWireHit)
{

  const Vector2D& pos2D = rlWireHit.getRefPos2D();

  m_observations.push_back(pos2D.x());
  m_observations.push_back(pos2D.y());
  m_observations.push_back(rlWireHit.getSignedRefDriftLength());

}


size_t CDCObservations2D::getNObservationsWithDriftRadius() const
{
  // Obtain an iterator an advance it to the first drift radius
  std::vector<FloatType>::const_iterator itDriftRadius = m_observations.begin();
  std::advance(itDriftRadius, 2);

  size_t result = 0;

  //Every thrid element is a drift radius
  for (; itDriftRadius < m_observations.end(); std::advance(itDriftRadius, 3)) {

    bool hasDriftRadius = (*itDriftRadius != 0.0);
    result += hasDriftRadius ? 1 : 0;

  }
  return result;
}





Map< Matrix< FloatType, Dynamic, Dynamic, RowMajor > > CDCObservations2D::getObservationMatrix()
{

  size_t nObservations = m_observations.size() / 3;
  FloatType* rawObservations = &(m_observations.front());
  Map< Matrix< FloatType, Dynamic, Dynamic, RowMajor > > eigenObservations(rawObservations, nObservations, 3);
  return eigenObservations;

}
