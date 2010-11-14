/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/geobuilder/GeoSaverModule.h>

#include <framework/core/ModuleUtils.h>
#include <geometry/geodetector/GeoDetector.h>

#include <boost/filesystem.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GeoSaverModule, "GeoSaver")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoSaverModule::GeoSaverModule() : Module()
{
  //Set module properties
  setDescription("Saves the Belle II detector geometry to a root file.");

  //Parameter definition
  addParam("Filename",  m_filenameROOT, string("Belle2.root"), "The filename of the ROOT output file.");
}


GeoSaverModule::~GeoSaverModule()
{

}


void GeoSaverModule::initialize()
{
  //Check parameters
  if (!ModuleUtils::filePathExists(m_filenameROOT)) {
    B2ERROR("Parameter <Filename>: The path of the filename " << m_filenameROOT << " does not exist !")
  }
}


void GeoSaverModule::event()
{
  GeoDetector::Instance().saveToRootFile(m_filenameROOT);
}
