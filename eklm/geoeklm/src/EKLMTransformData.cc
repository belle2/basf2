/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

/* System headers. */
#include <fcntl.h>
#include <unistd.h>

/* Belle2 headers. */
#include <eklm/geoeklm/GeoEKLMBelleII.h>
#include <eklm/geoeklm/EKLMTransformData.h>

using namespace Belle2;

/**
 * Write transformation data to file.
 * @param[in] fd File descriptor.
 * @param[in] t  Transformation.
 * @return 0  Successful.
 * @return -1 Error.
 */
static int writeTransform(int fd, HepGeom::Transform3D* t)
{
  int s;
  int ds;
  double buf[12];
  buf[0] = t->xx();
  buf[1] = t->xy();
  buf[2] = t->xz();
  buf[3] = t->yx();
  buf[4] = t->yy();
  buf[5] = t->yz();
  buf[6] = t->zx();
  buf[7] = t->zy();
  buf[8] = t->zz();
  buf[9] = t->dx();
  buf[10] = t->dy();
  buf[11] = t->dz();
  s = sizeof(buf);
  while (1) {
    ds = write(fd, buf, s);
    if (ds == -1)
      return -1;
    s = s - ds;
    if (s <= 0)
      return 0;
  }
}

/**
 * Read transformation data from file.
 * @param[in]  fd File descriptor
 * @param[out] t  Transformation.
 * @return 0  Successful.
 * @return -1 Error.
 */
static int readTransform(int fd, HepGeom::Transform3D* t)
{
  int s;
  int ds;
  double buf[12];
  CLHEP::HepRotation r;
  CLHEP::Hep3Vector v;
  s = sizeof(buf);
  while (1) {
    ds = read(fd, buf, s);
    if (ds == -1)
      return -1;
    s = s - ds;
    if (s <= 0)
      break;
  }
  r.set(CLHEP::HepRep3x3(buf[0], buf[1], buf[2], buf[3], buf[4],
                         buf[5], buf[6], buf[7], buf[8]));
  v.set(buf[9], buf[10], buf[11]);
  *t = HepGeom::Translate3D(v) * HepGeom::Rotate3D(r);
  return 0;
}

int EKLM::writeTransforms(const char* file, struct EKLM::TransformData* dat)
{
  int i1;
  int i2;
  int i3;
  int i4;
  int i5;
  int fd;
  fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1)
    return -1;
  for (i1 = 0; i1 < 2; i1++) {
    if (writeTransform(fd, &(dat->endcap[i1])) != 0)
      return -1;
    for (i2 = 0; i2 < 14; i2++) {
      if (writeTransform(fd, &(dat->layer[i1][i2])) != 0)
        return -1;
      for (i3 = 0; i3 < 4; i3++) {
        if (writeTransform(fd, &(dat->sector[i1][i2][i3])) != 0)
          return -1;
        for (i4 = 0; i4 < 2; i4++) {
          if (writeTransform(fd, &(dat->plane[i1][i2][i3][i4])) != 0)
            return -1;
          for (i5 = 0; i5 < 75; i5++) {
            if (writeTransform(fd, &(dat->strip[i1][i2][i3][i4][i5])) != 0)
              return -1;
          }
        }
      }
    }
  }
  return 0;
}

int EKLM::readTransforms(const char* file, struct EKLM::TransformData* dat)
{
  int i1;
  int i2;
  int i3;
  int i4;
  int i5;
  int fd;
  fd = open(file, O_RDONLY);
  if (fd == -1)
    return -1;
  for (i1 = 0; i1 < 2; i1++) {
    if (readTransform(fd, &(dat->endcap[i1])) != 0)
      return -1;
    for (i2 = 0; i2 < 14; i2++) {
      if (readTransform(fd, &(dat->layer[i1][i2])) != 0)
        return -1;
      for (i3 = 0; i3 < 4; i3++) {
        if (readTransform(fd, &(dat->sector[i1][i2][i3])) != 0)
          return -1;
        for (i4 = 0; i4 < 2; i4++) {
          if (readTransform(fd, &(dat->plane[i1][i2][i3][i4])) != 0)
            return -1;
          for (i5 = 0; i5 < 75; i5++) {
            if (readTransform(fd, &(dat->strip[i1][i2][i3][i4][i5])) != 0)
              return -1;
          }
        }
      }
    }
  }
  return 0;
}

void EKLM::fillTransforms(struct TransformData* dat)
{
  int i1;
  int i2;
  int i3;
  int i4;
  int i5;
  GeoEKLMBelleII g(false);
  for (i1 = 0; i1 < 2; i1++) {
    g.getEndcapTransform(&(dat->endcap[i1]), i1);
    for (i2 = 0; i2 < 14; i2++) {
      g.getLayerTransform(&(dat->layer[i1][i2]), i2);
      for (i3 = 0; i3 < 4; i3++) {
        g.getSectorTransform(&(dat->sector[i1][i2][i3]), i3);
        for (i4 = 0; i4 < 2; i4++) {
          g.getPlaneTransform(&(dat->plane[i1][i2][i3][i4]), i4);
          for (i5 = 0; i5 < 75; i5++) {
            g.getStripTransform(&(dat->strip[i1][i2][i3][i4][i5]), i5);
          }
        }
      }
    }
  }
}

