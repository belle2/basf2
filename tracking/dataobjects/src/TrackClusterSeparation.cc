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
  m_TrackIndex(0),
  m_Distance(1.0E10), // "infinity"
  m_Direction(0.0, 0.0, 1.0), // "parallel to z axis"
  m_TrackAngle(0.0)
{
}

