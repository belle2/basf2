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

using namespace Belle2;

EKLMElementID::EKLMElementID() : m_Endcap(-1), m_Layer(-1), m_Sector(-1),
  m_Plane(-1), m_Segment(-1)
{
}

EKLMElementID::EKLMElementID(
  int endcap, int layer, int sector, int plane, int segment) : m_Endcap(endcap),
  m_Layer(layer), m_Sector(sector), m_Plane(plane), m_Segment(segment)
{
}

EKLMElementID::EKLMElementID(int segment)
{
  static const EKLM::ElementNumbersSingleton& elementNumbers =
    EKLM::ElementNumbersSingleton::Instance();
  elementNumbers.segmentNumberToElementNumbers(
    segment, &m_Endcap, &m_Layer, &m_Sector, &m_Plane, &m_Segment);
}

EKLMElementID::~EKLMElementID()
{
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

int EKLMElementID::getSegmentGlobalNumber() const
{
  static const EKLM::ElementNumbersSingleton& elementNumbers =
    EKLM::ElementNumbersSingleton::Instance();
  return elementNumbers.segmentNumber(m_Endcap, m_Layer, m_Sector, m_Plane,
                                      m_Segment);
}

