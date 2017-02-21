/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

using namespace Belle2;
using namespace TrackFindingCDC;

const ISuperLayer ISuperLayerUtil::c_N;

const ISuperLayer ISuperLayerUtil::c_InnerVolume;

const ISuperLayer ISuperLayerUtil::c_OuterVolume;

const ISuperLayer ISuperLayerUtil::c_Invalid;

bool ISuperLayerUtil::isAxial(ISuperLayer iSuperLayer)
{
  return isInCDC(iSuperLayer) and (iSuperLayer % 2) == 0;
}

EStereoKind ISuperLayerUtil::getStereoKind(ISuperLayer iSuperLayer)
{
  if (not isInCDC(iSuperLayer)) return EStereoKind::c_Invalid;
  if (isAxial(iSuperLayer)) {
    return EStereoKind::c_Axial;
  } else if ((iSuperLayer % 4) == 1) {
    return EStereoKind::c_StereoU;
  } else {
    return EStereoKind::c_StereoV;
  }
}

bool ISuperLayerUtil::isInvalid(ISuperLayer iSuperLayer)
{
  return not isLogical(iSuperLayer);
}

bool ISuperLayerUtil::isInCDC(ISuperLayer iSuperLayer)
{
  return 0 <= iSuperLayer and iSuperLayer < c_N;
}

bool ISuperLayerUtil::isLogical(ISuperLayer iSuperLayer)
{
  return c_InnerVolume <= iSuperLayer and iSuperLayer <= c_OuterVolume;
}

bool ISuperLayerUtil::isInnerVolume(ISuperLayer iSuperLayer)
{
  return c_InnerVolume == iSuperLayer;
}

bool ISuperLayerUtil::isOuterVolume(ISuperLayer iSuperLayer)
{
  return c_OuterVolume == iSuperLayer;
}

ISuperLayer ISuperLayerUtil::getNextInwards(ISuperLayer iSuperLayer)
{
  if (isInvalid(iSuperLayer) or isInnerVolume(iSuperLayer)) {
    return c_Invalid;
  } else {
    return iSuperLayer - 1;
  }
}

ISuperLayer ISuperLayerUtil::getNextOutwards(ISuperLayer iSuperLayer)
{
  if (isInvalid(iSuperLayer) or isOuterVolume(iSuperLayer)) {
    return c_Invalid;
  } else {
    return iSuperLayer + 1;
  }
}
