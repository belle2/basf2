/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <geometry/modules/ExportGeometryModule.h>

#include <framework/utilities/FileSystem.h>
#include <geometry/GeometryManager.h>
#include <TGeoManager.h>
#ifdef HAS_GDML
#include <G4GDMLParser.hh>
#endif

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
  setDescription("Saves the Belle II detector geometry to a root or GDML file. "
                 "GDML support is only available if Geant4 has been compiled with "
                 "GDML enabled. This requires the xerces XML parser headers to "
                 "be installed when compiling the externals");

  //Parameter definition
  addParam("Filename",  m_filenameROOT, "The filename of the output file.", string("Belle2.root"));
  addParam("asGDML", m_exportAsGDML, "If True export as Geant4 GDML format, "
           "otherwise export as ROOT file", m_exportAsGDML);
  addParam("addAddresses", m_gdmlAdresses, "If True the names in the GDML file "
           "will be unique by adding their pointer address. This makes checking "
           "for differences problematic but would allow to use the Geometry directly",
           m_gdmlAdresses);
}


ExportGeometryModule::~ExportGeometryModule()  = default;


void ExportGeometryModule::initialize()
{
  //Check parameters
  if (!FileSystem::fileDirExists(m_filenameROOT)) {
    B2ERROR("Parameter <Filename>: The path of the filename " << m_filenameROOT << " does not exist !");
  }
#ifndef HAS_GDML
  if (m_exportAsGDML) {
    B2ERROR("This Geant4 version does not have GDML support, please recompile the externals with the xerces headers installed");
  }
#endif
}


void ExportGeometryModule::beginRun()
{
  if (!geometry::GeometryManager::getInstance().getTopVolume()) {
    B2ERROR("No Geometry found, cannot export anything");
    return;
  }
  if (m_exportAsGDML) {
#ifdef HAS_GDML
    G4GDMLParser parser;
    if (FileSystem::fileExists(m_filenameROOT)) {
      B2ERROR("Cannot export Geometry to GDML: file '" << m_filenameROOT << "' already exists");
    } else {
      parser.Write(m_filenameROOT, geometry::GeometryManager::getInstance().getTopVolume(), m_gdmlAdresses);
    }
    return;
#else
    // we should never come here as we checked in initialize but just in case ...
    B2FATAL("This Geant4 version does not have GDML support, please recompile the externals with the xerces headers installed");
#endif
  }
  geometry::GeometryManager::getInstance().createTGeoRepresentation();
  gGeoManager->Export(m_filenameROOT.c_str());
}
