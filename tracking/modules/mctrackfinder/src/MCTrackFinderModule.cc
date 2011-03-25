/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
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
  setDescription("Uses the MC information to create Relations between MCParticles and corresponding Tracks and between these Tracks and CDCRecoHits.");

  //Parameter definition
  addParam("MCParticlesColName", m_mcParticlesCollectionName, "Name of collection holding the MCParticles", string("MCParticles"));
  addParam("MCParticleToCDCSimHitsColName", m_mcPartToCDCSimHitsCollectionName, "Name of collection holding the relations the MCParticles and the CDCSimHits (should be created during the simulation within CDCSensitiveDetector)", string("MCPartToCDCSimHits"));
  addParam("CDCRecoHitsColName", m_cdcRecoHitsCollectionName, "Name of collection holding the CDCRecoHits (should be created by CDCRecoHitMaker module)", string("CDCRecoHits"));
  addParam("CDCSimHitToCDCHitColName", m_cdcSimHitToCDCHitCollectioName, "Name of collection holding the relations between CDCSimHits and CDCHits (CDCHit index = CDCRecoHit index) (should be created by CDCDigi module)", string("SimHitToCDCHits"));

  addParam("TracksColName", m_tracksCollectionName, "Name of collection holding the Tracks (output)", string("Tracks"));
  addParam("TrackToMCParticleColName", m_trackToMCParticleCollectionName, "Name of collection holding the relations between Tracks and MCParticles (output)", string("TrackToMCParticle"));
  addParam("TrackToCDCRecoHitColName", m_trackToCDCRecoHitCollectionName, "Name of collection holding the relations between Tracks and CDCRecoHits (output)", string("TrackToCDCRecoHits"));
}


MCTrackFinderModule::~MCTrackFinderModule()
{

}


void MCTrackFinderModule::initialize()
{
}



void MCTrackFinderModule::event()
{
  B2INFO("*******   MCTrackFinderModule  *******");
  StoreArray<MCParticle> mcParticles(m_mcParticlesCollectionName);
  //B2INFO("MCTrackFinder: total Number of MCParticles: "<<mcParticles.GetEntries());
  if (mcParticles.GetEntries() == 0) B2WARNING("MCTrackFinder: MCParticlesCollection is empty!");

  StoreArray<Relation>   mcPartToSimHits(m_mcPartToCDCSimHitsCollectionName);
  //B2INFO("MCTrackFinder: Number of relations between MCParticles and SimHits: "<<mcPartToSimHits.GetEntries());
  if (mcPartToSimHits.GetEntries() == 0) B2WARNING("MCTrackFinder: MCParticlesToSimHitsCollection is empty!");

  StoreArray<Relation>  cdcSimHitToCDCHits(m_cdcSimHitToCDCHitCollectioName);
  //B2INFO("MCTrackFinder: Number of relations between SimHits and CDCHits: "<<cdcSimHitToCDCHits.GetEntries());
  if (cdcSimHitToCDCHits.GetEntries() == 0) B2WARNING("MCTrackFinder: SimHitsToCDCHitsCollection is empty!");

  StoreArray<CDCRecoHit> cdcRecoHits(m_cdcRecoHitsCollectionName);
  //B2INFO("MCTrackFinder: Number of CDCRecoHits: "<<cdcRecoHits.GetEntries());
  if (cdcRecoHits.GetEntries() == 0) B2WARNING("MCTrackFinder: CDCRecoHitsCollection is empty!");

  B2INFO("-> Create relations for primary particles");
  // loop over MCParticles.
  for (unsigned short int iPart = 0; iPart < mcParticles->GetEntriesFast(); iPart++) {

    //make links only for interesting MCParticles, for the moment take only primary particle
    if (mcParticles[iPart]->getMother() == NULL) {
      B2DEBUG(149, "iPart: " << iPart);

      //Get indices of the SimHits from this MCParticle
      list<short unsigned int> myList;

      for (int i = 0; i < mcPartToSimHits->GetEntries(); i++) {
        if (mcPartToSimHits[i]->getFromIndex() == iPart) {
          myList.push_back(mcPartToSimHits[i]->getToIndex()); //this was different before, as the relation in CDCSensitiveCetector was created other way round
        }
      }
      //B2INFO("Nr of SimHits: "<<myList.size());
      //Now get all digitized Hits for this MCParticle

      list<int> otherList;
      // Not all SimHits have corresponding real hit. Find real CDCHits associated with the MCParticles:
      // Therefore I try to change the numbers in the list associated with each hit to the ones of the real CDCHits.
      for (list<short unsigned int>::iterator iter = myList.begin(), iterEnd = myList.end(); iter != iterEnd; iter++) {
        int cdcHit = getToForFrom(m_cdcSimHitToCDCHitCollectioName, (*iter));
        if (cdcHit >= 0) {
          B2DEBUG(149, cdcHit);
          otherList.push_back(cdcHit);
        }
      }


      //B2INFO("Nr of Hits: "<<otherList.size());
      //every CDCHit will have a CDCRecoHit.
      //Therefore I should now be able to create a Relation, that points to the RecoHits, that really belong to the same track.

      //Now create Tracks and the relations
      StoreArray<Track> tracks(m_tracksCollectionName);
      StoreArray<Relation> trackToCDCRecoHits(m_trackToCDCRecoHitCollectionName);
      StoreArray<Relation> trackToMCParticles(m_trackToMCParticleCollectionName);

      int counter = tracks->GetLast() + 1;
      B2DEBUG(100, counter);

      //create Track
      new(tracks->AddrAt(counter)) Track();
      //create relation between the track and cdcRecoHits
      new(trackToCDCRecoHits->AddrAt(counter)) Relation(tracks, cdcRecoHits, counter, otherList);
      B2INFO("Create relation between Track " << counter << "  and  " << otherList.size() << "  RecoHits");
      //create relation between th track and the mcParticle
      new(trackToMCParticles->AddrAt(counter)) Relation(tracks, mcParticles, counter, iPart);
      //B2INFO(" --- Create relation between Track "<<counter<<"  and MCParticle "<<iPart);


    } //endif

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
