/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Johannes Rauch                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/genfitVisModule/GenfitVisModule.h"

#include <framework/datastore/StoreArray.h>

#include <geometry/GeometryManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <genfit/Track.h>
#include <genfit/GFRaveVertex.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/TGeoMaterialInterface.h>

#include <TGeoManager.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GenfitVis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GenfitVisModule::GenfitVisModule() : Module()
{
  // Set module properties
  setDescription("Visualize genfit::Tracks using the genfit::EventDisplay.");

  // Parameter definitions
  addParam("onlyBadTracks", m_onlyBadTracks, "show only unfitted and unconverged tracks", false);

}

void GenfitVisModule::initialize()
{
  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
      geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
      geoManager.createTGeoRepresentation();
    }
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());

    // activate / deactivate material effects in genfit
    genfit::MaterialEffects::getInstance()->setEnergyLossBetheBloch(true);
    genfit::MaterialEffects::getInstance()->setNoiseBetheBloch(true);
    genfit::MaterialEffects::getInstance()->setNoiseCoulomb(true);
    genfit::MaterialEffects::getInstance()->setEnergyLossBrems(true);
    genfit::MaterialEffects::getInstance()->setNoiseBrems(true);

    genfit::MaterialEffects::getInstance()->setMscModel("Highland");
  }

  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2WARNING("Magnetic field not set up, doing this myself.  Please use SetupGenfitExtrapolationModule.");

    //pass the magnetic field to genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::FieldManager::getInstance()->useCache();
  }

  m_display = genfit::EventDisplay::getInstance();
}

void GenfitVisModule::event()
{
  StoreArray < genfit::Track > gfTracks("GF2Tracks");
  std::vector<const genfit::Track*> tracks;

  const int nTracks = gfTracks.getEntries();
  for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
    const genfit::Track* tr = gfTracks[iTrack];
    if (m_onlyBadTracks &&
        tr->getFitStatus()->isFitted() &&
        tr->getFitStatus()->isFitConverged())
      continue;
    //std::cout << " =============== pushing track ======= " << std::endl;
    tracks.push_back(tr);
  }

  if (tracks.size() > 0)
    m_display->addEvent(tracks);
}

void GenfitVisModule::endRun()
{
  m_display->open();
}

void GenfitVisModule::terminate()
{
}


