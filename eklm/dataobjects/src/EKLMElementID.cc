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
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMElementID::EKLMElementID() : m_Type(c_Section), m_Section(-1), m_Layer(-1),
  m_Sector(-1), m_Plane(-1), m_Segment(-1)
{
}

EKLMElementID::EKLMElementID(int section, int layer, int sector) :
  m_Type(c_Sector), m_Section(section), m_Layer(layer), m_Sector(sector),
  m_Plane(-1), m_Segment(-1)
{
}

EKLMElementID::EKLMElementID(
  int section, int layer, int sector, int plane, int segment) :
  m_Type(c_Segment), m_Section(section), m_Layer(layer), m_Sector(sector),
  m_Plane(plane), m_Segment(segment)
{
}

EKLMElementID::EKLMElementID(int globalNumber)
{
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  int id = globalNumber;
  if (id <= 0)
    B2FATAL("Incorrect (non-positive) EKLM global element number.");
  if (id <= elementNumbers->getMaximalSectionNumber()) {
    m_Type = c_Section;
    m_Section = id;
    return;
  }
  id -= elementNumbers->getMaximalSectionNumber();
  if (id <= elementNumbers->getMaximalLayerGlobalNumber()) {
    m_Type = c_Layer;
    elementNumbers->layerNumberToElementNumbers(id, &m_Section, &m_Layer);
    return;
  }
  id -= elementNumbers->getMaximalLayerGlobalNumber();
  if (id <= elementNumbers->getMaximalSectorGlobalNumber()) {
    m_Type = c_Sector;
    elementNumbers->sectorNumberToElementNumbers(
      id, &m_Section, &m_Layer, &m_Sector);
    return;
  }
  id -= elementNumbers->getMaximalSectorGlobalNumber();
  if (id <= elementNumbers->getMaximalPlaneGlobalNumber()) {
    m_Type = c_Plane;
    elementNumbers->planeNumberToElementNumbers(
      id, &m_Section, &m_Layer, &m_Sector, &m_Plane);
    return;
  }
  id -= elementNumbers->getMaximalPlaneGlobalNumber();
  if (id <= elementNumbers->getMaximalSegmentGlobalNumber()) {
    m_Type = c_Segment;
    elementNumbers->segmentNumberToElementNumbers(
      id, &m_Section, &m_Layer, &m_Sector, &m_Plane, &m_Segment);
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

void EKLMElementID::setSection(int section)
{
  m_Section = section;
}

int EKLMElementID::getSection() const
{
  return m_Section;
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
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  return elementNumbers->detectorLayerNumber(m_Section, m_Layer);
}

int EKLMElementID::getSectorNumber() const
{
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  return elementNumbers->sectorNumber(m_Section, m_Layer, m_Sector);
}

int EKLMElementID::getPlaneNumber() const
{
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  return elementNumbers->planeNumber(m_Section, m_Layer, m_Sector, m_Plane);
}

int EKLMElementID::getSegmentNumber() const
{
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  return elementNumbers->segmentNumber(m_Section, m_Layer, m_Sector,
                                       m_Plane, m_Segment);
}

int EKLMElementID::getGlobalNumber() const
{
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  int offset = 0;
  if (m_Type == c_Section)
    return m_Section;
  else
    offset += elementNumbers->getMaximalSectionNumber();
  if (m_Type == c_Layer)
    return offset + elementNumbers->detectorLayerNumber(m_Section, m_Layer);
  else
    offset += elementNumbers->getMaximalLayerGlobalNumber();
  if (m_Type == c_Sector)
    return offset + elementNumbers->sectorNumber(m_Section, m_Layer, m_Sector);
  else
    offset += elementNumbers->getMaximalSectorGlobalNumber();
  if (m_Type == c_Plane)
    return offset + elementNumbers->planeNumber(m_Section, m_Layer, m_Sector,
                                                m_Plane);
  else
    offset += elementNumbers->getMaximalPlaneGlobalNumber();
  return offset + elementNumbers->segmentNumber(m_Section, m_Layer, m_Sector,
                                                m_Plane, m_Segment);
}

