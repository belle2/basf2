/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/cdcQualityEstimator/TrackQualityEstimatorModule.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_TrackQualityEstimator);

TFCDC_TrackQualityEstimatorModule::TFCDC_TrackQualityEstimatorModule()
  : Super( {"CDCTrackVector"})
{
}
