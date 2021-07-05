/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
