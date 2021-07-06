/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/SegmentAutomataModules.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_TrackFinderSegmentPairAutomaton);
REG_MODULE(TFCDC_TrackFinderSegmentTripleAutomaton);

REG_MODULE(TFCDC_TrackCreatorSingleSegments);
REG_MODULE(TFCDC_TrackCreatorSegmentPairAutomaton);
REG_MODULE(TFCDC_TrackCreatorSegmentTripleAutomaton);

REG_MODULE(TFCDC_SegmentPairCreator);
REG_MODULE(TFCDC_AxialSegmentPairCreator);
REG_MODULE(TFCDC_SegmentTripleCreator);

TFCDC_TrackFinderSegmentPairAutomatonModule::TFCDC_TrackFinderSegmentPairAutomatonModule()
  : Super( {"CDCSegment2DVector", "CDCTrackVector"})
{
}

TFCDC_TrackFinderSegmentTripleAutomatonModule::TFCDC_TrackFinderSegmentTripleAutomatonModule()
  : Super( {"CDCSegment2DVector", "CDCTrackVector"})
{
}
TFCDC_TrackCreatorSingleSegmentsModule::TFCDC_TrackCreatorSingleSegmentsModule()
  : Super( {"CDCSegment2DVector", "CDCTrackVector"})
{
}
TFCDC_TrackCreatorSegmentPairAutomatonModule::TFCDC_TrackCreatorSegmentPairAutomatonModule()
  : Super( {"CDCSegmentPairVector", "" /*to be set externally*/})
{
}

TFCDC_TrackCreatorSegmentTripleAutomatonModule::TFCDC_TrackCreatorSegmentTripleAutomatonModule()
  : Super( {"CDCSegmentTripleVector", "" /*to be set externally*/})
{
}

TFCDC_SegmentPairCreatorModule::TFCDC_SegmentPairCreatorModule()
  : Super( {"CDCSegment2DVector", "CDCSegmentPairVector"})
{
}
TFCDC_AxialSegmentPairCreatorModule::TFCDC_AxialSegmentPairCreatorModule()
  : Super( {"CDCSegment2DVector", "CDCAxialSegmentPairVector"})
{
}
TFCDC_SegmentTripleCreatorModule::TFCDC_SegmentTripleCreatorModule()
  : Super( {"CDCSegment2DVector", "CDCAxialSegmentVector", "CDCSegmentTripleVector"})
{
}
