#include <trg/cdc/dataobjects/CDCTriggerMLPData.h>

using namespace Belle2;

void
CDCTriggerMLPData::addHit(unsigned iSL, int iTS)
{
  if (iSL < hitCounters.size()) {
    int N = hitCounters[iSL].size();
    // shift negative indices
    if (iTS < 0) iTS += N;
    // check the range
    //   0, N:        hit is on the track
    //   [0, N/4]:    hit is in quadrant left of the track
    //   [N/4, 3N/4]: hit is in hemisphere opposite of the track (skipped)
    //   [3N/4, N]:   hit is in quadrant right of the track
    if ((0 <= iTS && iTS <= N / 4) || (3 * N / 4 <= iTS && iTS < N)) {
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
