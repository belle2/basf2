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
#include <eklm/geometry/GeometryData2.h>
#include <eklm/geometry/TransformData.h>

using namespace Belle2;

void EKLM::fillTransforms(struct TransformData* dat)
{
  int i1;
  int i2;
  int i3;
  int i4;
  int i5;
  const GeometryData2& geoDat = GeometryData2::Instance();
  for (i1 = 0; i1 < 2; i1++) {
    geoDat.getEndcapTransform(&(dat->endcap[i1]), i1);
    for (i2 = 0; i2 < 14; i2++) {
      geoDat.getLayerTransform(&(dat->layer[i1][i2]), i2);
      for (i3 = 0; i3 < 4; i3++) {
        geoDat.getSectorTransform(&(dat->sector[i1][i2][i3]), i3);
        for (i4 = 0; i4 < 2; i4++) {
          geoDat.getPlaneTransform(&(dat->plane[i1][i2][i3][i4]), i4);
          for (i5 = 0; i5 < 75; i5++) {
            geoDat.getStripTransform(&(dat->strip[i1][i2][i3][i4][i5]), i5);
          }
        }
      }
    }
  }
}

void EKLM::transformsToGlobal(struct EKLM::TransformData* dat)
{
  int i1;
  int i2;
  int i3;
  int i4;
  int i5;
  for (i1 = 0; i1 < 2; i1++) {
    for (i2 = 0; i2 < 14; i2++) {
      dat->layer[i1][i2] = dat->endcap[i1] * dat->layer[i1][i2];
      for (i3 = 0; i3 < 4; i3++) {
        dat->sector[i1][i2][i3] = dat->layer[i1][i2] * dat->sector[i1][i2][i3];
        for (i4 = 0; i4 < 2; i4++) {
          dat->plane[i1][i2][i3][i4] = dat->sector[i1][i2][i3] *
                                       dat->plane[i1][i2][i3][i4];
          for (i5 = 0; i5 < 75; i5++) {
            dat->strip[i1][i2][i3][i4][i5] = dat->plane[i1][i2][i3][i4] *
                                             dat->strip[i1][i2][i3][i4][i5];
            dat->stripInverse[i1][i2][i3][i4][i5] =
              dat->strip[i1][i2][i3][i4][i5].inverse();
          }
        }
      }
    }
  }
}

HepGeom::Transform3D* EKLM::getStripLocalToGlobal(
  struct EKLM::TransformData* dat, EKLMDigit* hit)
{
  return &(dat->strip[hit->getEndcap() - 1][hit->getLayer() - 1]
           [hit->getSector() - 1][hit->getPlane() - 1][hit->getStrip() - 1]);
}


HepGeom::Transform3D* EKLM::getStripGlobalToLocal(
  struct EKLM::TransformData* dat, EKLMDigit* hit)
{
  return &(dat->stripInverse[hit->getEndcap() - 1][hit->getLayer() - 1]
           [hit->getSector() - 1][hit->getPlane() - 1][hit->getStrip() - 1]);
}


