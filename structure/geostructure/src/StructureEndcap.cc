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

#include <structure/geostructure/StructureEndcap.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;

StructureEndcap::StructureEndcap()
{

}

StructureEndcap::~StructureEndcap()
{

}

void StructureEndcap::clear()
{

}

void StructureEndcap::read()
{

  // Get Gearbox parameters
  GearDir strContent = Gearbox::Instance().getContent("Structure");

  //Endcap Vessel
  m_phi = strContent.getParamAngle("EndcapKLM/Phi");
  m_dphi = strContent.getParamAngle("EndcapKLM/Dphi");
  m_matname = strContent.getParamString("EndcapKLM/Material");
  m_nsides = int(strContent.getParamLength("EndcapKLM/Nsides"));

  // Get sense layers parameters
  m_nBoundary = strContent.getNumberNodes("EndcapKLM/ZBoundary");

  // Loop over all sense layers
  for (int izBoundary  = 0; izBoundary < m_nBoundary; izBoundary++) {
    int BoundaryId     = atoi((strContent.getParamString((format("EndcapKLM/ZBoundary[%1%]/@id") % (izBoundary + 1)).str())).c_str());

    m_z[izBoundary]    = strContent.getParamLength((format("EndcapKLM/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str());
    m_rmin[izBoundary] = strContent.getParamLength((format("EndcapKLM/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str());
    m_rmax[izBoundary] = strContent.getParamLength((format("EndcapKLM/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str());
  }

  //Endcap tube shape which should be subtracted from octagonal Endcap Vessel
  m_matnamesub = strContent.getParamString("EndcapKLMsub/Material");
  m_zsub    = strContent.getParamLength("EndcapKLMsub/Length");
  m_rminsub = strContent.getParamLength("EndcapKLMsub/InnerRadius");
  m_rmaxsub = strContent.getParamLength("EndcapKLMsub/OuterRadius");

  //radius parameter setting for layer sector
  m_rmin_layer = m_rmaxsub;
  m_rmax_layer = m_rmax[0];

  //Endcap KLM slot
  m_matname4slot = strContent.getParamString("EndcapKLMSlot/Material");
  m_matname4module = strContent.getParamString("EndcapKLMSlot/MaterialModule");
  m_thick_eiron  = strContent.getParamLength("EndcapKLMSlot/THICK_EIRON");
  m_thick_eiron_meas = strContent.getParamLength("EndcapKLMSlot/THICK_EIRON_MEAS");
  m_thick_eslot  = strContent.getParamLength("EndcapKLMSlot/THICK_ESLOT");
  m_thick_eslot_meas = strContent.getParamLength("EndcapKLMSlot/THICK_ESLOT_MEAS");

  m_thick_emod  = strContent.getParamLength("EndcapKLMSlot/THICK_EMOD");
  m_rmin_emod   = strContent.getParamLength("EndcapKLMSlot/RMIN_EMOD");
  m_rmax_emod   = strContent.getParamLength("EndcapKLMSlot/RMAX_EMOD");
  m_rmin_eslot  = strContent.getParamLength("EndcapKLMSlot/RMIN_ESLOT");
  m_rmax_eslot  = strContent.getParamLength("EndcapKLMSlot/RMAX_ESLOT");

  m_rshift_eslot = strContent.getParamLength("EndcapKLMSlot/RSHIFT_ESLOT");
  m_rmax_glass  = strContent.getParamLength("EndcapKLMSlot/RMAX_GLASS");
}


const TVector3 StructureEndcap::endcapModulePos(int sec, int mod) const
{
  return(TVector3(m_rshift_eslot *((sec == 0 || sec == 3) ? 1 : -1),
                  m_rshift_eslot *((sec == 0 || sec == 1) ? 1 : -1),
                  -(362.1 - 230.0) / 2. +
                  (m_thick_eiron + m_thick_eslot) *(mod + 1) - m_thick_eslot / 2.));
}
