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

BKLMChannelIndex::BKLMChannelIndex(enum IndexLevel indexLevel) :
  m_IndexLevel(indexLevel),
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
  int forward, int sector, int layer, int plane, int strip,
  enum IndexLevel indexLevel) :
  m_IndexLevel(indexLevel),
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
  static BKLMChannelIndex index(0, 1, 1, 0, 1, m_IndexLevel);
  return index;
}

BKLMChannelIndex& BKLMChannelIndex::end()
{
  static BKLMChannelIndex index(
    BKLMElementNumbers::getMaximalForwardNumber() + 1, 1, 1, 0, 1, m_IndexLevel);
  return index;
}

void BKLMChannelIndex::increment(enum IndexLevel indexLevel)
{
  switch (indexLevel) {
    case c_IndexLevelStrip:
      m_Strip++;
      if (m_Strip > m_NStripsPlane) {
        m_Strip = 1;
        increment(c_IndexLevelPlane);
        m_NStripsPlane = BKLMElementNumbers::getNStrips(
                           m_Forward, m_Sector, m_Layer, m_Plane);
      }
      break;
    case c_IndexLevelPlane:
      m_Plane++;
      if (m_Plane > BKLMElementNumbers::getMaximalPlaneNumber()) {
        m_Plane = 0;
        increment(c_IndexLevelLayer);
      }
      break;
    case c_IndexLevelLayer:
      m_Layer++;
      if (m_Layer > BKLMElementNumbers::getMaximalLayerNumber()) {
        m_Layer = 1;
        increment(c_IndexLevelSector);
      }
      break;
    case c_IndexLevelSector:
      m_Sector++;
      if (m_Sector > BKLMElementNumbers::getMaximalSectorNumber()) {
        m_Sector = 1;
        increment(c_IndexLevelForward);
      }
      break;
    case c_IndexLevelForward:
      m_Forward++;
      break;
  }
}

BKLMChannelIndex& BKLMChannelIndex::operator++()
{
  increment(m_IndexLevel);
  return *this;
}

bool BKLMChannelIndex::operator==(BKLMChannelIndex& index)
{
  switch (m_IndexLevel) {
    case c_IndexLevelStrip:
      if (m_Strip != index.getStrip())
        return false;
      [[fallthrough]];
    case c_IndexLevelPlane:
      if (m_Plane != index.getPlane())
        return false;
      [[fallthrough]];
    case c_IndexLevelLayer:
      if (m_Layer != index.getLayer())
        return false;
      [[fallthrough]];
    case c_IndexLevelSector:
      if (m_Sector != index.getSector())
        return false;
      [[fallthrough]];
    case c_IndexLevelForward:
      if (m_Forward != index.getForward())
        return false;
  }
  return true;
}

bool BKLMChannelIndex::operator!=(BKLMChannelIndex& index)
{
  switch (m_IndexLevel) {
    case c_IndexLevelStrip:
      if (m_Strip != index.getStrip())
        return true;
      [[fallthrough]];
    case c_IndexLevelPlane:
      if (m_Plane != index.getPlane())
        return true;
      [[fallthrough]];
    case c_IndexLevelLayer:
      if (m_Layer != index.getLayer())
        return true;
      [[fallthrough]];
    case c_IndexLevelSector:
      if (m_Sector != index.getSector())
        return true;
      [[fallthrough]];
    case c_IndexLevelForward:
      if (m_Forward != index.getForward())
        return true;
  }
  return false;
}

BKLMChannelIndex& BKLMChannelIndex::operator*()
{
  return *this;
}
