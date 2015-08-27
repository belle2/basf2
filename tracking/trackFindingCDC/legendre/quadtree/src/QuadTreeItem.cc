#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>
#include "../../include/QuadTreeHitWrapper.h"

using namespace Belle2;
using namespace TrackFindingCDC;

template<>
bool QuadTreeItem<TrackHit>::isUsed() const
{
  return getPointer()->getHitUsage() == TrackHit::c_usedInTrack;
}

template<>
void QuadTreeItem<TrackHit>::setUsedFlag(bool usedFlag)
{
  if (usedFlag) {
    getPointer()->setHitUsage(TrackHit::c_usedInTrack);
  } else {
    getPointer()->setHitUsage(TrackHit::c_notUsed);
  }
}

template<>
bool QuadTreeItem<QuadTreeHitWrapper>::isUsed() const
{
  return getPointer()->getUsedFlag() || getPointer()->getMaskedFlag();
}

template<>
void QuadTreeItem<QuadTreeHitWrapper>::setUsedFlag(bool usedFlag)
{
  getPointer()->setUsedFlag(usedFlag);
}

template<>
bool QuadTreeItem<CDCRecoSegment2D>::isUsed() const
{
  return getPointer()->getAutomatonCell().hasTakenFlag();
}

template<>
void QuadTreeItem<CDCRecoSegment2D>::setUsedFlag(bool usedFlag)
{
  getPointer()->getAutomatonCell().setTakenFlag(usedFlag);
}
