#include <trg/cdc/dataobjects/CDCTriggerMLPData.h>

using namespace Belle2;

void
CDCTriggerMLPData::addHit(unsigned iSL, int iTS)
{
  if (iSL < hitCounters.size()) {
    if (iTS < 0) iTS += hitCounters[iSL].size();
    if (0 <= iTS && (unsigned)iTS < hitCounters[iSL].size()) {
      ++hitCounters[iSL][iTS];
    }
  }
}

unsigned short
CDCTriggerMLPData::getHitCounter(unsigned iSL, int iTS) const
{
  if (iSL < hitCounters.size()) {
    if (iTS < 0) iTS += hitCounters[iSL].size();
    if (0 <= iTS && (unsigned)iTS < hitCounters[iSL].size()) {
      return hitCounters[iSL][iTS];
    }
  }
  return 0;
}
