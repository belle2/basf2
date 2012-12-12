/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#include <eklm/geometry/EKLMObjectNumbers.h>

using namespace Belle2;

int EKLM::layerNumber(int endcap, int layer)
{
  return 14 * (endcap - 1) + layer;
}

int EKLM::sectorNumber(int endcap, int layer, int sector)
{
  return 4 * (14 * (endcap - 1) + (layer - 1)) + sector;
}

int EKLM::planeNumber(int endcap, int layer, int sector, int plane)
{
  return 2 * (4 * (14 * (endcap - 1) + (layer - 1)) + (sector - 1)) + plane;
}

int EKLM::stripNumber(int endcap, int layer, int sector, int plane, int strip)
{
  return 75 * (2 * (4 * (14 * (endcap - 1) + (layer - 1)) +
                    (sector - 1)) + (plane - 1)) + strip;
}

int EKLM::boardNumber(int endcap, int layer, int sector, int plane, int board)
{
  return 5 * (2 * (4 * (14 * (endcap - 1) + (layer - 1)) +
                   (sector - 1)) + (plane - 1)) + board;
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

