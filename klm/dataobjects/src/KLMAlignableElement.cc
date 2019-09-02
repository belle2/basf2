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
#include <klm/dataobjects/KLMAlignableElement.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMAlignableElement::KLMAlignableElement() :
  m_Type(c_EKLMSector),
  m_Subdetector(-1),
  m_Section(-1),
  m_Sector(-1),
  m_Layer(-1),
  m_Plane(-1),
  m_Segment(-1)
{
}

KLMAlignableElement::KLMAlignableElement(
  enum ElementType type, int section, int sector, int layer, int plane,
  int segment) :
  m_Type(type),
  m_Section(section),
  m_Sector(sector),
  m_Layer(layer),
  m_Plane(plane),
  m_Segment(segment)
{
  if (m_Type == c_BKLMModule)
    m_Subdetector = KLMElementNumbers::c_BKLM;
  else
    m_Subdetector = KLMElementNumbers::c_EKLM;
}

KLMAlignableElement::KLMAlignableElement(int globalNumber)
{
  const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
  const EKLM::ElementNumbersSingleton* eklmElementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  if (globalNumber <= 0)
    B2FATAL("Incorrect (non-positive) KLM alignable element number.");
  if (globalNumber < c_EKLMSegmentOffset) {
    uint16_t module = globalNumber;
    elementNumbers->moduleNumberToElementNumbers(
      module, &m_Subdetector, &m_Section, &m_Sector, &m_Layer);
    if (m_Subdetector == KLMElementNumbers::c_EKLM)
      m_Type = c_EKLMSector;
    else
      m_Type = c_BKLMModule;
    m_Plane = 0;
    m_Segment = 0;
  } else {
    m_Type = c_EKLMSegment;
    m_Subdetector = KLMElementNumbers::c_EKLM;
    int segment = globalNumber - c_EKLMSegmentOffset;
    eklmElementNumbers->segmentNumberToElementNumbers(
      segment, &m_Section, &m_Layer, &m_Sector, &m_Plane, &m_Segment);
    return;
  }
}

KLMAlignableElement::~KLMAlignableElement()
{
}

void KLMAlignableElement::setType(ElementType type)
{
  m_Type = type;
}

KLMAlignableElement::ElementType KLMAlignableElement::getType() const
{
  return m_Type;
}

void KLMAlignableElement::setSection(int section)
{
  m_Section = section;
}

int KLMAlignableElement::getSection() const
{
  return m_Section;
}

void KLMAlignableElement::setLayer(int layer)
{
  m_Layer = layer;
}

int KLMAlignableElement::getLayer() const
{
  return m_Layer;
}

void KLMAlignableElement::setSector(int sector)
{
  m_Sector = sector;
}

int KLMAlignableElement::getSector() const
{
  return m_Sector;
}

void KLMAlignableElement::setPlane(int plane)
{
  m_Plane = plane;
}

int KLMAlignableElement::getPlane() const
{
  return m_Plane;
}

void KLMAlignableElement::setSegment(int segment)
{
  m_Segment = segment;
}

int KLMAlignableElement::getSegment() const
{
  return m_Segment;
}

uint16_t KLMAlignableElement::getModuleNumber() const
{
  const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
  return elementNumbers->moduleNumber(
           m_Subdetector, m_Section, m_Sector, m_Layer);
}

int KLMAlignableElement::getEKLMSegmentNumber() const
{
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  return elementNumbers->segmentNumber(m_Section, m_Layer, m_Sector,
                                       m_Plane, m_Segment);
}

int KLMAlignableElement::getNumber() const
{
  if (m_Type == c_EKLMSector || m_Type == c_BKLMModule)
    return getModuleNumber();
  else
    return c_EKLMSegmentOffset + getEKLMSegmentNumber();
}

