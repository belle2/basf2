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


