/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/dataobjects/TrackFitResult.h>
#include <GFAbsTrackRep.h>

using namespace Belle2;

ClassImp(TrackFitResult);

TrackFitResult::TrackFitResult() : m_pdg(0), m_pValue(0), m_charge(0)
{
}

// This class should be able to give back Helix information either in Perigee Parametrisation
// or as starting position + momentum.
TVector3 TrackFitResult::getPosition() const
{
  return m_position;
}
void TrackFitResult::setPosition(const TVector3& position)
{
  m_position = position;
}



TVector3 TrackFitResult::getMomentum() const
{
  return m_momentum;
}

void TrackFitResult::setMomentum(const TVector3& momentum)
{
  m_momentum = momentum;
}

TMatrixF TrackFitResult::getCovariance6() const
{
  return m_cov;
}

void TrackFitResult::setCovariance6(const TMatrixF& covariance)
{
  m_cov.ResizeTo(6, 6);
  m_cov = covariance;
}


/*
float TrackFitResult::getD0()  const {}
float TrackFitResult::getPhi() const {}
float TrackFitResult::getOmega() const {}
float TrackFitResult::getZ0() const {}
float TrackFitResult::getCotTheta() const {}

//bool TrackFitResult:: hitInSuperLayer(unsigned int iSuperLayer) const {}
*/


// Implementation note: Typically we will either have PXD or SVD hits,
// so it makes a lot of sense to simply start inside and test going further outside.
// Case without hits should never happen.
/*unsigned short TrackFitResult::getIInnermostLayer() const
{
  unsigned short ii = 0;
  while (!m_hitPattern[ii++]) {}
  return ii;
}
*/
