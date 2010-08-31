/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/geodetector/CreatorManager.h>
#include <geometry/geodetector/CreatorBase.h>
#include <framework/logging/Logger.h>

#include <G4LogicalVolume.hh>
#include <G4SDManager.hh>

#include <TGeoManager.h>

using namespace std;
using namespace Belle2;


CreatorBase::CreatorBase(const string& name) throw(GDetExcCreatorNameEmpty)
{
  if (name.empty()) throw GDetExcCreatorNameEmpty();
  m_name = name;
  m_hasGroupName = false;
  m_senDetNumber = 0;

  CreatorManager::Instance().registerCreator(this);
}


CreatorBase::~CreatorBase()
{

}


void CreatorBase::assignSensitiveVolumes(TG4RootDetectorConstruction* detConstruct)
{
  m_senDetNumber = 0;

  //Create the path to the subdetector
  gGeoManager->CdTop();
  string subDetPath(gGeoManager->GetPath() + string("/") + m_geoGroupName + string("_1"));
  INFO_S("Constructing " << m_geoGroupName << " sensitive volumes from TGeo path '" << subDetPath << "'...")

  //Check if there is the specified subdetector available
  if (!gGeoManager->CheckPath(subDetPath.c_str())) {
    ERROR("Could not find TGeo path to " << m_geoGroupName << " subdetector. Does the path " << subDetPath << " exist ?")
    return;
  }

  gGeoManager->cd(subDetPath.c_str());

  //Go recursively through the subdetector volumes and search for volumes having the prefix given by the creator.
  assignSensitiveVolumesRec(gGeoManager->GetCurrentVolume(), detConstruct);

  INFO_S("...constructed " << m_senDetNumber << " sensitive " << m_geoGroupName << " volumes.")
}


void CreatorBase::setDescription(const std::string& description)
{
  m_description = description;
}


TGeoVolumeAssembly* CreatorBase::addSubdetectorGroup(const std::string& groupName, TGeoMatrix* groupTrafo)
{
  if (m_hasGroupName) {
    ERROR("The Creator " << m_name << " has already a subdetector group name !")
    return 0;
  }

  if (groupName.empty()) {
    ERROR("Empty group name specified in creator " << m_name << " !")
    return 0;
  }

  TGeoVolume* topVolume = gGeoManager->GetTopVolume();
  TGeoVolumeAssembly* subDetGrp = new TGeoVolumeAssembly(groupName.c_str());
  topVolume->AddNode(subDetGrp, 1, groupTrafo);
  m_geoGroupName = groupName;
  return subDetGrp;
}


void CreatorBase::addSensitiveDetector(const std::string prefix, G4VSensitiveDetector* sensitiveDetector)
{
  //Sensitive detector class check
  if (sensitiveDetector == NULL) {
    ERROR("The sensitive detector class is NULL !")
    return;
  }

  //Check if prefix already exists
  map<string, G4VSensitiveDetector*>::iterator foundIter = m_sensitiveDetMap.find(prefix);

  if (foundIter != m_sensitiveDetMap.end()) {
    ERROR("The sensitive detector prefix " << prefix << " already exists in the Creator " << m_name << " !")
    return;
  }

  // Add sensitive detector to sensitive detector manager
  G4SDManager::GetSDMpointer()->AddNewDetector(sensitiveDetector);

  //Store the prefix and the sensitive detector class in a map
  m_sensitiveDetMap.insert(make_pair(prefix, sensitiveDetector));
}


//====================================================================
//                          Private methods
//====================================================================

void CreatorBase::assignSensitiveVolumesRec(TGeoVolume* volume, TG4RootDetectorConstruction* detConstruct)
{
  string currPath = gGeoManager->GetPath();

  //Check if current volume is a sensitive volume (search for a valid prefix)
  //If yes, connect the sensitive detector class to the sensitive volume.
  for (map<string, G4VSensitiveDetector*>::iterator mapIter = m_sensitiveDetMap.begin(); mapIter != m_sensitiveDetMap.end(); mapIter++) {
    string currPrefix = mapIter->first;
    string detName(volume->GetName());
    if (detName.substr(0, currPrefix.length()).compare(currPrefix) == 0) {
      //Connect the sensitive detector class to the sensitive volume.
      G4LogicalVolume* logicVol = detConstruct->GetG4Volume(volume);
      logicVol->SetSensitiveDetector(mapIter->second);

      m_senDetNumber++;
    }
  }

  //Go recursively through the daughters
  int nDaughter = volume->GetNdaughters();
  for (int iDaughter = 0; iDaughter < nDaughter; ++iDaughter) {
    gGeoManager->cd(currPath.c_str());
    gGeoManager->CdDown(iDaughter);
    assignSensitiveVolumesRec(volume->GetNode(iDaughter)->GetVolume(), detConstruct);
  }
}
