/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <mdst/dataobjects/V0.h>

using namespace Belle2;
using namespace std;

V0::V0():
  m_trackIndexPositive(-1),
  m_trackIndexNegative(-1),
  m_trackFitResultIndexPositive(-1),
  m_trackFitResultIndexNegative(-1)
{}

V0::V0(const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairPositive,
       const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairNegative) :
  m_trackIndexPositive(trackPairPositive.first->getArrayIndex()),
  m_trackIndexNegative(trackPairNegative.first->getArrayIndex()),
  m_trackFitResultIndexPositive(trackPairPositive.second->getArrayIndex()),
  m_trackFitResultIndexNegative(trackPairNegative.second->getArrayIndex())
{}

Const::ParticleType V0::getV0Hypothesis() const
{
  StoreArray<TrackFitResult> trackFitResults{};
  const auto posParticleType = trackFitResults[m_trackFitResultIndexPositive]->getParticleType();
  const auto negParticleType = trackFitResults[m_trackFitResultIndexNegative]->getParticleType();

  if ((posParticleType == Const::pion) && (negParticleType == Const::pion)) {
    return Const::Kshort;
  }
  if ((posParticleType == Const::proton) && (negParticleType == Const::pion)) {
    return Const::Lambda;
  }
  if ((posParticleType == Const::pion) && (negParticleType == Const::proton)) {
    return Const::antiLambda;
  }
  if ((posParticleType == Const::electron) && (negParticleType == Const::electron)) {
    return Const::photon;
  }
  return Const::unspecifiedParticle;
}
