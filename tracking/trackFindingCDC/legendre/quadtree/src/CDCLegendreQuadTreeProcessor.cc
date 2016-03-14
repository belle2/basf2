/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Thomas Hauth                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeProcessor.h>

#include <algorithm>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

double QuadTreeProcessor::rThreshold;
unsigned int QuadTreeProcessor::nHitsThreshold;
