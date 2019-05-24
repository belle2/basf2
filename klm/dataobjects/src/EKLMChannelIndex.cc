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

EKLMChannelIndex::EKLMChannelIndex() :
  m_Endcap(1),
  m_Sector(1),
  m_Layer(1),
  m_Plane(1),
  m_Strip(1)
{
  m_ElementNumbers = &(KLMElementNumbers::Instance());
}

EKLMChannelIndex::EKLMChannelIndex(
  int endcap, int sector, int layer, int plane, int strip) :
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
  static EKLMChannelIndex index(1, 1, 1, 1, 1);
  return index;
}

EKLMChannelIndex& EKLMChannelIndex::end()
{
  static EKLMChannelIndex index(
    EKLMElementNumbers::getMaximalEndcapNumber(),
    EKLMElementNumbers::getMaximalSectorNumber(),
    EKLMElementNumbers::getMaximalLayerNumber(),
    EKLMElementNumbers::getMaximalPlaneNumber(),
    EKLMElementNumbers::getMaximalStripNumber());
  return index;
}

EKLMChannelIndex& EKLMChannelIndex::operator++()
{
  m_Strip++;
  if (m_Strip > EKLMElementNumbers::getMaximalStripNumber()) {
    m_Strip = 1;
    m_Plane++;
    if (m_Plane > EKLMElementNumbers::getMaximalPlaneNumber()) {
      m_Plane = 1;
      m_Sector++;
      if (m_Sector > EKLMElementNumbers::getMaximalSectorNumber()) {
        m_Sector = 1;
        m_Layer++;
        if (m_Layer > m_ElementNumbersEKLM->getMaximalDetectorLayerNumber(m_Endcap)) {
          m_Layer = 1;
          m_Endcap++;
          if (m_Endcap > EKLMElementNumbers::getMaximalEndcapNumber())
            m_Endcap = 1;
        }
      }
    }
  }
  return *this;
}

bool EKLMChannelIndex::operator==(EKLMChannelIndex& index)
{
  if (m_Endcap != index.getEndcap())
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

bool EKLMChannelIndex::operator!=(EKLMChannelIndex& index)
{
  if (m_Endcap != index.getEndcap())
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

EKLMChannelIndex& EKLMChannelIndex::operator*()
{
  return *this;
}
