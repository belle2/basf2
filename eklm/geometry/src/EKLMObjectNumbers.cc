/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/geometry/EKLMObjectNumbers.h>
#include <eklm/geometry/GeometryData.h>

using namespace Belle2;

int EKLM::layerNumber(int endcap, int layer)
{
  return 14 * (endcap - 1) + layer;
}

int EKLM::detectorLayerNumber(int endcap, int layer)
{
  if (endcap == 1)
    return layer;
  return EKLM::GeometryData::Instance().getNDetectorLayers(1) + layer;
}

int EKLM::sectorNumber(int endcap, int layer, int sector)
{
  return 4 * (14 * (endcap - 1) + (layer - 1)) + sector;
}

int EKLM::planeNumber(int endcap, int layer, int sector, int plane)
{
  return 2 * (4 * detectorLayerNumber(endcap, layer) + (sector - 1)) + plane;
}

int EKLM::segmentNumber(int endcap, int layer, int sector, int plane,
                        int segment)
{
  return 5 * (2 * (4 * detectorLayerNumber(endcap, layer) +
                   (sector - 1)) + (plane - 1)) + segment;
}
int EKLM::stripNumber(int endcap, int layer, int sector, int plane, int strip)
{
  return 75 * (2 * (4 * detectorLayerNumber(endcap, layer) +
                    (sector - 1)) + (plane - 1)) + strip;
}

int EKLM::layerLocalNumber(int gnum)
{
  return (gnum - 1) % 14 + 1;
}

int EKLM::sectorLocalNumber(int gnum)
{
  return (gnum - 1) % 4 + 1;
}

int EKLM::planeLocalNumber(int gnum)
{
  return (gnum - 1) % 2 + 1;
}

int EKLM::stripLocalNumber(int gnum)
{
  return (gnum - 1) % 75 + 1;
}

