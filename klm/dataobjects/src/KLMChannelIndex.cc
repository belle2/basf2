/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

using namespace Belle2;

KLMChannelIndex::KLMChannelIndex(enum IndexLevel indexLevel) :
  m_IndexLevel(indexLevel),
  m_Subdetector(KLMElementNumbers::c_BKLM),
  m_Section(0),
  m_Sector(1),
  m_Layer(1),
  m_Plane(0),
  m_Strip(1),
  m_ElementNumbers(&(KLMElementNumbers::Instance())),
  m_eklmElementNumbers(&(EKLMElementNumbers::Instance()))
{
  setNStripsPlane();
}

KLMChannelIndex::KLMChannelIndex(
  int subdetector, int section, int sector, int layer, int plane, int strip,
  enum IndexLevel indexLevel) :
  m_IndexLevel(indexLevel),
  m_Subdetector(subdetector),
  m_Section(section),
  m_Sector(sector),
  m_Layer(layer),
  m_Plane(plane),
  m_Strip(strip),
  m_ElementNumbers(&(KLMElementNumbers::Instance())),
  m_eklmElementNumbers(&(EKLMElementNumbers::Instance()))
{
  setNStripsPlane();
}

KLMChannelIndex::~KLMChannelIndex()
{
}

void KLMChannelIndex::setNStripsPlane()
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM) {
    m_NStripsPlane = BKLMElementNumbers::getNStrips(
                       m_Section, m_Sector, m_Layer, m_Plane);
  } else {
    if (m_UseEKLMSegments)
      m_NStripsPlane = EKLMElementNumbers::getMaximalSegmentNumber();
    else
      m_NStripsPlane = EKLMElementNumbers::getMaximalStripNumber();
  }
}

void KLMChannelIndex::setIndexLevel(enum IndexLevel indexLevel)
{
  if (indexLevel > m_IndexLevel) {
    if (m_Subdetector == KLMElementNumbers::c_BKLM) {
      switch (m_IndexLevel) {
        case c_IndexLevelSubdetector:
          m_Section = 0;
          [[fallthrough]];
        case c_IndexLevelSection:
          m_Sector = 1;
          [[fallthrough]];
        case c_IndexLevelSector:
          m_Layer = 1;
          [[fallthrough]];
        case c_IndexLevelLayer:
          m_Plane = 0;
          [[fallthrough]];
        case c_IndexLevelPlane:
          m_Strip = 1;
          [[fallthrough]];
        case c_IndexLevelStrip:
          break;
      }
    } else {
      switch (m_IndexLevel) {
        case c_IndexLevelSubdetector:
          m_Section = 1;
          [[fallthrough]];
        case c_IndexLevelSection:
          m_Sector = 1;
          [[fallthrough]];
        case c_IndexLevelSector:
          m_Layer = 1;
          [[fallthrough]];
        case c_IndexLevelLayer:
          m_Plane = 1;
          [[fallthrough]];
        case c_IndexLevelPlane:
          m_Strip = 1;
          [[fallthrough]];
        case c_IndexLevelStrip:
          break;
      }
    }
  }
  m_IndexLevel = indexLevel;
  if (indexLevel == c_IndexLevelStrip)
    setNStripsPlane();
}

void KLMChannelIndex::useEKLMSegments(bool useSegments)
{
  m_UseEKLMSegments = useSegments;
  setNStripsPlane();
}

void KLMChannelIndex::setKLMModule(KLMModuleNumber module)
{
  m_ElementNumbers->moduleNumberToElementNumbers(
    module, &m_Subdetector, &m_Section, &m_Sector, &m_Layer);
  if (m_Subdetector == KLMElementNumbers::c_BKLM)
    m_Plane = 0;
  else
    m_Plane = 1;
  m_Strip = 1;
  useEKLMSegments(false);
  m_IndexLevel = c_IndexLevelLayer;
}

void KLMChannelIndex::setEKLMSegment(int segment)
{
  m_eklmElementNumbers->segmentNumberToElementNumbers(
    segment, &m_Section, &m_Sector, &m_Layer, &m_Plane, &m_Strip);
  m_Subdetector = KLMElementNumbers::c_EKLM;
  useEKLMSegments();
  m_IndexLevel = c_IndexLevelStrip;
}

KLMChannelNumber KLMChannelIndex::getKLMChannelNumber() const
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM) {
    return m_ElementNumbers->channelNumberBKLM(
             m_Section, m_Sector, m_Layer, m_Plane, m_Strip);
  } else {
    return m_ElementNumbers->channelNumberEKLM(
             m_Section, m_Sector, m_Layer, m_Plane, m_Strip);
  }
}

KLMPlaneNumber KLMChannelIndex::getKLMPlaneNumber() const
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM) {
    return m_ElementNumbers->planeNumberBKLM(
             m_Section, m_Sector, m_Layer, m_Plane);
  } else {
    return m_ElementNumbers->planeNumberEKLM(
             m_Section, m_Sector, m_Layer, m_Plane);
  }
}

KLMModuleNumber KLMChannelIndex::getKLMModuleNumber() const
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM)
    return m_ElementNumbers->moduleNumberBKLM(m_Section, m_Sector, m_Layer);
  else
    return m_ElementNumbers->moduleNumberEKLM(m_Section, m_Sector, m_Layer);
}

KLMSectorNumber KLMChannelIndex::getKLMSectorNumber() const
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM)
    return m_ElementNumbers->sectorNumberBKLM(m_Section, m_Sector);
  else
    return m_ElementNumbers->sectorNumberEKLM(m_Section, m_Sector);
}

int KLMChannelIndex::getEKLMSegmentNumber() const
{
  return m_eklmElementNumbers->segmentNumber(
           m_Section, m_Layer, m_Sector, m_Plane, m_Strip);
}

KLMChannelIndex KLMChannelIndex::beginBKLM()
{
  return KLMChannelIndex(KLMElementNumbers::c_BKLM, 0, 1, 1, 0, 1,
                         m_IndexLevel);
}

KLMChannelIndex& KLMChannelIndex::endBKLM()
{
  /*
   * The index level does not matter for end check, thus,
   * the object can be created once.
   */
  static KLMChannelIndex index(KLMElementNumbers::c_EKLM, 1, 1, 1, 1, 1);
  return index;
}

KLMChannelIndex KLMChannelIndex::beginEKLM()
{
  return KLMChannelIndex(KLMElementNumbers::c_EKLM, 1, 1, 1, 1, 1,
                         m_IndexLevel);
}

KLMChannelIndex& KLMChannelIndex::endEKLM()
{
  /*
   * The index level does not matter for end check, thus,
   * the object can be created once.
   */
  static KLMChannelIndex index(KLMElementNumbers::c_EKLM + 1, 1, 1, 1, 1, 1);
  return index;
}

void KLMChannelIndex::increment(enum IndexLevel indexLevel)
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM) {
    switch (indexLevel) {
      case c_IndexLevelStrip:
        m_Strip++;
        if (m_Strip > m_NStripsPlane) {
          m_Strip = 1;
          increment(c_IndexLevelPlane);
          setNStripsPlane();
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
          increment(c_IndexLevelSection);
        }
        break;
      case c_IndexLevelSection:
        m_Section++;
        if (m_Section > BKLMElementNumbers::getMaximalSectionNumber()) {
          m_Section = 0;
          increment(c_IndexLevelSubdetector);
        }
        break;
      case c_IndexLevelSubdetector:
        *this = beginEKLM();
        break;
    }
  } else {
    switch (indexLevel) {
      case c_IndexLevelStrip:
        m_Strip++;
        if (m_Strip > m_NStripsPlane) {
          m_Strip = 1;
          increment(c_IndexLevelPlane);
          setNStripsPlane();
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
        if (m_Layer > m_eklmElementNumbers->getMaximalDetectorLayerNumber(m_Section)) {
          m_Layer = 1;
          increment(c_IndexLevelSector);
        }
        break;
      case c_IndexLevelSector:
        m_Sector++;
        if (m_Sector > EKLMElementNumbers::getMaximalSectorNumber()) {
          m_Sector = 1;
          increment(c_IndexLevelSection);
        }
        break;
      case c_IndexLevelSection:
        m_Section++;
        if (m_Section > EKLMElementNumbers::getMaximalSectionNumber()) {
          m_Section = 1;
          increment(c_IndexLevelSubdetector);
        }
        break;
      case c_IndexLevelSubdetector:
        m_Subdetector++;
        break;
    }
  }
}

KLMChannelIndex& KLMChannelIndex::operator++()
{
  increment(m_IndexLevel);
  return *this;
}

KLMChannelIndex& KLMChannelIndex::increment()
{
  increment(m_IndexLevel);
  return *this;
}

bool KLMChannelIndex::operator==(const KLMChannelIndex& index)
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
    case c_IndexLevelSection:
      if (m_Section != index.getSection())
        return false;
      [[fallthrough]];
    case c_IndexLevelSubdetector:
      if (m_Subdetector != index.getSubdetector())
        return false;
  }
  return true;
}

bool KLMChannelIndex::operator!=(const KLMChannelIndex& index)
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
    case c_IndexLevelSection:
      if (m_Section != index.getSection())
        return true;
      [[fallthrough]];
    case c_IndexLevelSubdetector:
      if (m_Subdetector != index.getSubdetector())
        return true;
  }
  return false;
}

KLMChannelIndex& KLMChannelIndex::operator*()
{
  return *this;
}
