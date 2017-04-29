/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/TrackClusterSeparation.h>

using namespace std;
using namespace Belle2;

TrackClusterSeparation::TrackClusterSeparation() :
  m_Distance(1.0E10), // "infinity"
  m_TrackClusterAngle(0.0),
  m_TrackClusterInitialSeparationAngle(0.0),
  m_TrackClusterSeparationAngle(0.0),
  m_TrackRotationAngle(0.0)
{
}

