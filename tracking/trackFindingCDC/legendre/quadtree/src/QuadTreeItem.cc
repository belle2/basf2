#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/ConformalCDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template<>
bool QuadTreeItem<ConformalCDCWireHit>::isUsed() const
{
  return getPointer()->getUsedFlag() || getPointer()->getMaskedFlag();
}

template<>
void QuadTreeItem<ConformalCDCWireHit>::setUsedFlag(bool usedFlag)
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
