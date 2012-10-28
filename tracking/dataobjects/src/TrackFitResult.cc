/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/dataobjects/TrackFitResult.h>

using namespace Belle2;

ClassImp(TrackFitResult);

TrackFitResult::TrackFitResult()
{
}

TrackFitResult::TrackFitResult(const GFAbsTrackRep& gfAbsTrackRep)
{
}

// This class should be able to give back Helix information either in Perigee Parametrisation
// or as starting position + momentum.
TVector3 TrackFitResult::getPosition()
{
}

TVector3 TrackFitResult::getMomentum() {}

TMatrixF TrackFitResult::getCovariance6() {}

float TrackFitResult::getD0()  const {}
float TrackFitResult::getPhi() const {}
float TrackFitResult::getOmega() const {}
float TrackFitResult::getZ0() const {}
float TrackFitResult::getCotTheta() const {}

bool TrackFitResult:: hitInSuperLayer(unsigned int iSuperLayer) const {}


// Implementation note: Typically we will either have PXD or SVD hits,
// so it makes a lot of sense to simply start inside and test going further outside.
// Case without hits should never happen.
unsigned short TrackFitResult::getIInnermostLayer() const
{
  unsigned short ii = 0;
  while (!m_hitPattern[ii++]) {}
  return ii;
}

ParticleCode TrackFitResult::getParticleCode() const
{
  return ParticleCode(m_pdg);
}
