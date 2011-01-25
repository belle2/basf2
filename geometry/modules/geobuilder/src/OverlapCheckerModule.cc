/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/geobuilder/OverlapCheckerModule.h>

#include <framework/core/ModuleUtils.h>
#include <framework/logging/Logger.h>
#include <geometry/geodetector/GeoDetector.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(OverlapChecker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

OverlapCheckerModule::OverlapCheckerModule() : Module()
{
  //Set module properties
  setDescription("Checks the Belle II detector geometry for overlaps.");

  //Parameter definition
  addParam("Tolerance", m_tolerance, "The tolerance allowed for the overlapping distance.", 0.01);
  addParam("PrintOverlaps", m_printOverlaps, "Set to true to print the found overlaps", true);
}


OverlapCheckerModule::~OverlapCheckerModule()
{

}


void OverlapCheckerModule::initialize()
{

}


void OverlapCheckerModule::event()
{
  if (gGeoManager == NULL) {
    B2ERROR("No geometry was found in memory !")
    return;
  }

  gGeoManager->CheckOverlaps(m_tolerance);

  if (m_printOverlaps) gGeoManager->PrintOverlaps();
}
