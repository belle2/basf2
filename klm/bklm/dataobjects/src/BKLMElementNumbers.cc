/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/dataobjects/BKLMElementNumbers.h>

/* KLM headers. */
#include <klm/bklm/dataobjects/BKLMStatus.h>

/* Belle 2 headers. */
#include <rawdata/dataobjects/RawCOPPERFormat.h>

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
  checkChannelNumber(section, sector, layer, plane, strip);
  return (section << BKLM_END_BIT)
         | ((sector - 1) << BKLM_SECTOR_BIT)
         | ((layer - 1) << BKLM_LAYER_BIT)
         | ((plane) << BKLM_PLANE_BIT)
         | ((strip - 1) << BKLM_STRIP_BIT);
}

void BKLMElementNumbers::channelNumberToElementNumbers(
  uint16_t channel, int* section, int* sector, int* layer, int* plane,
  int* strip)
{
  *section = ((channel & BKLM_END_MASK) >> BKLM_END_BIT);
  *sector = ((channel & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1;
  *layer = ((channel & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1;
  *plane = ((channel & BKLM_PLANE_MASK) >> BKLM_PLANE_BIT);
  *strip = ((channel & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1;
}

uint16_t BKLMElementNumbers::moduleNumber(int section, int sector, int layer, bool fatalError)
{
  checkSection(section);
  checkSector(sector, fatalError);
  checkLayer(layer, fatalError);
  return (section << BKLM_END_BIT)
         | ((sector - 1) << BKLM_SECTOR_BIT)
         | ((layer - 1) << BKLM_LAYER_BIT);
}

void BKLMElementNumbers::moduleNumberToElementNumbers(
  uint16_t module, int* section, int* sector, int* layer)
{
  *section = ((module & BKLM_END_MASK) >> BKLM_END_BIT);
  *sector = ((module & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1;
  *layer = ((module & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1;
}

uint16_t BKLMElementNumbers::sectorNumber(int section, int sector)
{
  checkSection(section);
  checkSector(sector);
  return (section ? BKLM_END_MASK : 0)
         | ((sector - 1) << BKLM_SECTOR_BIT);
}

int BKLMElementNumbers::layerGlobalNumber(int section, int sector, int layer)
{
  checkSection(section);
  checkSector(sector);
  checkLayer(layer);
  int layerGlobal = layer - 1;
  layerGlobal += (sector - 1) * m_MaximalLayerNumber;
  layerGlobal += section * m_MaximalSectorNumber * m_MaximalLayerNumber;
  return layerGlobal;
}

int BKLMElementNumbers::getNStrips(
  int section, int sector, int layer, int plane)
{
  checkSection(section);
  checkSector(sector);
  checkLayer(layer);
  checkPlane(plane);
  int strips = 0;
  if (section == BKLMElementNumbers::c_BackwardSection && sector == 3 &&
      plane == BKLMElementNumbers::c_ZPlane) {
    /* Chimney sector. */
    if (layer < 3)
      strips = 38;
    if (layer > 2)
      strips = 34;
  } else {
    /* Other sectors. */
    if (layer == 1 && plane == BKLMElementNumbers::c_PhiPlane)
      strips = 37;
    if (layer == 2 && plane == BKLMElementNumbers::c_PhiPlane)
      strips = 42;
    if (layer > 2 && layer < 7 && plane == BKLMElementNumbers::c_PhiPlane)
      strips = 36;
    if (layer > 6 && plane == BKLMElementNumbers::c_PhiPlane)
      strips = 48;
    if (layer == 1 && plane == BKLMElementNumbers::c_ZPlane)
      strips = 54;
    if (layer == 2 && plane == BKLMElementNumbers::c_ZPlane)
      strips = 54;
    if (layer > 2 && plane == BKLMElementNumbers::c_ZPlane)
      strips = 48;
  }
  return strips;
}

bool BKLMElementNumbers::checkSection(int section, bool fatalError)
{
  if (section < BKLMElementNumbers::c_BackwardSection || section > BKLMElementNumbers::c_ForwardSection) {
    if (fatalError)
      B2FATAL("Invalid BKLM section number: " << section << ".");
    return false;
  }
  return true;
}

bool BKLMElementNumbers::checkSector(int sector, bool fatalError)
{
  if (sector < 1 || sector > m_MaximalSectorNumber) {
    if (fatalError)
      B2FATAL("Invalid BKLM sector number: " << sector << ".");
    return false;
  }
  return true;
}

bool BKLMElementNumbers::checkLayer(int layer, bool fatalError)
{
  if (layer < 1 || layer > m_MaximalLayerNumber) {
    if (fatalError)
      B2FATAL("Invalid BKLM layer number: " << layer << ".");
    return false;
  }
  return true;
}

bool BKLMElementNumbers::checkPlane(int plane, bool fatalError)
{
  if (plane < 0 || plane > m_MaximalPlaneNumber) {
    if (fatalError)
      B2FATAL("Invalid BKLM plane number: " << plane << ".");
    return false;
  }
  return true;
}

bool BKLMElementNumbers::checkChannelNumber(
  int section, int sector, int layer, int plane, int strip, bool fatalError)
{
  if (strip < 1 || strip > getNStrips(section, sector, layer, plane)) {
    if (fatalError)
      B2FATAL("Invalid BKLM channel number: section = " << section << ", sector = " << sector << ", layer = " << layer << ", plane = " <<
              plane << ", strip = " << strip << ".");
    return false;
  }
  return true;
}

void BKLMElementNumbers::layerGlobalNumberToElementNumbers(int layerGlobal, int* section, int* sector, int* layer)
{
  *section = ((layerGlobal / m_MaximalLayerNumber) / m_MaximalSectorNumber) % (m_MaximalLayerNumber + 1);
  *sector = ((layerGlobal / m_MaximalLayerNumber) % m_MaximalSectorNumber) + 1;
  *layer = (layerGlobal % m_MaximalLayerNumber) + 1;
}

std::string BKLMElementNumbers::getHSLBName(int copper, int slot)
{
  char hslb = 'a' + slot - 1;
  return "700" + std::to_string(copper - BKLM_ID) + hslb;
}
