/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/geodetector/GeoDetector.h>
#include <framework/geodetector/GDetExceptions.h>
#include <framework/logging/Logger.h>

#include <simulation/simkernel/B4VSubDetectorDriver.h>
#include <simulation/simkernel/B4DetectorConstruction.h>

#include "globals.hh"
#include "G4SDManager.hh"
#include "G4LogicalVolume.hh"

#include <TGeoVolume.h>

#include <list>

using namespace Belle2;
using namespace std;


B4VSubDetectorDriver::B4VSubDetectorDriver(const std::string driverName)
{
  //--------------------------------------------------
  // Register this driver to B4DetectorConstruction.
  //--------------------------------------------------
  m_driverName = driverName;
  B4DetectorConstruction::Instance()->registerGeometryDriver(this);
}


bool B4VSubDetectorDriver::isApplicable(const std::string& driverName) const
{
  return (m_driverName == driverName);
}


void B4VSubDetectorDriver::setSensitiveDetectorAuto(G4VSensitiveDetector* sensitiveDetector, TG4RootDetectorConstruction* dc)
{
  //-----------------------------------------------------------
  // Add sensitive detector to sensitive detector manager
  //-----------------------------------------------------------
  G4SDManager* senDetMg = G4SDManager::GetSDMpointer();
  senDetMg->AddNewDetector(sensitiveDetector);

  //-----------------------------------------------------------
  //    Get Geant4 volumes and define sensitive detector
  //-----------------------------------------------------------
  try {
    const list<TGeoVolume*>& volumeList = GeoDetector::Instance().getSensitiveVolumes(m_driverName);

    //Loop over the sensitive volumes and assign the sensitive detector to them
    list<TGeoVolume*>::const_iterator listIter;
    for (listIter = volumeList.begin(); listIter != volumeList.end(); listIter++) {
      G4LogicalVolume* logicVol = dc->GetG4Volume(*listIter);
      logicVol->SetSensitiveDetector(sensitiveDetector);
    }
  } catch (GDetExcCreatorNameEmpty) {
    ERROR("The sensitive detector construction class does not have a name !")
  } catch (GDetExcCreatorNotExists& exc) {
    string logMessage = " does not define any sensitive volumes ! ";
    logMessage += "Is the sensitive detector driver name the same as the geometry creator name ?";
    ERROR("The subdetector driver " << m_driverName << logMessage)
  }
}
