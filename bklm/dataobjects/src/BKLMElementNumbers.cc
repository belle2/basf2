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

BKLMElementNumbers::BKLMElementNumbers()
{
}

BKLMElementNumbers::~BKLMElementNumbers()
{
}

uint16_t BKLMElementNumbers::channelNumber(
  int section, int sector, int layer, int plane, int strip)
{
  return (section ? BKLM_END_MASK : 0)
         | ((sector - 1) << BKLM_SECTOR_BIT)
         | ((layer - 1) << BKLM_LAYER_BIT)
         | ((plane) << BKLM_PLANE_BIT)
         | ((strip - 1) << BKLM_STRIP_BIT);
}

void BKLMElementNumbers::channelNumberToElementNumbers(
  uint16_t channel, int* forward, int* sector, int* layer, int* plane,
  int* strip)
{
  *forward = ((channel & BKLM_END_MASK) >> BKLM_END_BIT);
  *sector = ((channel & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1;
  *layer = ((channel & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1;
  *plane = ((channel & BKLM_PLANE_MASK) >> BKLM_PLANE_BIT);
  *strip = ((channel & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1;
}

uint16_t BKLMElementNumbers::moduleNumber(int section, int sector, int layer)
{
  return (section ? BKLM_END_MASK : 0)
         | ((sector - 1) << BKLM_SECTOR_BIT)
         | ((layer - 1) << BKLM_LAYER_BIT);
}

void BKLMElementNumbers::moduleNumberToElementNumbers(
  uint16_t module, int* forward, int* sector, int* layer)
{
  *forward = ((module & BKLM_END_MASK) >> BKLM_END_BIT);
  *sector = ((module & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1;
  *layer = ((module & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1;
}

uint16_t BKLMElementNumbers::sectorNumber(int section, int sector)
{
  return (section ? BKLM_END_MASK : 0)
         | ((sector - 1) << BKLM_SECTOR_BIT);
}

int BKLMElementNumbers::layerGlobalNumber(int section, int sector, int layer)
{
  int layerGlobal = layer - 1;
  layerGlobal += (sector - 1) * m_MaximalLayerNumber;
  layerGlobal += section * m_MaximalSectorNumber * m_MaximalLayerNumber;
  return layerGlobal;
}

int BKLMElementNumbers::getNStrips(
  int section, int sector, int layer, int plane)
{
  int strips = 0;
  if (section == BKLMElementNumbers::c_BackwardSection && sector == 3 && plane == 0) {
    /* Chimney sector. */
    if (layer < 3)
      strips = 38;
    if (layer > 2)
      strips = 34;
  } else {
    /* Other sectors. */
    if (layer == 1 && plane == 1)
      strips = 37;
    if (layer == 2 && plane == 1)
      strips = 42;
    if (layer > 2 && layer < 7 && plane == 1)
      strips = 36;
    if (layer > 6 && plane == 1)
      strips = 48;
    if (layer == 1 && plane == 0)
      strips = 54;
    if (layer == 2 && plane == 0)
      strips = 54;
    if (layer > 2 && plane == 0)
      strips = 48;
  }
  return strips;
}

bool BKLMElementNumbers::checkChannelNumber(
  int section, int sector, int layer, int plane, int strip)
{
  return (strip >= 1) && (strip <= BKLMElementNumbers::getNStrips(
                            section, sector, layer, plane));
}

void BKLMElementNumbers::layerGlobalNumberToElementNumbers(int layerGlobal, int* section, int* sector, int* layer)
{
  *section = ((layerGlobal / m_MaximalLayerNumber) / m_MaximalSectorNumber) % (m_MaximalLayerNumber + 1);
  *sector = ((layerGlobal / m_MaximalLayerNumber) % m_MaximalSectorNumber) + 1;
  *layer = (layerGlobal % m_MaximalLayerNumber) + 1;
}
