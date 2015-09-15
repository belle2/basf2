/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/geometry/EKLMObjectNumbers.h>
#include <eklm/geometry/GeometryData2.h>
#include <eklm/geometry/TransformData.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLM::TransformData::TransformData(bool global)
{
  int iEndcap, iLayer, iSector, iPlane, iSegment, iStrip, segment;
  EKLMAlignmentData* alignmentData;
  const GeometryData2& geoDat = GeometryData2::Instance();
  for (iEndcap = 0; iEndcap < 2; iEndcap++) {
    geoDat.getEndcapTransform(&m_Endcap[iEndcap], iEndcap);
    for (iLayer = 0; iLayer < 14; iLayer++) {
      geoDat.getLayerTransform(&m_Layer[iEndcap][iLayer], iLayer);
      for (iSector = 0; iSector < 4; iSector++) {
        geoDat.getSectorTransform(&m_Sector[iEndcap][iLayer][iSector], iSector);
        for (iPlane = 0; iPlane < 2; iPlane++) {
          geoDat.getPlaneTransform(&m_Plane[iEndcap][iLayer][iSector][iPlane],
                                   iPlane);
          for (iStrip = 0; iStrip < 75; iStrip++) {
            geoDat.getStripTransform(
              &m_Strip[iEndcap][iLayer][iSector][iPlane][iStrip], iStrip);
          }
        }
      }
    }
  }
  /* Read alignment data from the database and modify transformations. */
  DBObjPtr<EKLMAlignment> alignment("EKLMAlignment");
  if (alignment.isValid()) {
    for (iEndcap = 1; iEndcap <= 2; iEndcap++) {
      for (iLayer = 1; iLayer <= EKLM::GeometryData2::Instance().
           getNDetectorLayers(iEndcap); iLayer++) {
        for (iSector = 1; iSector <= 4; iSector++) {
          for (iPlane = 1; iPlane <= 2; iPlane++) {
            for (iSegment = 1; iSegment <= 5; iSegment++) {
              segment = EKLM::segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                            iSegment);
              alignmentData = alignment->getAlignmentData(segment);
              if (alignmentData == NULL)
                B2FATAL("Incomplete alignment data in the database.");
            }
          }
        }
      }
    }
  } else
    B2INFO("Could not read alignment data from the database, "
           "using default positions.");
  if (global)
    transformsToGlobal();
}

EKLM::TransformData::~TransformData()
{
}

void EKLM::TransformData::transformsToGlobal()
{
  int iEndcap, iLayer, iSector, iPlane, iStrip;
  for (iEndcap = 0; iEndcap < 2; iEndcap++) {
    for (iLayer = 0; iLayer < 14; iLayer++) {
      m_Layer[iEndcap][iLayer] = m_Endcap[iEndcap] * m_Layer[iEndcap][iLayer];
      for (iSector = 0; iSector < 4; iSector++) {
        m_Sector[iEndcap][iLayer][iSector] =
          m_Layer[iEndcap][iLayer] * m_Sector[iEndcap][iLayer][iSector];
        for (iPlane = 0; iPlane < 2; iPlane++) {
          m_Plane[iEndcap][iLayer][iSector][iPlane] =
            m_Sector[iEndcap][iLayer][iSector] *
            m_Plane[iEndcap][iLayer][iSector][iPlane];
          for (iStrip = 0; iStrip < 75; iStrip++) {
            m_Strip[iEndcap][iLayer][iSector][iPlane][iStrip] =
              m_Plane[iEndcap][iLayer][iSector][iPlane] *
              m_Strip[iEndcap][iLayer][iSector][iPlane][iStrip];
            m_StripInverse[iEndcap][iLayer][iSector][iPlane][iStrip] =
              m_Strip[iEndcap][iLayer][iSector][iPlane][iStrip].inverse();
          }
        }
      }
    }
  }
}

const HepGeom::Transform3D*
EKLM::TransformData::getEndcapTransform(int endcap) const
{
  return &m_Endcap[endcap - 1];
}

const HepGeom::Transform3D*
EKLM::TransformData::getLayerTransform(int endcap, int layer) const
{
  return &m_Layer[endcap - 1][layer - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getSectorTransform(int endcap, int layer, int sector) const
{
  return &m_Sector[endcap - 1][layer - 1][sector - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getPlaneTransform(int endcap, int layer, int sector, int plane) const
{
  return &m_Plane[endcap - 1][layer - 1][sector - 1][plane - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getStripTransform(int endcap, int layer, int sector, int plane, int strip) const
{
  return &m_Strip[endcap - 1][layer - 1][sector - 1][plane - 1][strip - 1];
}

const HepGeom::Transform3D*
EKLM::TransformData::getStripLocalToGlobal(EKLMDigit* hit) const
{
  return &(m_Strip[hit->getEndcap() - 1][hit->getLayer() - 1]
           [hit->getSector() - 1][hit->getPlane() - 1][hit->getStrip() - 1]);
}

const HepGeom::Transform3D*
EKLM::TransformData::getStripGlobalToLocal(EKLMDigit* hit) const
{
  return &(m_StripInverse[hit->getEndcap() - 1][hit->getLayer() - 1]
           [hit->getSector() - 1][hit->getPlane() - 1][hit->getStrip() - 1]);
}

