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
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

ClassImp(V0);

V0::V0():
  m_trackIndexPositive(-1),
  m_trackIndexNegative(-1),
  m_trackFitResultIndexPiPlus(-1),
  m_trackFitResultIndexProton(-1),
  m_trackFitResultIndexPiMinus(-1),
  m_trackFitResultIndexAProton(-1)
{}

V0::V0(const std::pair<Track*, std::pair<TrackFitResult*, TrackFitResult*> >& trackPairPositive,
       const std::pair<Track*, std::pair<TrackFitResult*, TrackFitResult*> >& trackPairNegative) :
  m_trackIndexPositive(trackPairPositive.first->getArrayIndex()),
  m_trackIndexNegative(trackPairNegative.first->getArrayIndex()),
  m_trackFitResultIndexPiPlus(trackPairPositive.second.first->getArrayIndex()),
  m_trackFitResultIndexProton(trackPairPositive.second.second->getArrayIndex()),
  m_trackFitResultIndexPiMinus(trackPairNegative.second.first->getArrayIndex()),
  m_trackFitResultIndexAProton(trackPairNegative.second.second->getArrayIndex())

{}


V0::V0(const std::pair<Belle2::Track*, Belle2::TrackFitResult*>& trackPairPositive,
       const std::pair<Belle2::Track*, Belle2::TrackFitResult*>& trackPairNegative) :
  m_trackIndexPositive(trackPairPositive.first->getArrayIndex()),
  m_trackIndexNegative(trackPairNegative.first->getArrayIndex()),
  m_trackFitResultIndexPiPlus(trackPairPositive.second->getArrayIndex()),
  m_trackFitResultIndexProton(-1),
  m_trackFitResultIndexPiMinus(trackPairNegative.second->getArrayIndex()),
  m_trackFitResultIndexAProton(-1)
{
  B2WARNING("Deprecated, use the new constructor instead.")
}
