/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/StereoHitFinderModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_StereoHitFinder);

TFCDC_StereoHitFinderModule::TFCDC_StereoHitFinderModule()
  : Super( {"CDCWireHitVector", "CDCTrackVector"})
{
}
