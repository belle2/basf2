#include <tracking/modules/trackFinderCDC/WireHitPreparerModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(WireHitPreparer);

// Deprecated alias
REG_MODULE(WireHitTopologyPreparer);

WireHitPreparerModule::WireHitPreparerModule()
  : Super{{{"CDCWireHitVector"}}}
{
  setPropertyFlags(c_ParallelProcessingCertified);
}

void WireHitTopologyPreparerModule::initialize()
{
  // Super call
  WireHitPreparerModule::initialize();
  B2WARNING("WireHitPreparerTopologyModule is a deprecated alias for WireHitPreparerModule. "
            "Please adjust your steering file: WireHitTopologyPreparer -> WireHitPreparer");
}
