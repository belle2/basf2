/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/WireHitPreparationModules.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_WireHitPreparer);
REG_MODULE(TFCDC_WireHitCreator);

TFCDC_WireHitPreparerModule::TFCDC_WireHitPreparerModule()
  : Super( {"CDCWireHitVector"})
{
}

TFCDC_WireHitCreatorModule::TFCDC_WireHitCreatorModule()
  : Super( {"CDCWireHitVector"})
{
}
