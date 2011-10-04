/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/geoeklm/StructureEndcap.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

using namespace std;

namespace Belle2 {

  //  using namespace geometry;

  //  namespace eklm {

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
    //GearDir strContent = Gearbox::Instance().getContent("Structure");

    GearDir strContent = GearDir("Detector/DetectorComponent[@name=\"ESTR\"]/Content/");

    //Endcap Vessel
    m_phi = strContent.getAngle("EndcapKLM/Phi") / Unit::rad;
    m_dphi = strContent.getAngle("EndcapKLM/Dphi") / Unit::rad;
    m_matname = strContent.getString("EndcapKLM/Material");
    m_nsides = int(strContent.getLength("EndcapKLM/Nsides"));

    m_nBoundary = strContent.getNumberNodes("EndcapKLM/ZBoundary");

    for (int izBoundary  = 0; izBoundary < m_nBoundary; izBoundary++) {
      m_z[izBoundary]    = strContent.getLength((boost::format("EndcapKLM/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str()) / Unit::mm;
      m_rmin[izBoundary] = strContent.getLength((boost::format("EndcapKLM/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str()) / Unit::mm;
      m_rmax[izBoundary] = strContent.getLength((boost::format("EndcapKLM/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str()) / Unit::mm;
    }

    //Endcap tube shape which should be subtracted from octagonal Endcap Vessel
    m_matnamesub = strContent.getString("EndcapKLMsub/Material");
    m_zsub    = strContent.getLength("EndcapKLMsub/Length") / Unit::mm;
    m_rminsub = strContent.getLength("EndcapKLMsub/InnerRadius") / Unit::mm;
    m_rmaxsub = strContent.getLength("EndcapKLMsub/OuterRadius") / Unit::mm;

    //radius parameter setting for layer sector
    m_rmin_layer = m_rmaxsub;
    m_rmax_layer = m_rmax[0];

    //Endcap KLM slot
    m_matname4slot = strContent.getString("EndcapKLMSlot/Material");
    m_matname4module = strContent.getString("EndcapKLMSlot/MaterialModule");
    m_thick_eiron  = strContent.getLength("EndcapKLMSlot/THICK_EIRON") / Unit::mm;
    m_thick_eiron_meas = strContent.getLength("EndcapKLMSlot/THICK_EIRON_MEAS") / Unit::mm;
    m_thick_eslot  = strContent.getLength("EndcapKLMSlot/THICK_ESLOT") / Unit::mm;
    m_thick_eslot_meas = strContent.getLength("EndcapKLMSlot/THICK_ESLOT_MEAS") / Unit::mm;

    m_thick_emod  = strContent.getLength("EndcapKLMSlot/THICK_EMOD") / Unit::mm;
    m_rmin_emod   = strContent.getLength("EndcapKLMSlot/RMIN_EMOD") / Unit::mm;
    m_rmax_emod   = strContent.getLength("EndcapKLMSlot/RMAX_EMOD") / Unit ::mm;
    m_rmin_eslot  = strContent.getLength("EndcapKLMSlot/RMIN_ESLOT") / Unit::mm;
    m_rmax_eslot  = strContent.getLength("EndcapKLMSlot/RMAX_ESLOT") / Unit::mm;

    m_rshift_eslot = strContent.getLength("EndcapKLMSlot/RSHIFT_ESLOT") / Unit::mm;
    m_rmax_glass  = strContent.getLength("EndcapKLMSlot/RMAX_GLASS") / Unit::mm;
  }


  const TVector3 StructureEndcap::endcapModulePos(int sec, int mod) const
  {
    return(TVector3(m_rshift_eslot *((sec == 0 || sec == 3) ? 1 : -1),
                    m_rshift_eslot *((sec == 0 || sec == 1) ? 1 : -1),
                    -(362.1 - 230.0) / 2. +
                    (m_thick_eiron + m_thick_eslot) *(mod + 1) - m_thick_eslot / 2.));
  }

  //  }
}
