/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>
#include <rawdata/dataobjects/RawCOPPERFormat.h>

using namespace Belle2;

const EKLMElementNumbers& EKLMElementNumbers::Instance()
{
  static EKLMElementNumbers ens;
  return ens;
}

EKLMElementNumbers::EKLMElementNumbers()
{
}

EKLMElementNumbers::~EKLMElementNumbers()
{
}

bool EKLMElementNumbers::checkSection(int section, bool fatalError) const
{
  if (section <= 0 || section > m_MaximalSectionNumber) {
    if (fatalError) {
      B2FATAL("Number of section must be 1 (backward) or 2 (forward).");
    }
    return false;
  }
  return true;
}

bool EKLMElementNumbers::checkLayer(int layer, bool fatalError) const
{
  if (layer <= 0 || layer > m_MaximalLayerNumber) {
    if (fatalError) {
      B2FATAL("Number of layer must be from 1 to " << m_MaximalLayerNumber <<
              ".");
    }
    return false;
  }
  return true;
}

bool EKLMElementNumbers::checkDetectorLayer(int section, int layer,
                                            bool fatalError) const
{
  /* cppcheck-suppress variableScope */
  const char* sectionName[2] = {"backward", "forward"};
  if (layer < 0 || layer > m_MaximalDetectorLayerNumber[section - 1]) {
    if (fatalError) {
      B2FATAL("Number of layer must be from 1 to the maximal number of "
              "detector layers in the " << sectionName[section - 1] << " section ("
              << m_MaximalDetectorLayerNumber[section - 1] << ").");
    }
    return false;
  }
  return true;
}

bool EKLMElementNumbers::checkSector(int sector, bool fatalError) const
{
  if (sector <= 0 || sector > m_MaximalSectorNumber) {
    if (fatalError) {
      B2FATAL("Number of sector must be from 1 to " << m_MaximalSectorNumber <<
              ".");
    }
    return false;
  }
  return true;
}

bool EKLMElementNumbers::checkPlane(int plane, bool fatalError) const
{
  if (plane <= 0 || plane > m_MaximalPlaneNumber) {
    if (fatalError) {
      B2FATAL("Number of plane must be from 1 to " << m_MaximalPlaneNumber <<
              ".");
    }
    return false;
  }
  return true;
}

bool EKLMElementNumbers::checkSegment(int segment, bool fatalError) const
{
  if (segment <= 0 || segment > m_MaximalSegmentNumber) {
    if (fatalError) {
      B2FATAL("Number of segment must be from 1 to " <<
              m_MaximalSegmentNumber << ".");
    }
    return false;
  }
  return true;
}

bool EKLMElementNumbers::checkStrip(int strip, bool fatalError) const
{
  if (strip <= 0 || strip > m_MaximalStripNumber) {
    if (fatalError) {
      B2FATAL("Number of strip must be from 1 to " << m_MaximalStripNumber <<
              ".");
    }
    return false;
  }
  return true;
}

int EKLMElementNumbers::detectorLayerNumber(int section, int layer) const
{
  checkSection(section);
  checkDetectorLayer(section, layer);
  if (section == 1)
    return layer;
  return m_MaximalDetectorLayerNumber[0] + layer;
}

void EKLMElementNumbers::layerNumberToElementNumbers(
  int layerGlobal, int* section, int* layer) const
{
  static int maxLayer = getMaximalLayerGlobalNumber();
  if (layerGlobal <= 0 || layerGlobal > maxLayer)
    B2FATAL("Number of segment must be from 1 to " << maxLayer << ".");
  if (layerGlobal <= m_MaximalDetectorLayerNumber[0]) {
    *section = 1;
    *layer = layerGlobal;
  } else {
    *section = 2;
    *layer = layerGlobal - m_MaximalDetectorLayerNumber[0];
  }
}

int EKLMElementNumbers::sectorNumber(int section, int layer, int sector) const
{
  checkSector(sector);
  return m_MaximalSectorNumber * (detectorLayerNumber(section, layer) - 1) +
         sector;
}

int EKLMElementNumbers::sectorNumberKLMOrder(int section, int sector) const
{
  checkSection(section);
  checkSector(sector);
  return m_MaximalSectorNumber * (section - 1) + sector;
}

void EKLMElementNumbers::sectorNumberToElementNumbers(
  int sectorGlobal, int* section, int* layer, int* sector) const
{
  static int maxSector = getMaximalSectorGlobalNumber();
  int layerGlobal;
  if (sectorGlobal <= 0 || sectorGlobal > maxSector)
    B2FATAL("Number of segment must be from 1 to " << maxSector << ".");
  *sector = (sectorGlobal - 1) % m_MaximalSectorNumber + 1;
  layerGlobal = (sectorGlobal - 1) / m_MaximalSectorNumber + 1;
  layerNumberToElementNumbers(layerGlobal, section, layer);
}

int EKLMElementNumbers::planeNumber(int section, int layer, int sector,
                                    int plane) const
{
  checkPlane(plane);
  return m_MaximalPlaneNumber * (sectorNumber(section, layer, sector) - 1) +
         plane;
}

void EKLMElementNumbers::planeNumberToElementNumbers(
  int planeGlobal, int* section, int* layer, int* sector, int* plane) const
{
  static int maxPlane = getMaximalPlaneGlobalNumber();
  int sectorGlobal;
  if (planeGlobal <= 0 || planeGlobal > maxPlane)
    B2FATAL("Number of segment must be from 1 to " << maxPlane << ".");
  *plane = (planeGlobal - 1) % m_MaximalPlaneNumber + 1;
  sectorGlobal = (planeGlobal - 1) / m_MaximalPlaneNumber + 1;
  sectorNumberToElementNumbers(sectorGlobal, section, layer, sector);
}

int EKLMElementNumbers::segmentNumber(int section, int layer, int sector,
                                      int plane, int segment) const
{
  checkSegment(segment);
  return m_MaximalSegmentNumber * (planeNumber(section, layer, sector, plane) -
                                   1) + segment;
}

void EKLMElementNumbers::segmentNumberToElementNumbers(
  int segmentGlobal, int* section, int* layer, int* sector, int* plane,
  int* segment) const
{
  static int maxSegment = getMaximalSegmentGlobalNumber();
  int planeGlobal;
  if (segmentGlobal <= 0 || segmentGlobal > maxSegment)
    B2FATAL("Number of segment must be from 1 to " << maxSegment << ".");
  *segment = (segmentGlobal - 1) % m_MaximalSegmentNumber + 1;
  planeGlobal = (segmentGlobal - 1) / m_MaximalSegmentNumber + 1;
  planeNumberToElementNumbers(planeGlobal, section, layer, sector, plane);
}

int EKLMElementNumbers::stripNumber(int section, int layer, int sector,
                                    int plane, int strip) const
{
  checkStrip(strip);
  return m_MaximalStripNumber * (planeNumber(section, layer, sector, plane) - 1)
         + strip;
}

void EKLMElementNumbers::stripNumberToElementNumbers(
  int stripGlobal, int* section, int* layer, int* sector, int* plane,
  int* strip) const
{
  static int maxStrip = getMaximalStripGlobalNumber();
  int planeGlobal;
  if (stripGlobal <= 0 || stripGlobal > maxStrip)
    B2FATAL("Number of strip must be from 1 to " << maxStrip << ".");
  *strip = (stripGlobal - 1) % m_MaximalStripNumber + 1;
  planeGlobal = (stripGlobal - 1) / m_MaximalStripNumber + 1;
  planeNumberToElementNumbers(planeGlobal, section, layer, sector, plane);
}

int EKLMElementNumbers::getSectionByGlobalStrip(int stripGlobal)
{
  int section, layer, sector, plane, strip;
  stripNumberToElementNumbers(stripGlobal,
                              &section, &layer, &sector, &plane, &strip);
  return section;
}

int EKLMElementNumbers::getLayerByGlobalStrip(int stripGlobal)
{
  int section, layer, sector, plane, strip;
  stripNumberToElementNumbers(stripGlobal,
                              &section, &layer, &sector, &plane, &strip);
  return layer;
}

int EKLMElementNumbers::getSectorByGlobalStrip(int stripGlobal)
{
  int section, layer, sector, plane, strip;
  stripNumberToElementNumbers(stripGlobal,
                              &section, &layer, &sector, &plane, &strip);
  return sector;
}

int EKLMElementNumbers::getPlaneByGlobalStrip(int stripGlobal)
{
  int section, layer, sector, plane, strip;
  stripNumberToElementNumbers(stripGlobal,
                              &section, &layer, &sector, &plane, &strip);
  return plane;
}

int EKLMElementNumbers::getStripByGlobalStrip(int stripGlobal)
{
  static int maxStrip = getMaximalStripGlobalNumber();
  if (stripGlobal <= 0 || stripGlobal > maxStrip)
    B2FATAL("Number of strip must be from 1 to " << maxStrip << ".");
  return (stripGlobal - 1) % m_MaximalStripNumber + 1;
}

std::string EKLMElementNumbers::getHSLBName(int copper, int slot)
{
  char hslb = 'a' + slot - 1;
  return "800" + std::to_string(copper - EKLM_ID) + hslb;
}

int EKLMElementNumbers::getMaximalDetectorLayerNumber(int section) const
{
  checkSection(section);
  return m_MaximalDetectorLayerNumber[section - 1];
}
