/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/TrackMergerModules.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_TrackLinker);
REG_MODULE(TFCDC_TrackCombiner);

TFCDC_TrackLinkerModule::TFCDC_TrackLinkerModule()
  : Super( {"CDCTrackVector"})
{
}

TFCDC_TrackCombinerModule::TFCDC_TrackCombinerModule()
  : Super( {"" /*to be set externally*/, "" /*to be set externally*/, ""  /*to be set externally*/})
{
}
