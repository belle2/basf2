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
  m_trackFitResultIndexPositive(-1),
  m_trackFitResultIndexNegative(-1)
{}
V0::V0(const pair<Track, TrackFitResult>& trackPairPositive,
       const pair<Track, TrackFitResult>& trackPairNegative) :
  m_trackIndexPositive(trackPairPositive.first.getArrayIndex()),
  m_trackIndexNegative(trackPairNegative.first.getArrayIndex()),
  m_trackFitResultIndexPositive(trackPairPositive.second.getArrayIndex()),
  m_trackFitResultIndexNegative(trackPairNegative.second.getArrayIndex())
{}
