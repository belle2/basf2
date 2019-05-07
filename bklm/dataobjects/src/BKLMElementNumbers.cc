/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMElementNumbers.h>
#include <bklm/dataobjects/BKLMStatus.h>

using namespace Belle2;

uint16_t BKLMElementNumbers::channelNumber(
  int forward, int sector, int layer, int plane, int strip)
{
  return (forward ? BKLM_END_MASK : 0)
         | ((sector - 1) << BKLM_SECTOR_BIT)
         | ((layer - 1) << BKLM_LAYER_BIT)
         | ((plane) << BKLM_PLANE_BIT)
         | ((strip - 1) << BKLM_STRIP_BIT);
}

bool BKLMElementNumbers::checkChannelNumber(
  int forward, int sector, int layer, int plane, int strip)
{
  bool flag = false;
  if (forward >= 0 and forward <= 1) {
    if (sector >= 1 and sector <= 8) {
      if (layer >= 1 and layer <= 15) {
        if (plane == 0) { // phi plane
          if (layer == 1 and strip >= 1 and strip <= 37)
            flag = true;
          if (layer == 2 and strip >= 1 and strip <= 42)
            flag = true;
          if (layer >= 3 and layer <= 6 and strip >= 1 and strip <= 36)
            flag = true;
          if (layer >= 7 and strip >= 1 and strip <= 48)
            flag = true;
        } // phi
        if (plane == 1) { // z plane
          if (forward == 0 and sector == 3) { // BB2 is a special case
            if (layer >= 1 and layer <= 2 and strip >= 1 and strip <= 38)
              flag = true;
            if (layer >= 3 and strip >= 1 and strip <= 34)
              flag = true;
          } // BB2
          else {
            if (layer >= 1 and layer <= 2 and strip >= 1 and strip <= 54)
              flag = true;
            if (layer >= 3 and strip >= 1 and strip <= 48)
              flag = true;
          }
        } // z
      } // layer
    } // sector
  } // forward
  return flag;
}
