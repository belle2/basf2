/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/SegmentFinderModules.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_SegmentFinderFacetAutomaton);

REG_MODULE(TFCDC_FacetCreator);
REG_MODULE(TFCDC_SegmentCreatorFacetAutomaton);
REG_MODULE(TFCDC_SegmentLinker);
REG_MODULE(TFCDC_SegmentFitter);
REG_MODULE(TFCDC_SegmentRejecter);
REG_MODULE(TFCDC_SegmentOrienter);
REG_MODULE(TFCDC_SegmentCreatorMCTruth);

TFCDC_SegmentFinderFacetAutomatonModule::TFCDC_SegmentFinderFacetAutomatonModule()
  : Super( {"CDCWireHitClusterVector", "CDCSegment2DVector"})
{
}

TFCDC_FacetCreatorModule::TFCDC_FacetCreatorModule()
  : Super( {"CDCWireHitClusterVector", "CDCFacetVector"})
{
}

TFCDC_SegmentCreatorFacetAutomatonModule::TFCDC_SegmentCreatorFacetAutomatonModule()
  : Super( {"CDCFacetVector", "CDCSegment2DVector"})
{
}

TFCDC_SegmentLinkerModule::TFCDC_SegmentLinkerModule()
  : Super( {"CDCSegment2DVector"})
{
}

TFCDC_SegmentOrienterModule::TFCDC_SegmentOrienterModule()
  : Super( {"CDCSegment2DVector"})
{
}

TFCDC_SegmentFitterModule::TFCDC_SegmentFitterModule()
  : Super( {"CDCSegment2DVector"})
{
}

TFCDC_SegmentRejecterModule::TFCDC_SegmentRejecterModule()
  : Super( {"CDCSegment2DVector"})
{
}

TFCDC_SegmentCreatorMCTruthModule::TFCDC_SegmentCreatorMCTruthModule()
  : Super( {"CDCWireHitVector", "CDCSegment2DVector"})
{
}
