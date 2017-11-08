/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/cdcHitBasedT0Extraction/cdcHitBasedT0ExtractionModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(CDCHitBasedT0Extraction);

CDCHitBasedT0ExtractionModule::CDCHitBasedT0ExtractionModule()
  : Super( {"CDCWireHitVector"})
{
}
