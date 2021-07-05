/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/WireHitPreparationModules.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_WireHitPreparer);
REG_MODULE(TFCDC_WireHitCreator);
REG_MODULE(TFCDC_HitReclaimer);
REG_MODULE(TFCDC_WireHitBackgroundDetector);

TFCDC_WireHitPreparerModule::TFCDC_WireHitPreparerModule()
  : Super( {"CDCWireHitVector"})
{
}

TFCDC_WireHitCreatorModule::TFCDC_WireHitCreatorModule()
  : Super( {"CDCWireHitVector"})
{
}

TFCDC_HitReclaimerModule::TFCDC_HitReclaimerModule()
  : Super( {"CDCWireHitVector"})
{
}

TFCDC_WireHitBackgroundDetectorModule::TFCDC_WireHitBackgroundDetectorModule()
  : Super( {"CDCWireHitVector"})
{
}
