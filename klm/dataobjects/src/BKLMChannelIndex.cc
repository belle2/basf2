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
#include <klm/dataobjects/BKLMChannelIndex.h>

using namespace Belle2;

BKLMChannelIndex::BKLMChannelIndex() :
  m_Forward(0),
  m_Sector(1),
  m_Layer(1),
  m_Plane(0),
  m_Strip(1)
{
  m_NStripsPlane = BKLMElementNumbers::getNStrips(
                     m_Forward, m_Sector, m_Layer, m_Plane);
  m_ElementNumbers = &(KLMElementNumbers::Instance());
}

BKLMChannelIndex::BKLMChannelIndex(
  int forward, int sector, int layer, int plane, int strip) :
  m_Forward(forward),
  m_Sector(sector),
  m_Layer(layer),
  m_Plane(plane),
  m_Strip(strip)
{
  m_NStripsPlane = BKLMElementNumbers::getNStrips(
                     m_Forward, m_Sector, m_Layer, m_Plane);
  m_ElementNumbers = &(KLMElementNumbers::Instance());
}

BKLMChannelIndex::~BKLMChannelIndex()
{
}

uint16_t BKLMChannelIndex::getKLMChannelNumber()
{
  return m_ElementNumbers->channelNumberBKLM(
           m_Forward, m_Sector, m_Layer, m_Plane, m_Strip);
}

BKLMChannelIndex& BKLMChannelIndex::begin()
{
  static BKLMChannelIndex index(0, 1, 1, 0, 1);
  return index;
}

BKLMChannelIndex& BKLMChannelIndex::end()
{
  static BKLMChannelIndex index(
    BKLMElementNumbers::getMaximalForwardNumber(),
    BKLMElementNumbers::getMaximalSectorNumber(),
    BKLMElementNumbers::getMaximalLayerNumber(),
    BKLMElementNumbers::getMaximalPlaneNumber(),
    BKLMElementNumbers::getNStrips(
      BKLMElementNumbers::getMaximalForwardNumber(),
      BKLMElementNumbers::getMaximalSectorNumber(),
      BKLMElementNumbers::getMaximalLayerNumber(),
      BKLMElementNumbers::getMaximalPlaneNumber()));
  return index;
}

BKLMChannelIndex& BKLMChannelIndex::operator++()
{
  m_Strip++;
  if (m_Strip > m_NStripsPlane) {
    m_Strip = 1;
    m_Plane++;
    if (m_Plane > BKLMElementNumbers::getMaximalPlaneNumber()) {
      m_Plane = 0;
      m_Layer++;
      if (m_Layer > BKLMElementNumbers::getMaximalLayerNumber()) {
        m_Layer = 1;
        m_Sector++;
        if (m_Sector > BKLMElementNumbers::getMaximalSectorNumber()) {
          m_Sector = 1;
          m_Forward++;
          if (m_Forward > BKLMElementNumbers::getMaximalForwardNumber())
            m_Forward = 0;
        }
      }
    }
    m_NStripsPlane = BKLMElementNumbers::getNStrips(
                       m_Forward, m_Sector, m_Layer, m_Plane);
  }
  return *this;
}

bool BKLMChannelIndex::operator==(BKLMChannelIndex& index)
{
  if (m_Forward != index.getForward())
    return false;
  if (m_Sector != index.getSector())
    return false;
  if (m_Layer != index.getLayer())
    return false;
  if (m_Plane != index.getPlane())
    return false;
  if (m_Strip != index.getStrip())
    return false;
  return true;
}

bool BKLMChannelIndex::operator!=(BKLMChannelIndex& index)
{
  if (m_Forward != index.getForward())
    return true;
  if (m_Sector != index.getSector())
    return true;
  if (m_Layer != index.getLayer())
    return true;
  if (m_Plane != index.getPlane())
    return true;
  if (m_Strip != index.getStrip())
    return true;
  return false;
}

BKLMChannelIndex& BKLMChannelIndex::operator*()
{
  return *this;
}
