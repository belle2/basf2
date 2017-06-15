/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/ClusterPreparationModules.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_ClusterPreparer);
REG_MODULE(TFCDC_SuperClusterCreator);
REG_MODULE(TFCDC_ClusterRefiner);
REG_MODULE(TFCDC_ClusterBackgroundDetector);

TFCDC_ClusterPreparerModule::TFCDC_ClusterPreparerModule()
  : Super( {"CDCWireHitVector", "CDCWireHitClusterVector", "CDCWireHitSuperClusterVector"})
{
}

TFCDC_SuperClusterCreatorModule::TFCDC_SuperClusterCreatorModule()
  : Super( {"CDCWireHitVector", "CDCWireHitSuperClusterVector"})
{
}

TFCDC_ClusterRefinerModule::TFCDC_ClusterRefinerModule()
  : Super( {"CDCWireHitSuperClusterVector", "CDCWireHitClusterVector"})
{
}

TFCDC_ClusterBackgroundDetectorModule::TFCDC_ClusterBackgroundDetectorModule()
  : Super( {"CDCWireHitClusterVector"})
{
}
