/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/geobuilder/GeoBuilder.h>

#include <framework/fwcore/ModuleManager.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearboxIOXML.h>
#include <framework/geodetector/GeoDetector.h>

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
  addParam("InputFileXML",  m_filenameXML, string("Belle2.xml"), "The filename of the XML input file which should be loaded.");
  addParam("BuildGeometry", m_buildGeometry, true, "Set to true to build the detector geometry.");
}


GeoBuilder::~GeoBuilder()
{

}


void GeoBuilder::initialize()
{
  GearboxIOXML* gearboxIOXML = new GearboxIOXML();
  bool result = gearboxIOXML->open(m_filenameXML);

  if (result) {
    Gearbox& gearbox = Gearbox::Instance();
    gearbox.connect(gearboxIOXML);

  } else {
    ERROR("Could not open XML file: " << m_filenameXML)
  }
}


void GeoBuilder::beginRun()
{
  if (m_buildGeometry) {
    GeoDetector::Instance().createDetector();
  }
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
