/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <top/modules/TOPGeometryParInitializer/TOPGeometryParInitializerModule.h>
#include <top/geometry/TOPGeometryPar.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>


using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPGeometryParInitializer)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPGeometryParInitializerModule::TOPGeometryParInitializerModule() : Module()

  {
    // set module description
    setDescription("Class for initializing TOPGeometryPar. "
                   "This class is by default initialized when Geant geometry is created. "
                   "Useful if Geant geometry is not needed. Be carefull when using!");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("useDB", m_useDB,
             "If true load the Geometry from the database instead of the gearbox", true);
  }

  void TOPGeometryParInitializerModule::initialize()
  {
    auto* gp = TOPGeometryPar::Instance();
    if (gp->isValid()) {
      B2WARNING("TOPGeometryPar has already been initialized");
      return;
    }

    if (m_useDB) {
      gp->Initialize();
    } else {
      bool found = false;
      GearDir detectorDir("/Detector");
      for (const GearDir& component : detectorDir.getNodes("DetectorComponent")) {
        if (component.getString("@name") == "TOP") {
          gp->Initialize(GearDir(component, "Content"));
          found = true;
          break;
        }
      }
      if (not found) B2ERROR("Component TOP does not exist or is empty");
    }

  }


} // end Belle2 namespace

