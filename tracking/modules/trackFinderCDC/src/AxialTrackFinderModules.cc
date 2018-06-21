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
#include <tracking/modules/trackFinderCDC/AxialTrackFinderModules.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <mdst/dataobjects/ECLCluster.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_AxialTrackFinderLegendre);
REG_MODULE(TFCDC_AxialTrackFinderHough);
REG_MODULE(TFCDC_AxialTrackCreatorSegmentHough);
REG_MODULE(TFCDC_AxialTrackCreatorMCTruth);
REG_MODULE(TFCDC_MonopoleAxialTrackFinderLegendre);
REG_MODULE(TFCDC_AxialStraightTrackFinder);

TFCDC_AxialTrackFinderLegendreModule::TFCDC_AxialTrackFinderLegendreModule()
  : Super( {"CDCWireHitVector", "CDCTrackVector"})
{
}

TFCDC_AxialTrackFinderHoughModule::TFCDC_AxialTrackFinderHoughModule()
  : Super( {"CDCWireHitVector", "CDCTrackVector"})
{
}

TFCDC_AxialTrackCreatorSegmentHoughModule::TFCDC_AxialTrackCreatorSegmentHoughModule()
  : Super( {"CDCSegment2DVector", "CDCTrackVector"})
{
}

TFCDC_AxialTrackCreatorMCTruthModule::TFCDC_AxialTrackCreatorMCTruthModule()
  : Super( {"CDCWireHitVector", "CDCTrackVector"})
{
}

TFCDC_MonopoleAxialTrackFinderLegendreModule::TFCDC_MonopoleAxialTrackFinderLegendreModule()
  : Super( {"CDCWireHitVector", "CDCTrackVector"})
{
}

TFCDC_AxialStraightTrackFinderModule::TFCDC_AxialStraightTrackFinderModule()
  : Super( {"CDCWireHitVector", "CDCTrackVector"})
{
}
