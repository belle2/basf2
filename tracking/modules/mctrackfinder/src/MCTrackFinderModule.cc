/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/mctrackfinder/MCTrackFinderModule.h>
#include <tracking/dataobjects/Track.h>


#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <utility>
#include <list>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MCTrackFinder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MCTrackFinderModule::MCTrackFinderModule() : Module()
{
  //Set module properties
  setDescription("Creates CDCRecoHits from CDCHits.");

  //Parameter definition
  addParam("TrackToCDCRecoHitCollectionName", m_trackToCDCRecoHitCollectionName,
           "Name of collection holding the relations between track and CDCRecoHit", string("TrackToCDCRecoHits"));
}


MCTrackFinderModule::~MCTrackFinderModule()
{

}


void MCTrackFinderModule::initialize()
{
}



void MCTrackFinderModule::event()
{
  StoreArray<MCParticle> mcParticles("MCParticles");
  StoreArray<Relation>   cdcSimHitToMCParticles("MCPartToCDCSimHits");

  // loop over MCParticles.
  for (unsigned short int iPart = 0; iPart < mcParticles->GetEntriesFast(); iPart++) {

    //make links only for interesting MCParticles, assuming these are
    if (mcParticles[iPart]->getEnergy() > 0.03 && abs(mcParticles[iPart]->getPDG()) == 13 /*check for not being neutrino*/) {
      B2DEBUG(149, "iPart: " << iPart);
      list<int> myList = getFromForTo("MCPartToCDCSimHits", iPart);
      list<int> otherList;

      // Not all SimHits have corresponding real hit. Find real CDCHits associated with the MCParticles:
      // Therefore I try to change the numbers in the list associated with each hit to the ones of the real CDCHits.
      for (list<int>::iterator iter = myList.begin(), iterEnd = myList.end(); iter != iterEnd; iter++) {
        int cdcHit = getToForFrom("SimHitToCDCHits", (*iter));
        if (cdcHit >= 0) {
          B2DEBUG(149, cdcHit);
          otherList.push_back(cdcHit);
        }
      }
      //every CDCHit will have a CDCRecoHit.
      //Therefore I should now be able to create a Relation, that points to the RecoHits, that really belong to the same track.
      StoreArray<Relation>trackToCDCRecoHits(m_trackToCDCRecoHitCollectionName);
      StoreArray<Track> tracks("Tracks");
      int counter = tracks->GetLast() + 1;
      B2DEBUG(100, counter);
      new(tracks->AddrAt(counter)) Track();
      StoreArray<CDCRecoHit> cdcRecoHits("CDCRecoHits");

      new(trackToCDCRecoHits->AddrAt(counter)) Relation(tracks, cdcRecoHits, counter, otherList);
    }

  }//end loop over MCParticles



}


void MCTrackFinderModule::terminate()
{
}


list<int> MCTrackFinderModule::getFromForTo(const std::string& relationName, const unsigned short int& toIndex)
{
  StoreArray<Relation> relations(relationName);
  list<int> myList;
  for (int ii = 0; ii < relations->GetEntriesFast(); ii++) {
    if (relations[ii]->getToIndex() == toIndex) {
      myList.push_back(relations[ii]->getFromIndex());
      B2DEBUG(250, relations[ii]->getFromIndex());
    }
  }
  return (myList);
}
