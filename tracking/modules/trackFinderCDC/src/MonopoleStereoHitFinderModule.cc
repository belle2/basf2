/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/MonopoleStereoHitFinderModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_MonopoleStereoHitFinder);

TFCDC_MonopoleStereoHitFinderModule::TFCDC_MonopoleStereoHitFinderModule()
  : Super( {"CDCWireHitVector", "CDCTrackVector"})
{
}
