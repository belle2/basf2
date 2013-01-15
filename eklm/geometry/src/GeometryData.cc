/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/geometry/GeometryData.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

int EKLM::GeometryData::save(const char* file)
{
  int res;
  FILE* f;
  /* Create file. */
  f = fopen(file, "w");
  if (f == NULL)
    return -1;
  /* Fill transformation data. */
  EKLM::fillTransforms(&transf);
  /* Write and close file. */
  res = writeTransforms(f, &transf);
  if (res != 0)
    return res;
  fclose(f);
  return 0;
}

int EKLM::GeometryData::read(const char* file)
{
  int i;
  int n;
  int res;
  char str[32];
  FILE* f;
  f = fopen(file, "r");
  if (f == NULL)
    return -1;
  res = readTransforms(f, &transf);
  if (res != 0)
    return res;
  fclose(f);
  /* Fill strip data. */
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/Endcap/"
             "Layer/Sector/Plane/Strips");
  n = gd.getNumberNodes("Strip");
  if (n != 75)
    B2FATAL("Unexpected number of strips in EKLM geometry XML data!");
  for (i = 0; i < 75; i++) {
    GearDir gds(gd);
    snprintf(str, 32, "/Strip[%d]", i + 1);
    gds.append(str);
    m_stripLen[i] = gds.getLength("Length");
  }
  return 0;
}

double EKLM::GeometryData::getStripLength(int strip)
{
  return m_stripLen[strip - 1];
}

