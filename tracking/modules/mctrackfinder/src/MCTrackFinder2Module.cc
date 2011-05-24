/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/mctrackfinder/MCTrackFinder2Module.h>
#include <tracking/dataobjects/Track.h>


#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/dataobjects/PXDRecoHit.h>
#include <svd/dataobjects/SVDRecoHit.h>
#include <utility>
#include <vector>
#include <list>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MCTrackFinder2)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MCTrackFinder2Module::MCTrackFinder2Module() : Module()
{
  //Set module properties
  setDescription("Uses the MC information to create Relations between MCParticles and corresponding Tracks and between these Tracks and CDCRecoHits.");

  //Parameter definition
  addParam("MCParticlesColName", m_mcParticlesCollectionName, "Name of collection holding the MCParticles", string(DEFAULT_MCPARTICLES));
  addParam("MCParticleToCDCSimHitsColName", m_mcPartToCDCSimHitsCollectionName, "Name of collection holding the relations the MCParticles and the CDCSimHits (should be created during the simulation within CDCSensitiveDetector)", string(DEFAULT_MCPART_TO_CDCSIMHITS));
  addParam("CDCRecoHitsColName", m_cdcRecoHitsCollectionName, "Name of collection holding the CDCRecoHits (should be created by CDCRecoHitMaker module)", string("CDCRecoHits"));
  addParam("CDCSimHitToCDCHitColName", m_cdcSimHitToCDCHitCollectioName, "Name of collection holding the relations between CDCSimHits and CDCHits (CDCHit index = CDCRecoHit index) (should be created by CDCDigi module)", string("SimHitToCDCHits"));

  addParam("TracksColName", m_tracksCollectionName, "Name of collection holding the Tracks (output)", string("Tracks"));
  addParam("TrackToMCParticleColName", m_trackToMCParticleCollectionName, "Name of collection holding the relations between Tracks and MCParticles (output)", string("TrackToMCParticle"));
  addParam("TrackToCDCRecoHitColName", m_trackToCDCRecoHitCollectionName, "Name of collection holding the relations between Tracks and CDCRecoHits (output)", string("TrackToCDCRecoHits"));


  addParam("MCParticleToPXDSimHitsColName", m_mcPartToPxdSimHitsColName, "Name of collection holding the relations MCParticles->PXDSimHits", string(DEFAULT_PXDSIMHITSREL));
  addParam("MCParticleToSVDSimHitsColName", m_mcPartToSvdSimHitsColName, "Name of collection holding the relations MCParticles->SVDSimHits", string(DEFAULT_SVDSIMHITSREL));
  addParam("PXDRecoHitsColName", m_pxdRecoHitColName, "Name of collection holding the PXDRecoHits", string(DEFAULT_PXDRECOHITS));
  addParam("SVDRecoHitsColName", m_svdRecoHitColName, "Name of collection holding the SVDRecoHits", string(DEFAULT_SVDRECOHITS));
  addParam("TrackToPXDRecoHitColName", m_trackToPxdRecoHitCollectionName, "Name of collection holding the relations between Tracks and PXDRecoHits (output)", string("TrackToPXDRecoHits"));
  addParam("TrackToSVDRecoHitColName", m_trackToSvdRecoHitCollectionName, "Name of collection holding the relations between Tracks and SVDRecoHits (output)", string("TrackToSVDRecoHits"));

}



MCTrackFinder2Module::~MCTrackFinder2Module()
{

}


void MCTrackFinder2Module::initialize()
{
  dataOutCdc.open("trfcdc");
  dataOutPxd.open("trfpx");
  dataOutSvd.open("trfsv");
}



void MCTrackFinder2Module::event()
{
  B2INFO("*******   MCTrackFinder2Module  *******");
  StoreArray<MCParticle> mcParticles(m_mcParticlesCollectionName);
  if (mcParticles.GetEntries() == 0) B2WARNING("MCTrackFinder2: MCParticlesCollection is empty!");
  //CDC
  StoreArray<Relation>   mcPartToCdcSimHits(m_mcPartToCDCSimHitsCollectionName);
  if (mcPartToCdcSimHits.GetEntries() == 0) B2WARNING("MCTrackFinder2: MCParticlesToSimHitsCollection is empty!");
  dataOutCdc << "nSimRel " << mcPartToCdcSimHits.GetEntries() << "\n";
  StoreArray<Relation>  cdcSimHitToCDCHits(m_cdcSimHitToCDCHitCollectioName);
  //B2INFO("MCTrackFinder2: Number of relations between SimHits and CDCHits: "<<cdcSimHitToCDCHits.GetEntries());
  if (cdcSimHitToCDCHits.GetEntries() == 0) B2WARNING("MCTrackFinder2: SimHitsToCDCHitsCollection is empty!");

  StoreArray<CDCRecoHit> cdcRecoHits(m_cdcRecoHitsCollectionName);
  //B2INFO("MCTrackFinder2: Number of CDCRecoHits: "<<cdcRecoHits.GetEntries());
  if (cdcRecoHits.GetEntries() == 0) B2WARNING("MCTrackFinder2: CDCRecoHitsCollection is empty!");

  StoreArray<PXDRecoHit> pxdRecoHits(m_pxdRecoHitColName);
  dataOutPxd << "nRecoHits " << pxdRecoHits.GetEntries() << "\n";
  StoreArray<Relation>   mcPartToPxdSimHits(m_mcPartToPxdSimHitsColName);
  dataOutPxd << "nSimRel " << mcPartToPxdSimHits.GetEntries() << "\n";
  StoreArray<Relation>   mcPartToPxdRecoHits(DEFAULT_PXDRECOHITSREL);
  dataOutPxd << "nRecoRel " << mcPartToPxdRecoHits.GetEntries() << "\n";

  StoreArray<SVDRecoHit> svdRecoHits(m_svdRecoHitColName);
  dataOutSvd << "nRecoHits " << svdRecoHits.GetEntries() << "\n";
  StoreArray<Relation>   mcPartToSvdSimHits(m_mcPartToSvdSimHitsColName);
  dataOutSvd << "nSimRel " << mcPartToSvdSimHits.GetEntries() << "\n";
  StoreArray<Relation>   mcPartToSvdRecoHits(DEFAULT_SVDRECOHITSREL);
  dataOutSvd << "nRecoRel " << mcPartToSvdRecoHits.GetEntries() << "\n";

  StoreArray<PXDSimHit> pxdSimHits(DEFAULT_PXDSIMHITS);
  int nPxdSimHits = pxdSimHits.GetEntries();
  dataOutPxd << "nSimHit " << nPxdSimHits << "\n";
  dataOutPxd << "trackId ";
  for (int i = 0; i not_eq nPxdSimHits; ++i) {
    int trackId = pxdSimHits[i]->getTrackID();
    dataOutPxd << trackId << " ";
  }
  dataOutPxd << "\n";

  StoreArray<SVDSimHit> svdSimHits(DEFAULT_SVDSIMHITS);
  int nSvdSimHits = svdSimHits.GetEntries();
  dataOutSvd << "nSimHit " << nSvdSimHits << "\n";
  dataOutSvd << "trackId ";
  for (int i = 0; i not_eq nSvdSimHits; ++i) {
    int trackId = svdSimHits[i]->getTrackID();
    dataOutSvd << trackId << " ";
  }
  dataOutSvd << "\n";

  StoreArray<CDCSimHit> cdcSimHits(DEFAULT_CDCSIMHITS);
  int nCdcSimHits = cdcSimHits.GetEntries();
  dataOutCdc << "nSimHit " << nCdcSimHits << "\n";
  dataOutCdc << "trackId ";
  for (int i = 0; i not_eq nCdcSimHits; ++i) {
    int trackId = cdcSimHits[i]->getTrackId();
    dataOutCdc << trackId << " ";
  }
  dataOutCdc << "\n";

  B2INFO("-> Create relations for primary particles");
  // loop over MCParticles.
  int nMcParticles = mcParticles->GetEntries();
  for (int iPart = 0; iPart < nMcParticles; iPart++) {

    //make links only for interesting MCParticles, for the moment take only primary particle
    // which means only particles createc by the particle gun
    if (mcParticles[iPart]->getMother() == NULL) {
      B2DEBUG(149, "iPart: " << iPart);

      // create a containter "indicesOfGoodCdcHits" containing the indices to cdcdigiHits / cdcrecoHits
      // which were cause by the particle gun partiles
      list<int> indicesOfGoodCdcHits;
      int nMcPartToCdcSimHits =  mcPartToCdcSimHits->GetEntries();
      for (int i = 0; i < nMcPartToCdcSimHits; i++) {
        if (mcPartToCdcSimHits[i]->getFromIndex() == iPart) {
          int cdcSimHitIndex = mcPartToCdcSimHits[i]->getToIndex();
          int nCdcSimHitToCDCHitCol = cdcSimHitToCDCHits.GetEntries();
          for (int j = 0; j not_eq nCdcSimHitToCDCHitCol; ++j) {
            if (cdcSimHitToCDCHits[j]->getFromIndex() == cdcSimHitIndex) {
              int cdcHitIndex = cdcSimHitToCDCHits[j]->getToIndex();
              if (cdcHitIndex >= 0) {
                indicesOfGoodCdcHits.push_back(cdcHitIndex);
              }
            }
          }
        }
      }
      // same as above for pxd reco hits the code block is a few lines shorter
      // because there are direct relations between mcParticles and the recoHits
      list<int> indicesOfGoodPxdHits;
      int nMcPartToPxdRecoHits =  mcPartToPxdRecoHits->GetEntries();
      //dataOutPxd << "nRecoRel " << nMcPartToPxdRecoHits << "\n";
      for (int i = 0; i not_eq nMcPartToPxdRecoHits; i++) {
        if (mcPartToPxdSimHits[i]->getFromIndex() == iPart) {
          int pxdRecoHitIndex = mcPartToPxdSimHits[i]->getToIndex();

          indicesOfGoodPxdHits.push_back(pxdRecoHitIndex);
        }
      }
      // same as above for svd reco hits
      list<int> indicesOfGoodSvdHits;
      int nMcPartToSvdRecoHits =  mcPartToSvdRecoHits->GetEntries();
      //dataOutSvd << "nRecoRel " << nMcPartToSvdRecoHits << "\n";
      for (int i = 0; i not_eq nMcPartToSvdRecoHits; i++) {
        if (mcPartToSvdSimHits[i]->getFromIndex() == iPart) {
          int svdRecoHitIndex = mcPartToSvdSimHits[i]->getToIndex();

          indicesOfGoodSvdHits.push_back(svdRecoHitIndex);
        }
      }
      //B2INFO("Nr of Hits: "<<otherList.size());
      //every CDCHit will have a CDCRecoHit.
      //Therefore I should now be able to create a Relation, that points to the RecoHits, that really belong to the same track.

      //Now create Tracks and the relations
      StoreArray<Track> tracks(m_tracksCollectionName);
      StoreArray<Relation> trackToCDCRecoHits(m_trackToCDCRecoHitCollectionName);
      StoreArray<Relation> trackToMCParticles(m_trackToMCParticleCollectionName);
      StoreArray<Relation> trackToPxdRecoHits(m_trackToPxdRecoHitCollectionName);
      StoreArray<Relation> trackToSvdRecoHits(m_trackToSvdRecoHitCollectionName);
      int counter = tracks->GetLast() + 1;
      B2DEBUG(100, counter);

      //create Track
      new(tracks->AddrAt(counter)) Track();
      //create relation between the track and cdcRecoHits
      dataOutCdc << counter << "\n";

      list<int>::const_iterator iter = indicesOfGoodCdcHits.begin();
      list<int>::const_iterator iterMax = indicesOfGoodCdcHits.end();
      dataOutCdc << "nGoodHits " << indicesOfGoodCdcHits.size() << " | ";
      while (iter not_eq iterMax) {
        dataOutCdc << *iter << " ";
        ++iter;
      }
      dataOutCdc << "\n";


      iter = indicesOfGoodPxdHits.begin();
      iterMax = indicesOfGoodPxdHits.end();
      dataOutPxd << "nGoodHits " << indicesOfGoodPxdHits.size() << " | ";
      while (iter not_eq iterMax) {
        dataOutPxd << *iter << " ";
        ++iter;
      }
      dataOutPxd << "\n";
      iter = indicesOfGoodSvdHits.begin();
      iterMax = indicesOfGoodSvdHits.end();
      dataOutSvd << "nGoodHits " << indicesOfGoodSvdHits.size() << " | ";
      while (iter not_eq iterMax) {
        dataOutSvd << *iter << " ";
        ++iter;
      }
      dataOutSvd << "\n";
      new(trackToCDCRecoHits->AddrAt(counter)) Relation(tracks, cdcRecoHits, counter, indicesOfGoodCdcHits);
      B2INFO("Create relation between Track " << counter << "  and  " << indicesOfGoodCdcHits.size() << "  RecoHits");
      // create track -> PxdRecoHit relation
      new(trackToPxdRecoHits->AddrAt(counter)) Relation(tracks, pxdRecoHits, counter, indicesOfGoodPxdHits);
      // create track -> SvdRecoHit relation
      new(trackToSvdRecoHits->AddrAt(counter)) Relation(tracks, svdRecoHits, counter, indicesOfGoodSvdHits);
      //create relation between th track and the mcParticle
      dataOutCdc << counter << " " << iPart << "\n";
      new(trackToMCParticles->AddrAt(counter)) Relation(tracks, mcParticles, counter, iPart);
      //B2INFO(" --- Create relation between Track "<<counter<<"  and MCParticle "<<iPart);


    } //endif

  }//end loop over MCParticles



}


void MCTrackFinder2Module::terminate()
{
  dataOutCdc.close();
  dataOutPxd.close();
  dataOutSvd.close();
}
