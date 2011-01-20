/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <structure/geostructure/StructureBarrel.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;

StructureBarrel::StructureBarrel()
{
}

StructureBarrel::~StructureBarrel()
{
}

void StructureBarrel::clear()
{
}

void StructureBarrel::read()
{

  // Get Gearbox parameters
  GearDir strContent = Gearbox::Instance().getContent("Structure");

  //Barrel Vessel
  m_phi = strContent.getParamAngle("BarrelKLM/Phi");
  m_dphi = strContent.getParamAngle("BarrelKLM/Dphi");
  m_matname = strContent.getParamString("BarrelKLM/Material");
  m_nsides = int(strContent.getParamLength("BarrelKLM/Nsides"));

  // Get sense layers parameters
  m_nBoundary = strContent.getNumberNodes("BarrelKLM/ZBoundary");

  // Loop over all sense layers
  for (int izBoundary  = 0; izBoundary < m_nBoundary; izBoundary++) {
    int BoundaryId     = atoi((strContent.getParamString((format("BarrelKLM/ZBoundary[%1%]/@id") % (izBoundary + 1)).str())).c_str());

    m_z[izBoundary]    = strContent.getParamLength((format("BarrelKLM/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str());
    m_rmin[izBoundary] = strContent.getParamLength((format("BarrelKLM/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str());
    m_rmax[izBoundary] = strContent.getParamLength((format("BarrelKLM/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str());
  }


  //Barrel KLM slot
  m_matname4slot = strContent.getParamString("BarrelKLMSlot/Material");
  m_matname4module = strContent.getParamString("BarrelKLMSlot/MaterialModule");
  m_nsector = m_nsides;
  m_nmodule = int(strContent.getParamLength("BarrelKLMSlot/Nmodule"));
  m_rmin_bslot0  = strContent.getParamLength("BarrelKLMSlot/RMIN_BSLOT0");
  m_thick_bslot0 = strContent.getParamLength("BarrelKLMSlot/THICK_BSLOT0");
  m_dsx_bslot0   = strContent.getParamLength("BarrelKLMSlot/DSX_BSLOT0");
  m_length_bslot = strContent.getParamLength("BarrelKLMSlot/LEN_BSLOT");

  m_thick_biron  = strContent.getParamLength("BarrelKLMSlot/THICK_BIRON");
  m_thick_biron_meas = strContent.getParamLength("BarrelKLMSlot/THICK_BIRON_MEAS");

  m_thick_bslot0_meas = m_thick_bslot0 + (m_thick_biron - m_thick_biron_meas) / 2.;

  m_thick_bslot = strContent.getParamLength("BarrelKLMSlot/THICK_BSLOT");
  m_dsx_bslot   = strContent.getParamLength("BarrelKLMSlot/DSX_BSLOT");

  m_rmin_bslot1 = m_rmin_bslot0 + m_thick_bslot0 + m_thick_biron;
  m_thick_bslot_meas = m_thick_bslot + (m_thick_biron - m_thick_biron_meas);

  B2INFO("Structure : RMIN_BSLOT0  =" << m_rmin_bslot0);
  B2INFO("Structure : THICK_BSLOT0 =" << m_thick_bslot0);
  B2INFO("Structure : DSX_BSLOT0   =" << m_dsx_bslot0);

  m_thick_bmod = strContent.getParamLength("BarrelKLMSlot/THICK_BMOD");
  m_length_bmod = m_length_bslot;

  m_length_chole = strContent.getParamLength("BarrelKLMSlot/LEN_CHOLE");
  m_cov_chole    = strContent.getParamLength("BarrelKLMSlot/COV_CHOLE");
  m_width_chole  = strContent.getParamLength("BarrelKLMSlot/WID_CHOLE");
}


const TVector3 StructureBarrel::barrelSlotSize(int mod) const
{
  double r, ds, dx;
  if (mod == 0) {
    r  = m_rmin_bslot0 + m_thick_bslot0;
    ds = m_dsx_bslot0;
    dx = m_thick_bslot0_meas / 2.0;
  } else {
    r  = m_rmin_bslot1 + m_thick_bslot + (m_thick_biron + m_thick_bslot) * (mod - 1);
    ds = m_dsx_bslot;
    dx = m_thick_bslot_meas / 2.0;
  }

  /*
  B2INFO("XStructure : r  =" << r);
  B2INFO("XStructure : ds =" << ds);
  B2INFO("XStructure : dx =" << dx);
  */
  return(TVector3(dx,                         //dx
                  r * tan(M_PI / 8) - ds,   //dy
                  m_length_bslot / 2.0));        //dz
}

const TVector3 StructureBarrel::barrelModuleSize(int mod) const
{
  TVector3 size = barrelSlotSize(mod);
  size.SetX(m_thick_bmod / 2.0);
  return (size);
}


const double StructureBarrel::barrelSlotR(int mod) const
{
  if (mod == 0) {
    return(m_rmin_bslot0 + m_thick_bslot0_meas / 2.0);
  }
  return(barrelModuleR(mod));
}

const double StructureBarrel::barrelModuleR(int mod) const
{
  if (mod == 0) {
    return(m_rmin_bslot0 + m_thick_bslot0 / 2.0);
  }
  return(m_rmin_bslot1 + m_thick_bslot / 2.0 + (m_thick_biron + m_thick_bslot)*(mod - 1));
}
