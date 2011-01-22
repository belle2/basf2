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

#include <structure/geostructure/StructurePoletip.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;

StructurePoletip::StructurePoletip()
{

}

StructurePoletip::~StructurePoletip()
{

}

void StructurePoletip::clear()
{

}

void StructurePoletip::read()
{
  // Get Gearbox parameters
  GearDir strContent = Gearbox::Instance().getContent("Structure");

  m_phi = strContent.getParamAngle("Poletip/Phi");
  m_dphi = strContent.getParamAngle("Poletip/Dphi");
  m_matname = strContent.getParamString("Poletip/Material");

  m_nBoundary = strContent.getNumberNodes("Poletip/ZBoundary");

  for (int izBoundary  = 0; izBoundary < m_nBoundary; izBoundary++) {
    m_z[izBoundary]    = strContent.getParamLength((format("Poletip/ZBoundary[%1%]/Zposition") % (izBoundary + 1)).str());
    m_rmin[izBoundary] = strContent.getParamLength((format("Poletip/ZBoundary[%1%]/InnerRadius") % (izBoundary + 1)).str());
    m_rmax[izBoundary] = strContent.getParamLength((format("Poletip/ZBoundary[%1%]/OuterRadius") % (izBoundary + 1)).str());
  }
}
