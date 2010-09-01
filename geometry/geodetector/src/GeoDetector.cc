/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/GbxExceptions.h>
#include <geometry/geodetector/GeoDetector.h>
#include <geometry/geodetector/CreatorManager.h>
#include <geometry/geodetector/GDetExceptions.h>
#include <framework/logging/Logger.h>

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


//====================================================================
//                          Private methods
//====================================================================

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

}
