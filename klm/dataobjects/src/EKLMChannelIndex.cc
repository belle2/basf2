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
#include <eklm/dataobjects/EKLMElementNumbers.h>
#include <klm/dataobjects/EKLMChannelIndex.h>

using namespace Belle2;

EKLMChannelIndex::EKLMChannelIndex(enum IndexLevel indexLevel) :
  m_IndexLevel(indexLevel),
  m_Endcap(1),
  m_Sector(1),
  m_Layer(1),
  m_Plane(1),
  m_Strip(1)
{
  m_ElementNumbers = &(KLMElementNumbers::Instance());
}

EKLMChannelIndex::EKLMChannelIndex(
  int endcap, int sector, int layer, int plane, int strip,
  enum IndexLevel indexLevel) :
  m_IndexLevel(indexLevel),
  m_Endcap(endcap),
  m_Sector(sector),
  m_Layer(layer),
  m_Plane(plane),
  m_Strip(strip)
{
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_ElementNumbersEKLM = &(EKLM::ElementNumbersSingleton::Instance());
}

EKLMChannelIndex::~EKLMChannelIndex()
{
}

uint16_t EKLMChannelIndex::getKLMChannelNumber()
{
  return m_ElementNumbers->channelNumberEKLM(
           m_Endcap, m_Layer, m_Sector, m_Plane, m_Strip);
}

EKLMChannelIndex& EKLMChannelIndex::begin()
{
  static EKLMChannelIndex index(1, 1, 1, 1, 1, m_IndexLevel);
  return index;
}

EKLMChannelIndex& EKLMChannelIndex::end()
{
  static EKLMChannelIndex index(
    EKLMElementNumbers::getMaximalEndcapNumber(),
    EKLMElementNumbers::getMaximalSectorNumber(),
    EKLMElementNumbers::getMaximalLayerNumber(),
    EKLMElementNumbers::getMaximalPlaneNumber(),
    EKLMElementNumbers::getMaximalStripNumber(),
    m_IndexLevel);
  return index;
}

void EKLMChannelIndex::increment(enum IndexLevel indexLevel)
{
  switch (indexLevel) {
    case c_IndexLevelStrip:
      m_Strip++;
      if (m_Strip > EKLMElementNumbers::getMaximalStripNumber()) {
        m_Strip = 1;
        increment(c_IndexLevelPlane);
      }
      break;
    case c_IndexLevelPlane:
      m_Plane++;
      if (m_Plane > EKLMElementNumbers::getMaximalPlaneNumber()) {
        m_Plane = 1;
        increment(c_IndexLevelLayer);
      }
      break;
    case c_IndexLevelLayer:
      m_Layer++;
      if (m_Layer > EKLMElementNumbers::getMaximalLayerNumber()) {
        m_Layer = 1;
        increment(c_IndexLevelSector);
      }
      break;
    case c_IndexLevelSector:
      m_Sector++;
      if (m_Sector > EKLMElementNumbers::getMaximalSectorNumber()) {
        m_Sector = 1;
        increment(c_IndexLevelEndcap);
      }
      break;
    case c_IndexLevelEndcap:
      m_Endcap++;
      if (m_Endcap > EKLMElementNumbers::getMaximalEndcapNumber())
        m_Endcap = 1;
      break;
  }
}

EKLMChannelIndex& EKLMChannelIndex::operator++()
{
  increment(m_IndexLevel);
  return *this;
}

bool EKLMChannelIndex::operator==(EKLMChannelIndex& index)
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
    case c_IndexLevelEndcap:
      if (m_Endcap != index.getEndcap())
        return false;
  }
  return true;
}

bool EKLMChannelIndex::operator!=(EKLMChannelIndex& index)
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
    case c_IndexLevelEndcap:
      if (m_Endcap != index.getEndcap())
        return true;
  }
  return false;
}

EKLMChannelIndex& EKLMChannelIndex::operator*()
{
  return *this;
}
