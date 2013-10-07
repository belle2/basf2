/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/ExportGeometryModule.h>

#include <framework/utilities/FileSystem.h>
#include <geometry/GeometryManager.h>
#include <TGeoManager.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ExportGeometry)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ExportGeometryModule::ExportGeometryModule() : Module()
{
  //Set module properties
  setDescription("Saves the Belle II detector geometry to a root file.");

  //Parameter definition
  addParam("Filename",  m_filenameROOT, "The filename of the ROOT output file.", string("Belle2.root"));
}


ExportGeometryModule::~ExportGeometryModule()
{

}


void ExportGeometryModule::initialize()
{
  //Check parameters
  if (!FileSystem::fileDirExists(m_filenameROOT)) {
    B2ERROR("Parameter <Filename>: The path of the filename " << m_filenameROOT << " does not exist !")
  }
}


void ExportGeometryModule::beginRun()
{
  geometry::GeometryManager::getInstance().createTGeoRepresentation();
  gGeoManager->Export(m_filenameROOT.c_str());
}
