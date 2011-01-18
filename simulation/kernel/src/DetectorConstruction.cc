/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/GeoDetector.h>
#include <geometry/geodetector/CreatorManager.h>

#include <framework/gearbox/MaterialProperty.h>
#include <framework/gearbox/MaterialPropertyList.h>

#include <simulation/kernel/DetectorConstruction.h>

#include <framework/logging/Logger.h>

#include <G4Material.hh>
#include <G4MaterialPropertiesTable.hh>

#include <TCollection.h>
#include <TList.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

DetectorConstruction::DetectorConstruction()
{

}


DetectorConstruction::~DetectorConstruction()
{

}


void DetectorConstruction::Initialize(TG4RootDetectorConstruction *dc)
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

    } catch (CreatorManager::GeometryCreatorNameEmptyError& exc) {
      B2ERROR(exc.what());
    } catch (CreatorManager::GeometryCreatorNotExistsError& exc) {
      B2ERROR(exc.what());
    }
  }

  //--------------------------------------------------------------------
  // Get the list of created ROOT materials and read the Cerenkov
  // properties. If they exist, create a G4MaterialPropertiesTable,
  // fill it with the data from the Cerenkov properties and append
  // it to the corresponding G4Material.
  //--------------------------------------------------------------------
  TIterator* matIter = gGeoManager->GetListOfMaterials()->MakeIterator();
  TGeoMaterial* currMaterial;
  while ((currMaterial = dynamic_cast<TGeoMaterial*>(matIter->Next()))) {

    //Check if the material has Cerenkov properties attached
    if (currMaterial->GetCerenkovProperties() != NULL) {
      MaterialPropertyList* currList = dynamic_cast<MaterialPropertyList*>(currMaterial->GetCerenkovProperties());

      //Check if the property list has at least one property
      if (currList->GetSize() > 0) {
        G4MaterialPropertiesTable* g4PropTable = new G4MaterialPropertiesTable();

        //Loop over the list of properties
        MaterialProperty* currProperty;
        TIterator* propIter = currList->MakeIterator();
        while ((currProperty = dynamic_cast<MaterialProperty*>(propIter->Next()))) {
          double energies[currProperty->getNumberValues()];
          double values[currProperty->getNumberValues()];

          //Fill the arrays of energies and values and use them to create a new property entry in the Geant4 table
          currProperty->fillArrays(energies, values);
          g4PropTable->AddProperty(currProperty->getName().c_str(), energies, values, currProperty->getNumberValues());
        }
        delete propIter;

        G4Material* g4Material = dc->GetG4Material(currMaterial);
        g4Material->SetMaterialPropertiesTable(g4PropTable);
      } else {
        B2ERROR("The material property list of the material '" << currMaterial->GetName() << "' is empty !")
      }
    }
  }
  delete matIter;
}
