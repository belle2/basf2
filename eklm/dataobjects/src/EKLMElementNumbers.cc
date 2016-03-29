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

void EKLMElementNumbers::checkEndcap(int endcap) const
{
  if (endcap <= 0 || endcap > m_MaximalEndcapNumber)
    B2FATAL("Number of endcap must be 1 (backward) or 2 (forward).");
}

void EKLMElementNumbers::checkLayer(int layer) const
{
  if (layer <= 0 || layer > m_MaximalLayerNumber)
    B2FATAL("Number of layer must be from 1 to " << m_MaximalLayerNumber <<
            ".");
}

void EKLMElementNumbers::checkDetectorLayer(int endcap, int layer) const
{
  const char* endcapName[2] = {"backward", "forward"};
  if (layer < 0 || layer > m_MaximalDetectorLayerNumber[endcap - 1])
    B2FATAL("Number of layer must be less from 1 to the maximal number of "
            "detector layers in the " << endcapName[endcap - 1] << " endcap ("
            << m_MaximalDetectorLayerNumber[endcap - 1] << ").");
}

void EKLMElementNumbers::checkSector(int sector) const
{
  if (sector <= 0 || sector > m_MaximalSectorNumber)
    B2FATAL("Number of sector must be from 1 to " << m_MaximalSectorNumber <<
            ".");
}

void EKLMElementNumbers::checkPlane(int plane) const
{
  if (plane <= 0 || plane > m_MaximalPlaneNumber)
    B2FATAL("Number of plane must be from 1 to " << m_MaximalPlaneNumber <<
            ".");
}

void EKLMElementNumbers::checkSegment(int segment) const
{
  if (segment <= 0 || segment > m_MaximalSegmentNumber)
    B2FATAL("Number of segment must be from 1 to " << m_MaximalSegmentNumber <<
            ".");
}

void EKLMElementNumbers::checkStrip(int strip) const
{
  if (strip <= 0 || strip > m_MaximalStripNumber)
    B2FATAL("Number of strip must be from 1 to " << m_MaximalStripNumber <<
            ".");
}

int EKLMElementNumbers::detectorLayerNumber(int endcap, int layer) const
{
  checkEndcap(endcap);
  checkDetectorLayer(endcap, layer);
  if (endcap == 1)
    return layer;
  return m_MaximalDetectorLayerNumber[0] + layer;
}

int EKLMElementNumbers::sectorNumber(int endcap, int layer, int sector) const
{
  checkSector(sector);
  return m_MaximalSectorNumber * (detectorLayerNumber(endcap, layer) - 1) +
         sector;
}

int EKLMElementNumbers::planeNumber(int endcap, int layer, int sector,
                                    int plane) const
{
  checkPlane(plane);
  return m_MaximalPlaneNumber * (sectorNumber(endcap, layer, sector) - 1) +
         plane;
}

int EKLMElementNumbers::segmentNumber(int endcap, int layer, int sector,
                                      int plane, int segment) const
{
  checkSegment(segment);
  return m_MaximalSegmentNumber * (planeNumber(endcap, layer, sector, plane) -
                                   1) + segment;
}

int EKLMElementNumbers::stripNumber(int endcap, int layer, int sector,
                                    int plane, int strip) const
{
  checkStrip(strip);
  return m_MaximalStripNumber * (planeNumber(endcap, layer, sector, plane) - 1)
         + strip;
}

int EKLMElementNumbers::stripLocalNumber(int strip) const
{
  static int maxStrip = getMaximalStripNumber();
  if (strip <= 0 || strip > maxStrip)
    B2FATAL("Number of strip must be from 1 to getMaximalStripNumber().");
  return (strip - 1) % m_MaximalStripNumber + 1;
}

int EKLMElementNumbers::getMaximalStripNumber() const
{
  return stripNumber(m_MaximalEndcapNumber,
                     m_MaximalDetectorLayerNumber[m_MaximalEndcapNumber - 1],
                     m_MaximalSectorNumber, m_MaximalPlaneNumber,
                     m_MaximalStripNumber);
}

