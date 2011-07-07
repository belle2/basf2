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

#include <bklm/geobklm/BKLMGeometryPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;

BKLMGeometryPar* BKLMGeometryPar::m_BKLMGeometryParDB = 0;

BKLMGeometryPar* BKLMGeometryPar::Instance()
{
  if (!m_BKLMGeometryParDB) m_BKLMGeometryParDB = new BKLMGeometryPar();
  return m_BKLMGeometryParDB;
}

BKLMGeometryPar::BKLMGeometryPar()
{
  clear();
  read();
}

BKLMGeometryPar::~BKLMGeometryPar()
{
}

void BKLMGeometryPar::clear()
{
  m_rotation         = 0.0;
  m_offsetZ          = 0.0;
  m_phi              = 0.0;
  m_nsides           = 0;
  m_rsolenoid        = 0.0;
  m_rmax             = 0.0;
  m_length           = 0.0;
  m_nlayer           = 0;
  m_height_iron      = 0.0;
  m_height_iron_meas = 0.0;
  m_height_gap0      = 0.0;
  m_height_gap0_meas = 0.0;
  m_height_gap       = 0.0;
  m_height_gap_meas  = 0.0;
  m_height_layer     = 0.0;
  m_rmin_gap0        = 0.0;
  m_rmin_gap1        = 0.0;
  m_dsx_gap0         = 0.0;
  m_dsx_gap          = 0.0;
  m_length_gap       = 0.0;
  m_length_mod       = 0.0;
  m_length_mod_chimney = 0.0;
  m_height_cover     = 0.0;
  m_height_copper    = 0.0;
  m_height_foam      = 0.0;
  m_height_mylar     = 0.0;
  m_height_glass     = 0.0;
  m_height_gas       = 0.0;
  m_height_readout   = 0.0;
  m_height_mod       = 0.0;
  m_width_frame      = 0.0;
  m_width_spacer     = 0.0;
  m_length_chimney   = 0.0;
  m_height_chimney   = 0.0;
  m_width_chimney    = 0.0;
  m_cover_chimney    = 0.0;
  m_x_chimney        = 0.0;
  m_y_chimney        = 0.0;
  m_z_chimney        = 0.0;
  m_thickness_rib    = 0.0;
  m_width_cables     = 0.0;
  m_width_brace      = 0.0;
  m_width_brace_chimney = 0.0;
  m_width_support_plate  = 0.0;
  m_height_support_plate = 0.0;
}

void BKLMGeometryPar::read()
{
  // Get Gearbox parameters for BKLM
  GearDir content = Gearbox::Instance().getContent("BKLM");

  //Polygonal vessel
  m_rotation = content.getParamAngle("Rotation");
  m_offsetZ = content.getParamLength("OffsetZ");
  m_phi = content.getParamLength("Phi");
  m_nsides = int(content.getParamLength("Nsides"));
  m_rsolenoid = content.getParamLength("SolenoidRadius");
  m_rmax = content.getParamLength("OuterRadius");
  m_length = content.getParamLength("Length");
  m_nlayer = int(content.getParamLength("Nlayer"));

  //Each layer

  m_height_iron      = content.getParamLength("Layer/HeightIron");
  m_height_iron_meas = content.getParamLength("Layer/HeightIronMeas");


  m_height_gap0      = content.getParamLength("Layer/HeightGap0");
  m_height_gap0_meas = m_height_gap0 + (m_height_iron - m_height_iron_meas) / 2.;

  m_height_gap       = content.getParamLength("Layer/HeightGap");
  m_height_gap_meas  = m_height_gap + (m_height_iron - m_height_iron_meas);

  m_height_layer0    = m_height_iron + m_height_gap0;
  m_height_layer     = m_height_iron + m_height_gap;

  m_rmin_gap0        = content.getParamLength("Layer/RminGap0");
  m_rmin_gap1        = m_rmin_gap0 + m_height_layer0;

  m_dsx_gap0         = content.getParamLength("Layer/DSXGap0");
  m_dsx_gap          = content.getParamLength("Layer/DSXGap");

  m_length_gap       = content.getParamLength("Layer/Length");

  m_length_mod       = content.getParamLength("Module/Length");
  m_length_mod_chimney = content.getParamLength("Module/LengthChimney");
  m_height_cover = content.getParamLength("Module/HeightCover");
  m_height_copper = content.getParamLength("Module/HeightCopper");
  m_height_foam = content.getParamLength("Module/HeightFoam");
  m_height_mylar = content.getParamLength("Module/HeightMylar");
  m_height_glass = content.getParamLength("Module/HeightGlass");
  m_height_gas = content.getParamLength("Module/HeightGas");
  m_height_readout = m_height_foam + (m_height_copper + m_height_mylar) * 2.0;
  m_height_mod = (m_height_cover + m_height_readout + m_height_gas + m_height_glass * 2.0) * 2.0;
  m_width_frame = content.getParamLength("Module/WidthFrame");
  m_width_spacer = content.getParamLength("Module/WidthSpacer");

  m_length_chimney   = content.getParamLength("Chimney/Length");
  m_height_chimney   = m_rmax - m_rsolenoid;
  m_width_chimney    = content.getParamLength("Chimney/Width");
  m_cover_chimney    = content.getParamLength("Chimney/Cover");
  m_z_chimney        = m_length_gap - m_length_chimney / 2.0;
  m_chimney_housing_rmin = content.getParamLength("Chimney/HousingRmin");
  m_chimney_housing_rmax = content.getParamLength("Chimney/HousingRmax");
  m_chimney_shield_rmin = content.getParamLength("Chimney/ShieldRmin");
  m_chimney_shield_rmax = content.getParamLength("Chimney/ShieldRmax");
  m_chimney_pipe_rmin = content.getParamLength("Chimney/PipeRmin");
  m_chimney_pipe_rmax = content.getParamLength("Chimney/PipeRmax");
  m_thickness_rib = content.getParamLength("ThicknessRib");
  m_width_cables = content.getParamLength("WidthCables");
  m_width_brace = content.getParamLength("WidthBrace");
  m_width_brace_chimney = content.getParamLength("WidthBraceChimney");
  m_width_support_plate = content.getParamLength("WidthSupportPlate");
  m_height_support_plate = content.getParamLength("HeightSupportPlate");

}


const double BKLMGeometryPar::layerRmin(int lyr) const
{
  if (lyr == 0) {
    return (m_rmin_gap0);
  }
  return (m_rmin_gap1 - (m_height_iron - m_height_iron_meas) / 2.0 + m_height_layer *(lyr - 1));
}

const double BKLMGeometryPar::layerRmax(int lyr) const
{
  if (lyr == m_nlayer - 1) {
    return (m_rmax);
  }
  return (m_rmin_gap1 - (m_height_iron - m_height_iron_meas) / 2.0 + m_height_layer * lyr);
}

const TVector3 BKLMGeometryPar::gapSize(int lyr, bool flag) const
{
  double r, ds, dx;
  if (lyr == 0) {
    r  = m_rmin_gap0 + m_height_gap0;
    ds = m_dsx_gap0;
    dx = m_height_gap0_meas / 2.0;
  } else {
    r  = m_rmin_gap1 + m_height_gap + m_height_layer * (lyr - 1);
    ds = m_dsx_gap;
    dx = m_height_gap_meas / 2.0;
  }
  double dz = (flag ? m_length_gap - m_length_chimney : m_length_gap) / 2.0;
  return(TVector3(dx, r * tan(M_PI / m_nsides) - ds, dz));
}

const TVector3 BKLMGeometryPar::moduleSize(int lyr, bool flag) const
{
  TVector3 size = gapSize(lyr, flag);
  size.SetX(m_height_mod / 2.0);
  if (flag) {
    size.SetZ(m_length_mod_chimney / 2.0);
  } else {
    size.SetZ(m_length_mod / 2.0);
  }
  return (size);
}

const TVector3 BKLMGeometryPar::readoutSize(int lyr, bool flag) const
{
  TVector3 size = moduleSize(lyr, flag);
  size.SetX(m_height_readout + m_height_glass * 2.0 + m_height_gas);
  size.SetY(size.Y() - m_width_frame);
  size.SetZ(size.Z() - m_width_frame);
  return (size);
}

const TVector3 BKLMGeometryPar::electrodeSize(int lyr, bool flag) const
{
  TVector3 size = readoutSize(lyr, flag);
  size.SetX(m_height_glass * 2.0 + m_height_gas);
  return (size);
}

const TVector3 BKLMGeometryPar::gasSize(int lyr, bool flag) const
{
  TVector3 size = electrodeSize(lyr, flag);
  size.SetX(m_height_gas * 0.5);
  size.SetY(size.Y() - m_width_spacer);
  size.SetZ(size.Z() - m_width_spacer);
  return (size);
}

const double BKLMGeometryPar::gapR(int lyr) const
{
  if (lyr == 0) {
    return(m_rmin_gap0 + m_height_gap0_meas / 2.0);
  }
  return(m_rmin_gap1 + m_height_gap_meas / 2.0 + m_height_layer *(lyr - 1));
}

const double BKLMGeometryPar::moduleR(int lyr) const
{
  if (lyr == 0) {
    return(m_rmin_gap0 + m_height_gap0_meas / 2.0);
  }
  return(m_rmin_gap1 + m_height_gap / 2.0 + m_height_layer *(lyr - 1));
}

const TVector3 BKLMGeometryPar::size_chimney(int lyr) const
{
  return(TVector3((layerRmax(lyr) - layerRmin(lyr)) / 2.0, m_width_chimney / 2.0, m_length_chimney / 2.0));
}

const TVector3 BKLMGeometryPar::pos_chimney(int lyr) const
{
  return(TVector3((layerRmax(lyr) + layerRmin(lyr)) / 2.0, m_y_chimney, m_z_chimney));
}

const TVector3 BKLMGeometryPar::size_support_plate(bool flag) const
{
  TVector3 size;
  size.SetX(m_height_support_plate / 2.0);
  size.SetY(m_width_support_plate / 2.0);
  if (flag) {
    size.SetZ((m_length_mod_chimney + 1.6) / 2.0);
  } else {
    size.SetZ(m_length_gap / 2.0);
  }
  return(size);
}

