/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
