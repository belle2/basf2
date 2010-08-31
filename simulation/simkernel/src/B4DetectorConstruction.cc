/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/GeoDetector.h>
#include <geometry/geodetector/CreatorManager.h>

#include <simulation/simkernel/B4DetectorConstruction.h>
#include <simulation/simkernel/B4MagneticField.h>

#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;


B4DetectorConstruction::B4DetectorConstruction() : m_magneticField(NULL)
{
  m_magneticField = new B4MagneticField();
}


B4DetectorConstruction::~B4DetectorConstruction()
{
  if (m_magneticField) delete m_magneticField;
}


void B4DetectorConstruction::Initialize(TG4RootDetectorConstruction *dc)
{
  //--------------------------------------------------------------------
  // Get the list of called Creators, connect the sensitive volumes
  // to the sensitive detector classes and call the initializeGeant4
  // method of the Creator.
  //--------------------------------------------------------------------
  CreatorManager& creatorMgr = CreatorManager::Instance();
  list<string> creatorList = GeoDetector::Instance().getCalledCreators();

  //Loop over all creators which were called during the creation of the ROOT geometry
  for (list<string>::iterator listIter = creatorList.begin(); listIter != creatorList.end(); listIter++) {
    try {
      //Get the current creator
      CreatorBase& currCreator = creatorMgr.getCreator(*listIter);

      //Connect the sensitive volumes to the sensitive detector
      currCreator.assignSensitiveVolumes(dc);

      //Do work on the native geant4 volumes
      currCreator.initializeGeant4(dc);

    } catch (GDetExcCreatorNameEmpty) {
      ERROR("The name of the Creator is empty !");
    } catch (GDetExcCreatorNotExists& e) {
      ERROR("A Creator with the name " << e.getName() << " does not exist !");
    }
  }
}
