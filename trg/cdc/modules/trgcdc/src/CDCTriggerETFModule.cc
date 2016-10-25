#include "trg/cdc/modules/trgcdc/CDCTriggerETFModule.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/trg/dataobjects/TRGTiming.h>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTriggerETF);

CDCTriggerETFModule::CDCTriggerETFModule() : Module::Module()
{
  setDescription(
    "The Event Time Finder module of the CDC trigger.\n"
    "Uses fastest time of CDCTriggerSegmentHits to find the event time.\n"
  );
}

void
CDCTriggerETFModule::initialize()
{
  // register DataStore elements
  StoreObjPtr<TRGTiming>::registerPersistent("CDCTriggerEventTime");
  StoreArray<CDCTriggerSegmentHit>::required();
}

void
CDCTriggerETFModule::event()
{
  StoreArray<CDCTriggerSegmentHit> hits;

  // find the event time
  // ... TODO ...
}
