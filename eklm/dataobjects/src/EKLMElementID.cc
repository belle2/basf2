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
#include <eklm/dataobjects/EKLMElementID.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMElementID::EKLMElementID() : m_Type(c_Endcap), m_Endcap(-1), m_Layer(-1),
  m_Sector(-1), m_Plane(-1), m_Segment(-1)
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

EKLMElementID::EKLMElementID(int endcap, int layer, int sector) :
  m_Type(c_Sector), m_Endcap(endcap), m_Layer(layer), m_Sector(sector),
  m_Plane(-1), m_Segment(-1)
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

EKLMElementID::EKLMElementID(
  int endcap, int layer, int sector, int plane, int segment) :
  m_Type(c_Segment), m_Endcap(endcap), m_Layer(layer), m_Sector(sector),
  m_Plane(plane), m_Segment(segment)
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

EKLMElementID::EKLMElementID(int globalNumber)
{
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
  int id = globalNumber;
  if (id <= 0)
    B2FATAL("Incorrect (non-positive) EKLM global element number.");
  if (id <= m_ElementNumbers->getMaximalEndcapNumber()) {
    m_Type = c_Endcap;
    m_Endcap = id;
    return;
  }
  id -= m_ElementNumbers->getMaximalEndcapNumber();
  if (id <= m_ElementNumbers->getMaximalLayerGlobalNumber()) {
    m_Type = c_Layer;
    m_ElementNumbers->layerNumberToElementNumbers(id, &m_Endcap, &m_Layer);
    return;
  }
  id -= m_ElementNumbers->getMaximalLayerGlobalNumber();
  if (id <= m_ElementNumbers->getMaximalSectorGlobalNumber()) {
    m_Type = c_Sector;
    m_ElementNumbers->sectorNumberToElementNumbers(
      id, &m_Endcap, &m_Layer, &m_Sector);
    return;
  }
  id -= m_ElementNumbers->getMaximalSectorGlobalNumber();
  if (id <= m_ElementNumbers->getMaximalPlaneGlobalNumber()) {
    m_Type = c_Plane;
    m_ElementNumbers->planeNumberToElementNumbers(
      id, &m_Endcap, &m_Layer, &m_Sector, &m_Plane);
    return;
  }
  id -= m_ElementNumbers->getMaximalPlaneGlobalNumber();
  if (id <= m_ElementNumbers->getMaximalSegmentGlobalNumber()) {
    m_Type = c_Segment;
    m_ElementNumbers->segmentNumberToElementNumbers(
      id, &m_Endcap, &m_Layer, &m_Sector, &m_Plane, &m_Segment);
    return;
  }
  B2FATAL("Incorrect (too large) EKLM global element number.");
}

EKLMElementID::~EKLMElementID()
{
}

void EKLMElementID::setType(ElementType type)
{
  m_Type = type;
}

EKLMElementID::ElementType EKLMElementID::getType() const
{
  return m_Type;
}

void EKLMElementID::setEndcap(int endcap)
{
  m_Endcap = endcap;
}

int EKLMElementID::getEndcap() const
{
  return m_Endcap;
}

void EKLMElementID::setLayer(int layer)
{
  m_Layer = layer;
}

int EKLMElementID::getLayer() const
{
  return m_Layer;
}

void EKLMElementID::setSector(int sector)
{
  m_Sector = sector;
}

int EKLMElementID::getSector() const
{
  return m_Sector;
}

void EKLMElementID::setPlane(int plane)
{
  m_Plane = plane;
}

int EKLMElementID::getPlane() const
{
  return m_Plane;
}

void EKLMElementID::setSegment(int segment)
{
  m_Segment = segment;
}

int EKLMElementID::getSegment() const
{
  return m_Segment;
}

int EKLMElementID::getLayerNumber() const
{
  return m_ElementNumbers->detectorLayerNumber(m_Endcap, m_Layer);
}

int EKLMElementID::getSectorNumber() const
{
  return m_ElementNumbers->sectorNumber(m_Endcap, m_Layer, m_Sector);
}

int EKLMElementID::getPlaneNumber() const
{
  return m_ElementNumbers->planeNumber(m_Endcap, m_Layer, m_Sector, m_Plane);
}

int EKLMElementID::getSegmentNumber() const
{
  return m_ElementNumbers->segmentNumber(m_Endcap, m_Layer, m_Sector,
                                         m_Plane, m_Segment);
}

int EKLMElementID::getGlobalNumber() const
{
  int offset = 0;
  if (m_Type == c_Endcap)
    return m_Endcap;
  else
    offset += m_ElementNumbers->getMaximalEndcapNumber();
  if (m_Type == c_Layer)
    return offset + m_ElementNumbers->detectorLayerNumber(m_Endcap, m_Layer);
  else
    offset += m_ElementNumbers->getMaximalLayerGlobalNumber();
  if (m_Type == c_Sector)
    return offset + m_ElementNumbers->sectorNumber(m_Endcap, m_Layer, m_Sector);
  else
    offset += m_ElementNumbers->getMaximalSectorGlobalNumber();
  if (m_Type == c_Plane)
    return offset + m_ElementNumbers->planeNumber(m_Endcap, m_Layer, m_Sector,
                                                  m_Plane);
  else
    offset += m_ElementNumbers->getMaximalPlaneGlobalNumber();
  return offset + m_ElementNumbers->segmentNumber(m_Endcap, m_Layer, m_Sector,
                                                  m_Plane, m_Segment);
}

