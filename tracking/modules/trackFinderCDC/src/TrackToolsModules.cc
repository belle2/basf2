/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/TrackToolsModules.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_TrackRejecter);
REG_MODULE(TFCDC_TrackQualityAsserter);
REG_MODULE(TFCDC_TrackOrienter);
REG_MODULE(TFCDC_TrackFlightTimeAdjuster);
REG_MODULE(TFCDC_TrackExporter);

TFCDC_TrackRejecterModule::TFCDC_TrackRejecterModule()
  : Super( {"CDCTrackVector"})
{
}

TFCDC_TrackQualityAsserterModule::TFCDC_TrackQualityAsserterModule()
  : Super( {"CDCTrackVector"})
{
}

TFCDC_TrackOrienterModule::TFCDC_TrackOrienterModule()
  : Super( {"CDCTrackVector"})
{
}

TFCDC_TrackFlightTimeAdjusterModule::TFCDC_TrackFlightTimeAdjusterModule()
  : Super( {"CDCTrackVector"})
{
}

TFCDC_TrackExporterModule::TFCDC_TrackExporterModule()
  : Super( {"CDCTrackVector"})
{
}
