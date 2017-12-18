#include "trg/cdc/modules/trgcdc/CDCTriggerETFModule.h"

#include <cdc/geometry/CDCGeometryPar.h>

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

  addParam("hitCollectionName",
           m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string(""));
  addParam("EventTimeName", m_EventTimeName,
           "Name of the output StoreObjPtr.",
           string(""));
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
  m_eventTime.registerInDataStore();
  m_hits.isRequired(m_hitCollectionName);
}

void
CDCTriggerETFModule::event()
{
  if (!m_eventTime.isValid()) m_eventTime.create();

  if (m_trueEventTime) {
    m_eventTime->addBinnedEventT0(0, Const::CDC);
    return;
  }

  // counter for hits per super layer and clock cycle
  int cnt[9][64] = {};
  // histogram for fastest timings
  TH1* h = new TH1D("h", "h", 1000, -500, 499);
  // loop over clock cycles to get time ordered hits
  for (int iClk = 0; iClk < 64; ++iClk) {
    for (int iTS = 0; iTS < m_hits.getEntries(); ++iTS) {
      int foundT = m_hits[iTS]->foundTime();
      if (foundT / 16 + 31 != iClk)
        continue;
      int fastestT = m_hits[iTS]->fastestTime();
      int whdiff = foundT - fastestT;
      if (whdiff <= 256) {
        // loop over super layers to get hits ordered by layer
        for (int iSL = 0; iSL < 9; ++iSL) {
          if (m_hits[iTS]->getISuperLayer() != iSL)
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
    // add the event time as int for the following trigger modules
    m_eventTime->addBinnedEventT0(T0, Const::CDC);
  }
}
