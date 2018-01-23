/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMElementNumbers.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMElementNumbers::EKLMElementNumbers() : m_MaximalEndcapNumber(2),
  m_MaximalLayerNumber(14), m_MaximalDetectorLayerNumber{12, 14},
  m_MaximalSectorNumber(4), m_MaximalPlaneNumber(2),
  m_MaximalSegmentNumber(5), m_MaximalStripNumber(75)
{
}

EKLMElementNumbers::~EKLMElementNumbers()
{
}

bool EKLMElementNumbers::checkEndcap(int endcap, bool fatalError) const
{
  if (endcap <= 0 || endcap > m_MaximalEndcapNumber) {
    if (fatalError) {
      B2FATAL("Number of endcap must be 1 (backward) or 2 (forward).");
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

bool EKLMElementNumbers::checkDetectorLayer(int endcap, int layer,
                                            bool fatalError) const
{
  const char* endcapName[2] = {"backward", "forward"};
  if (layer < 0 || layer > m_MaximalDetectorLayerNumber[endcap - 1]) {
    if (fatalError) {
      B2FATAL("Number of layer must be from 1 to the maximal number of "
              "detector layers in the " << endcapName[endcap - 1] << " endcap ("
              << m_MaximalDetectorLayerNumber[endcap - 1] << ").");
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

int EKLMElementNumbers::detectorLayerNumber(int endcap, int layer) const
{
  checkEndcap(endcap);
  checkDetectorLayer(endcap, layer);
  if (endcap == 1)
    return layer;
  return m_MaximalDetectorLayerNumber[0] + layer;
}

void EKLMElementNumbers::layerNumberToElementNumbers(
  int layerGlobal, int* endcap, int* layer) const
{
  static int maxLayer = getMaximalLayerGlobalNumber();
  if (layerGlobal <= 0 || layerGlobal > maxLayer)
    B2FATAL("Number of segment must be from 1 to " << maxLayer << ".");
  if (layerGlobal <= m_MaximalDetectorLayerNumber[0]) {
    *endcap = 1;
    *layer = layerGlobal;
  } else {
    *endcap = 2;
    *layer = layerGlobal - m_MaximalDetectorLayerNumber[0];
  }
}

int EKLMElementNumbers::sectorNumber(int endcap, int layer, int sector) const
{
  checkSector(sector);
  return m_MaximalSectorNumber * (detectorLayerNumber(endcap, layer) - 1) +
         sector;
}

void EKLMElementNumbers::sectorNumberToElementNumbers(
  int sectorGlobal, int* endcap, int* layer, int* sector) const
{
  static int maxSector = getMaximalSectorGlobalNumber();
  int layerGlobal;
  if (sectorGlobal <= 0 || sectorGlobal > maxSector)
    B2FATAL("Number of segment must be from 1 to " << maxSector << ".");
  *sector = (sectorGlobal - 1) % m_MaximalSectorNumber + 1;
  layerGlobal = (sectorGlobal - 1) / m_MaximalSectorNumber + 1;
  layerNumberToElementNumbers(layerGlobal, endcap, layer);
}

int EKLMElementNumbers::planeNumber(int endcap, int layer, int sector,
                                    int plane) const
{
  checkPlane(plane);
  return m_MaximalPlaneNumber * (sectorNumber(endcap, layer, sector) - 1) +
         plane;
}

void EKLMElementNumbers::planeNumberToElementNumbers(
  int planeGlobal, int* endcap, int* layer, int* sector, int* plane) const
{
  static int maxPlane = getMaximalPlaneGlobalNumber();
  int sectorGlobal;
  if (planeGlobal <= 0 || planeGlobal > maxPlane)
    B2FATAL("Number of segment must be from 1 to " << maxPlane << ".");
  *plane = (planeGlobal - 1) % m_MaximalPlaneNumber + 1;
  sectorGlobal = (planeGlobal - 1) / m_MaximalPlaneNumber + 1;
  sectorNumberToElementNumbers(sectorGlobal, endcap, layer, sector);
}

int EKLMElementNumbers::segmentNumber(int endcap, int layer, int sector,
                                      int plane, int segment) const
{
  checkSegment(segment);
  return m_MaximalSegmentNumber * (planeNumber(endcap, layer, sector, plane) -
                                   1) + segment;
}

void EKLMElementNumbers::segmentNumberToElementNumbers(
  int segmentGlobal, int* endcap, int* layer, int* sector, int* plane,
  int* segment) const
{
  static int maxSegment = getMaximalSegmentGlobalNumber();
  int planeGlobal;
  if (segmentGlobal <= 0 || segmentGlobal > maxSegment)
    B2FATAL("Number of segment must be from 1 to " << maxSegment << ".");
  *segment = (segmentGlobal - 1) % m_MaximalSegmentNumber + 1;
  planeGlobal = (segmentGlobal - 1) / m_MaximalSegmentNumber + 1;
  planeNumberToElementNumbers(planeGlobal, endcap, layer, sector, plane);
}

int EKLMElementNumbers::stripNumber(int endcap, int layer, int sector,
                                    int plane, int strip) const
{
  checkStrip(strip);
  return m_MaximalStripNumber * (planeNumber(endcap, layer, sector, plane) - 1)
         + strip;
}

void EKLMElementNumbers::stripNumberToElementNumbers(
  int stripGlobal, int* endcap, int* layer, int* sector, int* plane,
  int* strip) const
{
  static int maxStrip = getMaximalStripGlobalNumber();
  int planeGlobal;
  if (stripGlobal <= 0 || stripGlobal > maxStrip)
    B2FATAL("Number of strip must be from 1 to " << maxStrip << ".");
  *strip = (stripGlobal - 1) % m_MaximalStripNumber + 1;
  planeGlobal = (stripGlobal - 1) / m_MaximalStripNumber + 1;
  planeNumberToElementNumbers(planeGlobal, endcap, layer, sector, plane);
}

int EKLMElementNumbers::stripLocalNumber(int strip) const
{
  static int maxStrip = getMaximalStripGlobalNumber();
  if (strip <= 0 || strip > maxStrip)
    B2FATAL("Number of strip must be from 1 to " << maxStrip << ".");
  return (strip - 1) % m_MaximalStripNumber + 1;
}

int EKLMElementNumbers::getMaximalEndcapNumber() const
{
  return m_MaximalEndcapNumber;
}

int EKLMElementNumbers::getMaximalLayerNumber() const
{
  return m_MaximalLayerNumber;
}

int EKLMElementNumbers::getMaximalDetectorLayerNumber(int endcap) const
{
  checkEndcap(endcap);
  return m_MaximalDetectorLayerNumber[endcap - 1];
}

int EKLMElementNumbers::getMaximalSectorNumber() const
{
  return m_MaximalSectorNumber;
}

int EKLMElementNumbers::getMaximalPlaneNumber() const
{
  return m_MaximalPlaneNumber;
}

int EKLMElementNumbers::getMaximalSegmentNumber() const
{
  return m_MaximalSegmentNumber;
}

int EKLMElementNumbers::getMaximalStripNumber() const
{
  return m_MaximalStripNumber;
}

int EKLMElementNumbers::getMaximalLayerGlobalNumber() const
{
  return detectorLayerNumber(
           m_MaximalEndcapNumber,
           m_MaximalDetectorLayerNumber[m_MaximalEndcapNumber - 1]);
}

int EKLMElementNumbers::getMaximalSectorGlobalNumber() const
{
  return sectorNumber(m_MaximalEndcapNumber,
                      m_MaximalDetectorLayerNumber[m_MaximalEndcapNumber - 1],
                      m_MaximalSectorNumber);
}

int EKLMElementNumbers::getMaximalPlaneGlobalNumber() const
{
  return planeNumber(m_MaximalEndcapNumber,
                     m_MaximalDetectorLayerNumber[m_MaximalEndcapNumber - 1],
                     m_MaximalSectorNumber, m_MaximalPlaneNumber);
}

int EKLMElementNumbers::getMaximalSegmentGlobalNumber() const
{
  return segmentNumber(m_MaximalEndcapNumber,
                       m_MaximalDetectorLayerNumber[m_MaximalEndcapNumber - 1],
                       m_MaximalSectorNumber, m_MaximalPlaneNumber,
                       m_MaximalSegmentNumber);
}

int EKLMElementNumbers::getMaximalStripGlobalNumber() const
{
  return stripNumber(m_MaximalEndcapNumber,
                     m_MaximalDetectorLayerNumber[m_MaximalEndcapNumber - 1],
                     m_MaximalSectorNumber, m_MaximalPlaneNumber,
                     m_MaximalStripNumber);
}

