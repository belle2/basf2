/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gearbox/Gearbox.h>
#include <gearbox/GearDir.h>
#include <geodetector/GeoDetector.h>
#include <geodetector/CreatorManager.h>
#include <geodetector/GDetExceptions.h>
#include <gearbox/GbxExceptions.h>
#include <logging/Logger.h>

#include <TGeoManager.h>
#include <TGeoMaterial.h>
#include <TGeoMedium.h>

#include <boost/format.hpp>

using namespace std;
using namespace Belle2;

GeoDetector* GeoDetector::m_instance = NULL;


GeoDetector& GeoDetector::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) {
    m_instance = new GeoDetector();
  }
  return *m_instance;
}


void GeoDetector::createDetector()
{
  GearDir detectorDir("/Detector/");

  try {
    string detectorName = detectorDir.getParamString("Name");
    string detectorDesc = detectorDir.getParamString("Description");

    INFO_S("----------------------------------------------")
    INFO_S("Creating geometry for detector: " << detectorName)
    INFO_S("----------------------------------------------")

    m_calledCreators.clear();
    clearSensitiveDetDB();

    //Create a new TGeoManager. This might look like a memory leak, but ROOT takes care of
    //deleting the old TGeoManager when the new one is created.
    new TGeoManager(detectorName.c_str(), detectorDesc.c_str());
    gGeoManager->Clear();

    CreatorManager& creatorManager = CreatorManager::Instance();

    //Create ROOT objects by calling the associated creators
    //Loop over all supported sections
    for (list<string>::iterator section = m_supportedSections.begin(); section != m_supportedSections.end(); section++) {
      GearDir sectionDir(detectorDir, (*section) + "/");

      //Loop over all ParamSets
      int nSet = sectionDir.getNumberNodes("ParamSet");
      for (int iSet = 1; iSet <= nSet; ++iSet) {
        //Check if the ParamSet has a Creator associated to it
        //if yes, get a reference to the Creator and call its create method.
        boost::format queryString("ParamSet[%1%]/");
        queryString % iSet;
        GearDir sectionDirLocal(sectionDir, queryString.str());

        string creatorName = sectionDirLocal.getParamString("Creator");
        string subDetName  = sectionDirLocal.getParamString("Name");

        if (!creatorName.empty()) {
          INFO_S("Calling creator for: " << subDetName << " (" << creatorName << ")")

          CreatorBase& currCreator = creatorManager.getCreator(creatorName);
          GearDir content(sectionDir, queryString.str() + "Content/");

          //Create the TGeo objects
          currCreator.create(content);
          m_calledCreators.push_back(creatorName);
        } else ERROR("Could not call creator. There was no creator defined (" + queryString.str() + "Creator" + ") !")
        }
    }
    gGeoManager->CloseGeometry();

    //After the geometry was closed, find the sensitive volumes. They are specified by the name prefix given by each creator.
    //Add the found sensitive detectors to the internal sensitive detector database.
    for (list<string>::iterator listIter = m_calledCreators.begin(); listIter != m_calledCreators.end(); listIter++) {
      CreatorBase& currCreator = creatorManager.getCreator(*listIter);
      if (currCreator.hasAutoSensitiveVolumes()) findSensitiveDetectorByPrefix(currCreator);
    }

  } catch (GbxExcIONotConnected&) {
    ERROR("Could not fetch parameters. No GearboxIO object was created !")
  } catch (GDetExcCreatorNotExists& exc) {
    ERROR("Could not call creator. A creator with the name '" + exc.getName() + "' does not exist !")
  } catch (GbxExcPathEmptyResult& exc) {
    ERROR("The result of the statement (" + exc.getXPathStatement() + ") is empty !")
  } catch (GbxExcXPathBase& exc) {
    ERROR("The path statement is not valid (" + exc.getXPathStatement() + ") !")
  } catch (GbxException) {
    ERROR("An error occurred during the creation of the detector geometry !")
  }
}


void GeoDetector::saveToRootFile(const std::string& filename)
{
  try {
    string detectorName = Gearbox::Instance().getParamString("/Detector/Name");
    gGeoManager->Export(filename.c_str(), detectorName.c_str());
  } catch (GbxExcIONotConnected&) {
    ERROR("Could not fetch parameters. No GearboxIO object was created !")
  } catch (GbxExcXPathBase& exc) {
    ERROR("Could not fetch parameters. The path statement is not valid (" + exc.getXPathStatement() + ") !")
  }
}


const list<string>& GeoDetector::getCalledCreators() const
{
  return m_calledCreators;
}


const list<TGeoVolume*>& GeoDetector::getSensitiveVolumes(const string& creatorName) const
throw(GDetExcCreatorNameEmpty, GDetExcCreatorNotExists)
{
  if (creatorName.empty()) throw GDetExcCreatorNameEmpty();

  map<string, SensitiveDetDBItem*>::const_iterator mapIter = m_sensitiveDetDB.find(creatorName);
  if (mapIter == m_sensitiveDetDB.end()) throw GDetExcCreatorNotExists(creatorName);

  return mapIter->second->getSensitiveVolumes();
}


//====================================================================
//                          Private methods
//====================================================================

void GeoDetector::findSensitiveDetectorByPrefix(CreatorBase& creator)
{
  m_senDetNumber = 0;

  //Create the path to the subdetector
  gGeoManager->CdTop();
  string subDetPath(gGeoManager->GetPath() + string("/") + creator.getGeoGroupName() + string("_1"));
  INFO_S("Constructing " << creator.getGeoGroupName() << " sensitive volumes from TGeo path '" << subDetPath << "'...")

  //Check if there is the specified subdetector available
  if (gGeoManager->CheckPath(subDetPath.c_str())) {
    gGeoManager->cd(subDetPath.c_str());

    //Create a new sensitive detector database item
    SensitiveDetDBItem* currDBItem = new SensitiveDetDBItem(creator.getGeoGroupName());

    //Go recursively through the subdetector volumes and search for volumes having the prefix given by the creator.
    setSensitiveDetectorRec(gGeoManager->GetCurrentVolume(), *currDBItem, creator.getAutoSenVolPrefix());

    //If there were sensitive volumes found, add the database item to the database.
    if (m_senDetNumber > 0) m_sensitiveDetDB.insert(make_pair(creator.getName(), currDBItem));
    else delete currDBItem;

  } else {
    ERROR("Could not find TGeo path to " << creator.getGeoGroupName() << " subdetector. Does the path " << subDetPath << " exist ?")
  }

  INFO_S("...constructed " << m_senDetNumber << " sensitive " << creator.getGeoGroupName() << " volumes.")
}


void GeoDetector::setSensitiveDetectorRec(TGeoVolume* volume, SensitiveDetDBItem& dbItem, const string& prefix)
{
  string currPath = gGeoManager->GetPath();

  //Check if current volume is a sensitive volume (search for prefix given as parameter)
  //If yes, add a pointer of the volume to the internal database.
  string detName(volume->GetName());
  if (detName.substr(0, prefix.length()).compare(prefix) == 0) {
    dbItem.addSensitiveVolume(volume);
    m_senDetNumber++;
  }

  //Go recursively through the daughters
  int nDaughter = volume->GetNdaughters();
  for (int iDaughter = 0; iDaughter < nDaughter; ++iDaughter) {
    gGeoManager->cd(currPath.c_str());
    gGeoManager->CdDown(iDaughter);
    setSensitiveDetectorRec(volume->GetNode(iDaughter)->GetVolume(), dbItem, prefix);
  }
}


void GeoDetector::clearSensitiveDetDB()
{
  map<string, SensitiveDetDBItem*>::iterator mapIter;
  for (mapIter = m_sensitiveDetDB.begin(); mapIter != m_sensitiveDetDB.end(); mapIter++) {
    delete mapIter->second;
  }

  m_sensitiveDetDB.clear();
}


GeoDetector::GeoDetector()
{
  //Fill the list with all supported sections
  //The order is important (the materials first !)
  m_supportedSections.push_back("MaterialSets");
  m_supportedSections.push_back("Global");
  m_supportedSections.push_back("Subdetectors");
}

GeoDetector::~GeoDetector()
{
  clearSensitiveDetDB();
}
