/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/geobuilder/GeoBuilder.h>
#include <framework/core/ModuleManager.h>
#include <geometry/geodetector/GeoDetector.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GeoBuilder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoBuilder::GeoBuilder() : Module("GeoBuilder")
{
  //Set module properties
  setDescription("Loads the Belle II detector parameters and creates the detector geometry.");

  //Parameter definition
}


GeoBuilder::~GeoBuilder()
{

}


void GeoBuilder::initialize()
{
  GeoDetector::Instance().createDetector();
}


void GeoBuilder::beginRun()
{

}


void GeoBuilder::event()
{

}


void GeoBuilder::endRun()
{

}


void GeoBuilder::terminate()
{

}
