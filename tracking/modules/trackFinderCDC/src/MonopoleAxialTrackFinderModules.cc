/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun, Oliver Frost, Dmitrii Neverov                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/MonopoleAxialTrackFinderModules.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_MonopoleAxialTrackFinderLegendre);

TFCDC_MonopoleAxialTrackFinderLegendreModule::TFCDC_MonopoleAxialTrackFinderLegendreModule()
  : Super( {"CDCWireHitVector", "CDCTrackVector"})
{
}
