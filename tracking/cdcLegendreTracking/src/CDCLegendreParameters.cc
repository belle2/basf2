/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreParameters.h>

using namespace Belle2;

int CDCLegendreParameters::s_minTrackHits = 0;
bool CDCLegendreParameters::s_reconstructCurler = false;
bool CDCLegendreParameters::s_fitTracks = false;
bool CDCLegendreParameters::s_mergeTracks = false;
bool CDCLegendreParameters::s_appendHits = false;

CDCLegendreParameters* CDCLegendreParameters::s_cdcLegendreParameters = 0;
