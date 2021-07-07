/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template<>
bool QuadTreeItem<const CDCWireHit>::isUsed() const
{
  return (*getPointer())->hasTakenFlag() or (*getPointer())->hasMaskedFlag();
}

template<>
void QuadTreeItem<const CDCWireHit>::setUsedFlag(bool usedFlag)
{
  (*getPointer())->setTakenFlag(usedFlag);
}

template<>
bool QuadTreeItem<CDCSegment2D>::isUsed() const
{
  return getPointer()->getAutomatonCell().hasTakenFlag();
}

template<>
void QuadTreeItem<CDCSegment2D>::setUsedFlag(bool usedFlag)
{
  getPointer()->getAutomatonCell().setTakenFlag(usedFlag);
}
