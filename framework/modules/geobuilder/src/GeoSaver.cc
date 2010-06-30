/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/geobuilder/GeoSaver.h>

#include <framework/fwcore/ModuleManager.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearboxIOXML.h>
#include <framework/geodetector/GeoDetector.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GeoSaver)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoSaver::GeoSaver() : Module("GeoSaver")
{
  //Set module properties
  setDescription("Saves the Belle II detector geometry to a root file.");

  //Parameter definition
  addParam("Filename",  m_filenameROOT, string("Belle2.root"), "The filename of the ROOT output file.");
}


GeoSaver::~GeoSaver()
{

}


void GeoSaver::initialize()
{

}


void GeoSaver::beginRun()
{

}


void GeoSaver::event()
{

}


void GeoSaver::endRun()
{

}


void GeoSaver::terminate()
{

}
