#include "trg/cdc/modules/trgcdc/CDCTriggerETFModule.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/trg/dataobjects/TRGTiming.h>

#include <TH1.h>

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

  addParam("TSHitCollectionName",
           m_TSHitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string(""));
  addParam("EventTimeName", m_EventTimeName,
           "Name of the output StoreObtPtr.",
           string("CDCTriggerEventTime"));
  addParam("trueEventTime",
           m_trueEventTime,
           "If true, always output 0 (assuming this is the true event time for MC).",
           false);
  addParam("threshold",
           m_threshold,
           "Event time is given by first timing bin with more than threshold hits.",
           unsigned(3));
}

void
CDCTriggerETFModule::initialize()
{
  // register DataStore elements
  StoreObjPtr<TRGTiming>::registerPersistent(m_EventTimeName);
  StoreArray<CDCTriggerSegmentHit>::required(m_TSHitCollectionName);
}

void
CDCTriggerETFModule::event()
{
  if (m_trueEventTime) {
    StoreObjPtr<TRGTiming> eventTime(m_EventTimeName);
    eventTime.construct(Const::CDC, 0);
    return;
  }

  // counter for hits per super layer and clock cycle
  int cnt[9][64] = {};
  // histogram for fastest timings
  TH1* h = new TH1D("h", "h", 1000, -500, 499);
  // loop over hits
  StoreArray<CDCTriggerSegmentHit> hits(m_TSHitCollectionName);
  // loop over clock cycles to get time ordered hits
  for (int iClk = 0; iClk < 64; ++iClk) {
    for (int iTS = 0; iTS < hits.getEntries(); ++iTS) {
      int foundT = hits[iTS]->foundTime();
      if (foundT / 16 + 31 != iClk)
        continue;
      int fastestT = hits[iTS]->fastestTime();
      int whdiff = foundT - fastestT;
      if (whdiff <= 256) {
        // loop over super layers to get hits ordered by layer
        for (int iSL = 0; iSL < 9; ++iSL) {
          if (hits[iTS]->getISuperLayer() != iSL)
            continue;
          cnt[iSL][iClk] += 1;
          if (cnt[iSL][iClk] <= 10) {
            h->Fill(fastestT);
            B2DEBUG(100, "fill fastestT " << fastestT);
          }
        }
      }
    }
  }

  // find first histogram entry above threshold
  bool foundT0 = false;
  int T0 = 500;
  for (int i = 450; i < 600; ++i) {
    if (h->GetBinContent(i) > m_threshold) {
      foundT0 = true;
      T0 = i - 500;
      break;
    }
  }
  delete h;

  // save event time
  if (foundT0) {
    StoreObjPtr<TRGTiming> eventTime(m_EventTimeName);
    eventTime.construct(Const::CDC, T0);
  }
}
