/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/geobuilder/GeoBuilderModule.h>

#include <geometry/geodetector/GeoDetector.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GeoBuilderModule, "GeoBuilder")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoBuilderModule::GeoBuilderModule() : Module()
{
  //Set module properties
  setDescription("Loads the Belle II detector parameters and creates the detector geometry.");

  //Parameter definition
}


GeoBuilderModule::~GeoBuilderModule()
{

}


void GeoBuilderModule::initialize()
{
  GeoDetector::Instance().createDetector();
}
