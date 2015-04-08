#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template<>
bool QuadTreeItem<TrackHit>::isUsed() const
{
  return getPointer()->getHitUsage() == TrackHit::used_in_track;
}

template<>
void QuadTreeItem<TrackHit>::setUsedFlag(bool usedFlag)
{
  if (usedFlag) {
    getPointer()->setHitUsage(TrackHit::used_in_track);
  } else {
    getPointer()->setHitUsage(TrackHit::not_used);
  }
}
