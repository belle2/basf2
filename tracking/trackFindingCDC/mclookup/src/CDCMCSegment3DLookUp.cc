/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/mclookup/CDCMCSegment3DLookUp.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitCollectionLookUp.icc.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::CDCMCHitCollectionLookUp<CDCSegment3D>;

const CDCMCSegment3DLookUp& CDCMCSegment3DLookUp::getInstance()
{
  return CDCMCManager::getMCSegment3DLookUp();
}
