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
#include <eklm/dataobjects/EKLMSegmentID.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>

using namespace Belle2;

EKLMSegmentID::EKLMSegmentID() : m_Endcap(-1), m_Layer(-1), m_Sector(-1),
  m_Plane(-1), m_Segment(-1)
{
}

EKLMSegmentID::EKLMSegmentID(
  int endcap, int layer, int sector, int plane, int segment) : m_Endcap(endcap),
  m_Layer(layer), m_Sector(sector), m_Plane(plane), m_Segment(segment)
{
}

EKLMSegmentID::EKLMSegmentID(int segment)
{
  static const EKLM::ElementNumbersSingleton& elementNumbers =
    EKLM::ElementNumbersSingleton::Instance();
  elementNumbers.segmentNumberToElementNumbers(
    segment, &m_Endcap, &m_Layer, &m_Sector, &m_Plane, &m_Segment);
}

EKLMSegmentID::~EKLMSegmentID()
{
}

void EKLMSegmentID::setEndcap(int endcap)
{
  m_Endcap = endcap;
}

int EKLMSegmentID::getEndcap() const
{
  return m_Endcap;
}

void EKLMSegmentID::setLayer(int layer)
{
  m_Layer = layer;
}

int EKLMSegmentID::getLayer() const
{
  return m_Layer;
}

void EKLMSegmentID::setSector(int sector)
{
  m_Sector = sector;
}

int EKLMSegmentID::getSector() const
{
  return m_Sector;
}

void EKLMSegmentID::setPlane(int plane)
{
  m_Plane = plane;
}

int EKLMSegmentID::getPlane() const
{
  return m_Plane;
}

void EKLMSegmentID::setSegment(int segment)
{
  m_Segment = segment;
}

int EKLMSegmentID::getSegment() const
{
  return m_Segment;
}

int EKLMSegmentID::getSegmentGlobalNumber() const
{
  static const EKLM::ElementNumbersSingleton& elementNumbers =
    EKLM::ElementNumbersSingleton::Instance();
  return elementNumbers.segmentNumber(m_Endcap, m_Layer, m_Sector, m_Plane,
                                      m_Segment);
}

