/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
