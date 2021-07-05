/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

