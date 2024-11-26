/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/beamabort/dbobjects/BeamabortGeo.h>

using namespace Belle2;

void BeamabortGeo::initialize(const GearDir& content)
{
  //------------------------------
  // Get BEAMABORT geometry parameters from the gearbox
  //------------------------------

  addParameter("stepSize", content.getWithUnit("stepSize"));
  addParameter("BeamBackgroundStudy", content.getInt("BeamBackgroundStudy"));
  addParameter("divisions", content.getInt("divisions"));
  addParameter("foo", content.getBool("foo"));
  addArray("bar", content.getArray("bar"));
  addParameter("WorkFunction", content.getDouble("WorkFunction"));
  addParameter("FanoFactor", content.getDouble("FanoFactor"));

  GearDir cActive(content, "Active/");
  addParameter("Active.phase", cActive.getInt("phase"));
  addArray("Active.z", cActive.getArray("z"));
  addArray("Active.r_dia", cActive.getArray("r_dia"));
  addArray("Active.ThetaZ", cActive.getArray("ThetaZ"));
  addArray("Active.Phi", cActive.getArray("Phi"));
  addArray("Active.deltaX", cActive.getArray("deltaX"));
  addArray("Active.addAngle", cActive.getArray("addAngle"));
}
