/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Takanori Hara, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <errno.h>

/* External headers. */
#include <boost/format.hpp>
#include <globals.hh>

/* Belle2 headers. */
#include <eklm/geometry/GeoESTRCreator.h>
#include <framework/gearbox/GearDir.h>

#include "CLHEP/Units/PhysicalConstants.h"

using namespace Belle2;

int EKLM::readESTRData(struct EKLM::ESTRGeometryParams* par)
{
  int i;
  GearDir d("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/ESTR");
  GearDir d1(d);
  GearDir d2(d);
  GearDir d3(d);
  d1.append("/EndcapKLM");
  d2.append("/EndcapKLMsub");
  d3.append("/EndcapKLMSlot");
  par->phi = d1.getAngle("Phi") * CLHEP::rad;
  par->dphi = d1.getAngle("Dphi") * CLHEP::rad;
  par->nsides = d1.getInt("Nsides");
  par->nboundary = d1.getNumberNodes("ZBoundary");
  par->z = (double*)malloc(par->nboundary * sizeof(double));
  par->rmin = (double*)malloc(par->nboundary * sizeof(double));
  par->rmax = (double*)malloc(par->nboundary * sizeof(double));
  if (par->z == NULL || par->rmin == NULL || par->rmax == NULL)
    return ENOMEM;
  for (i = 0; i < par->nboundary; i++) {
    GearDir d4(d1);
    d4.append((boost::format("/ZBoundary[%1%]") % (i + 1)).str());
    par->z[i] = d4.getLength("Zposition") * CLHEP::cm;
    par->rmin[i] = d4.getLength("InnerRadius") * CLHEP::cm;
    par->rmax[i] = d4.getLength("OuterRadius") * CLHEP::cm;
  }
  par->zsub = d2.getLength("Length") * CLHEP::cm;
  par->rminsub = d2.getLength("InnerRadius") * CLHEP::cm;
  par->rmaxsub = d2.getLength("OuterRadius") * CLHEP::cm;
  par->thick_eiron = d3.getLength("THICK_EIRON") * CLHEP::cm;
  par->thick_eiron_meas = d3.getLength("THICK_EIRON_MEAS") * CLHEP::cm;
  par->thick_eslot = d3.getLength("THICK_ESLOT") * CLHEP::cm;
  par->thick_eslot_meas = d3.getLength("THICK_ESLOT_MEAS") * CLHEP::cm;
  par->thick_emod = d3.getLength("THICK_EMOD") * CLHEP::cm;
  par->rmin_emod = d3.getLength("RMIN_EMOD") * CLHEP::cm;
  par->rmax_emod = d3.getLength("RMAX_EMOD") * CLHEP::cm;
  par->rmin_eslot = d3.getLength("RMIN_ESLOT") * CLHEP::cm;
  par->rmax_eslot = d3.getLength("RMAX_ESLOT") * CLHEP::cm;
  par->rshift_eslot = d3.getLength("RSHIFT_ESLOT") * CLHEP::cm;
  par->rmax_glass = d3.getLength("RMAX_GLASS") * CLHEP::cm;
  return 0;
}

