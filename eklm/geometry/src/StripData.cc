/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <string.h>

/* Belle2 headers. */
#include <eklm/geometry/StripData.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

double EKLM::getStripLength(int strip)
{
  static bool filled = false;
  static double stripLen[75];
  char str[32];
  int n;
  int i;
  /* Return if data is already available. */
  if (filled)
    return stripLen[strip - 1];
  /* Read data. */
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/Endcap/"
             "Layer/Sector/Plane/Strips");
  n = gd.getNumberNodes("Strip");
  if (n != 75)
    B2FATAL("Unexpected number of strips in EKLM geometry XML data!");
  for (i = 0; i < 75; i++) {
    GearDir gds(gd);
    snprintf(str, 32, "/Strip[%d]", i + 1);
    gds.append(str);
    stripLen[i] = gds.getLength("Length");
  }
  filled = true;
  return stripLen[strip - 1];
}

