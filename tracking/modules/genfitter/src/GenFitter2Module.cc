/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/genfitter/GenFitter2Module.h>

#include <framework/dataobjects/Relation.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <tracking/cdcConformalTracking/CDCTrackCandidate.h>
#include <tracking/dataobjects/Track.h>

#include <tracking/karlsruhe/MCMatchParticle.h>

#include "GFTrack.h"
#include "GFKalman.h"
//#include "GFDaf.h"

#include "GFAbsTrackRep.h"
#include "RKTrackRep.h"

#include "GFConstField.h"
#include "GFFieldManager.h"

#include <cstdlib>
#include <iomanip>
#include <string>

#include <iostream>

#include <boost/foreach.hpp>

#include <GFDetPlane.h>

using namespace std;
using namespace Belle2;

REG_MODULE(GenFitter2)

GenFitter2Module::GenFitter2Module() :
    Module()
{

  //setDescription("Uses GenFit to fit tracks within the CDC. Can fit the MCTracks or Tracks provided by the CDCTrackingModule.");

  //addParam("MCParticlesColName", m_mcParticlesCollectionName, "Name of collection holding the MCParticles", string("MCParticles"));
  //addParam("CDCRecoHitsColName", m_cdcRecoHitsCollectionName, "CDCRecoHits collection (should be created by CDCRecoHitMaker module)", string("CDCRecoHits"));

  //addParam("TracksColName", m_tracksCollectionName, "Name of collection holding the Tracks (for MCTracks: should be created by MCTrackFinderModule and filled with 'content' after the fit, for recoTracks: will be created after the fit)", string("Tracks"));
  //addParam("TrackToCDCRecoHitColName",  m_trackToCDCRecoHitCollectionName, "Name of collection holding the relations between Tracks and CDCRecoHits (should be created by MCTrackFinderModule)", string("TrackToCDCRecoHits"));
  //addParam("TrackToMCParticleColName", m_trackToMCParticleCollectionName, "Name of collection holding the relations between Tracks and MCParticles (should be created by MCTrackFinderModule)", string("TrackToMCParticle"));

  //addParam("CDCTrackCandidatesColName", m_cdcTrackCandsCollectionName, "Name of collection holding the CDCTrackCandidates (output of the pattern recognition, should be created by CDCTrackingModule)", string("CDCTrackCandidates"));
  //addParam("CDCTrackCandsToCDCRecoHitsColName", m_cdcTrackCandToRecoHitsCollectionName, "Name of collection holding the relations between CDCTrackCandidates and CDCRecoHits (should be created by CDCTrackingModule)", string("CDCTrackCandidatesToCDCRecoHits"));
  //addParam("CDCTrackCandsToMCParticlesColName", m_cdcTrackCandToMCParticleCollectionName, "Relation of CDCTrackCandidates to MCParticles (should be created by CDCMCMatchingModule)", string("CDCTrackCandidateToMCParticle"));

  //addParam("MCMatchParticlesColName", m_mcMatchParticlesCollectionName, "Name of collection holding the MCMatchParticles (should be created by CDCMCMatchingModule)", string("MCMatchParticles"));

// addParam("FitMCTracks", m_fitMCTracks, "True if MC tracks should be fitted", true);
// addParam("FitRecoTracks", m_fitRecoTracks, "True if track candidates from pattern recognition should be fitted", false);
  //addParam("TrackToPXDRecoHitColName",  m_trackToPxdRecoHitColName, "Name of collection holding the relations between Tracks and PXDRecoHits (should be created by MCTrackFinderModule)", string("TrackToPXDRecoHits"));
  //addParam("PXDRecoHitsColName", m_pxdRecoHitsColName, "PXDRecoHits collection (should be created by PXDRecoHitMaker module)", string(DEFAULT_PXDRECOHITS));
  //addParam("TrackToSVDRecoHitColName",  m_trackToSvdRecoHitColName, "Name of collection holding the relations between Tracks and SVDRecoHits (should be created by MCTrackFinderModule)", string("TrackToSVDRecoHits"));
  //addParam("SVDRecoHitsColName", m_svdRecoHitsColName, "SVDRecoHits collection (should be created by SVDRecoHitMaker module)", string(DEFAULT_SVDRECOHITS));
}

GenFitter2Module::~GenFitter2Module()
{
}

void GenFitter2Module::initialize()
{
  //dataOut.open("kaltest");
  // Initialize Output StoreArray. It contains the GFTracks
  //StoreArray<GFTrack> fittedTracks("fittedTracks");
}

void GenFitter2Module::beginRun()
{

}

void GenFitter2Module::event()
{
  B2INFO("**********   GenFitter2Module  ************");
  StoreArray<MCParticle> mcParticles("MCParticles");
  B2INFO("GenFitter2: total Number of MCParticles: " << mcParticles.GetEntries());
  if (mcParticles.GetEntries() == 0) B2WARNING("GenFitter2: MCParticlesCollection is empty!");

  //StoreArray<CDCRecoHit> cdcRecoHits(m_cdcRecoHitsCollectionName);
  //B2INFO("GenFitter2: Number of CDCRecoHits: " << cdcRecoHits.GetEntries());
  //if (cdcRecoHits.GetEntries() == 0) B2WARNING("GenFitter2: CDCRecoHitsCollection is empty!");

  StoreArray<PXDRecoHit> pxdRecoHits(DEFAULT_PXDRECOHITS);
  B2INFO("GenFitter2: Number of PXDRecoHits: " << pxdRecoHits.GetEntries());
  if (pxdRecoHits.GetEntries() == 0) B2WARNING("GenFitter2: PXDRecoHitsCollection is empty!");
  //StoreArray<SVDRecoHit> svdRecoHits(m_svdRecoHitsColName);
  //B2INFO("GenFitter2: Number of SVDRecoHits: " << svdRecoHits.GetEntries());
  //if (svdRecoHits.GetEntries() == 0) B2WARNING("GenFitter2: SVDRecoHitsCollection is empty!");
  //The use of MCMatchParticles is optional, the fitting of MCtruth should work without CDCMCMatchingModule
  //StoreArray<MCMatchParticle> mcMatchParticles(m_mcMatchParticlesCollectionName);
  //Give Genfit the magnetic field, should come from the common database later...
  GFFieldManager::getInstance()->init(new GFConstField(0., 0., 15.));

//  if (m_fitMCTracks == true) {

  B2INFO("-------   Fitting using true MCparticle tracks  ---------------");

  //StoreArray<Track> tracks(m_tracksCollectionName);
  //B2INFO("GenFitter2: Number of Tracks: " << tracks.GetEntries());
  //if (tracks.GetEntries() == 0) B2WARNING("GenFitter2: TracksCollection is empty!");

// StoreArray<Relation> trackToCDCRecoHits(m_trackToCDCRecoHitCollectionName);
  //B2INFO("GenFitter: Number of relations between Tracks and CDCRecoHits: "<<trackToCDCRecoHits.GetEntries());
  //if (trackToCDCRecoHits.GetEntries() == 0) B2WARNING("GenFitter2: TrackToCDCRecoHitsCollection is empty!");

  //StoreArray<Relation> trackToPxdRecoHits(m_trackToPxdRecoHitColName);
  //if (trackToPxdRecoHits.GetEntries() == 0) B2WARNING("GenFitter2: TrackToPXDRecoHitColName is empty!");
  //StoreArray<Relation> trackToSvdRecoHits(m_trackToSvdRecoHitColName);
  //if (trackToSvdRecoHits.GetEntries() == 0) B2WARNING("GenFitter2: TrackToSVDRecoHitColName is empty!");
  //StoreArray<Relation> trackToMCPart(m_trackToMCParticleCollectionName);
  //B2INFO("GenFitter: Number of relations between Tracks and MCParticles: "<<trackToMCPart.GetEntries());
// if (trackToMCPart.GetEntries() == 0) B2WARNING("GenFitter2: TrackToMCParticlesCollection is empty!");
  //StoreArray<GFTrack> fittedTracks("fittedTracks"); //holds the output of this module



  //loop over all tracks in one event (so currently alsways just one track)
  //int nTracks = tracks->GetEntries();
  //for (int i = 0; i < nTracks; i++) {
//    B2INFO("#### Fit track Nr. : " << i);
  //get the index of the MCParticle which produced this track
  //int mcindex = trackToMCPart[i]->getToIndex();

  //get fit starting values from the MCParticle
  TVector3 vertex = mcParticles[0]->getProductionVertex();
  TVector3 momentum = mcParticles[0]->getMomentum();
  int pdg = mcParticles[0]->getPDG();

  //B2INFO("MCIndex: "<<mcindex);
  B2INFO("Start values: momentum: " << momentum.x() << "  " << momentum.y() << "  " << momentum.z() << " " << momentum.Mag());
  B2INFO("Start values: vertex:   " << vertex.x() << "  " << vertex.y() << "  " << vertex.z());
  B2INFO("Start values: pdg:      " << pdg);

  //Now create a GenFit track with this representation
  GFAbsTrackRep* trackRep = new RKTrackRep(vertex, momentum, pdg);
  GFTrack track(trackRep, false);
  track.Print();
  // check if extrapotlatin thorough all material works
  /*GFAbsTrackRep* trackRep2 = new RKTrackRep(vertex, momentum, pdg);
  cerr << "start " << momentum.Mag();
  TVector3 myO = momentum;
  cerr << "2"<< flush;
  TVector3 my1 = 1./myO.Mag() * 14 *myO;
  cerr << "3";
  GFDetPlane myD(my1,momentum);
  cerr << "4 ";
  cerr << "mom at 14 " << trackRep2->getMom(myD).Mag();
  cerr << " |\n";*/
  //trackRep->Print();

  //Collect all RecoHits for this track
  //Caution: for the moment RecoHits are not sorted, but GenFit needs them in a certain order.
  //It seems that that order is by chance correct for simple single tracks, but the fit may fail for more complicated events.
  //int hitCounter = -1;
  //list<short unsigned int> pxdIndexList = trackToPxdRecoHits[i]->getToIndices();
  //list<short unsigned int>::const_iterator iter = pxdIndexList.begin();
  //list<short unsigned int>::const_iterator iterMax = pxdIndexList.end();
  /*   while (iter not_eq iterMax) {
       hitCounter++;
       track.addHit(pxdRecoHits[*iter], 0, hitCounter);
       int aSensorUniID = pxdRecoHits[*iter]->getSensorUniID();
       SensorUniIDManager aIdConverter(aSensorUniID);
       int layerId = aIdConverter.getLayerID();
       int ladderId = aIdConverter.getLadderID();
       int sensorId = aIdConverter.getSensorID();
       B2INFO("====== NEXT HIT pxd " << *iter << " " << layerId << " " << ladderId << " " << sensorId);
       //GFDetPlane aGFDetPlane = pxdRecoHits[j]->getDetPlane(trackRep);
       //cout << "print pxd plane" << flush;
       //aGFDetPlane.Print();
       ++iter;
     }//end loop over pxd RecoHits
  */
  //track.addHit(pxdRecoHits[0], 0, 0);
  //track.addHit(pxdRecoHits[1], 0, 1);
  //track.addHit(pxdRecoHits[2], 0, 2);
  //SVD
  pxdRecoHits[0]->Print();
  track.addHit(pxdRecoHits[0], 1, 0);
  /*cerr << "ITERATORS ";
  list<short unsigned int> svdIndexList = trackToSvdRecoHits[i]->getToIndices();
  list<short unsigned int>::const_iterator iter = svdIndexList.begin();
  list<short unsigned int>::const_iterator iterMax = svdIndexList.end();
  while (iter not_eq iterMax) {
    hitCounter++;
    cerr << *iter << " ";
    SVDRecoHit* aSVDRecoHit = svdRecoHits[*iter];
    aSVDRecoHit->Print();
    track.addHit(aSVDRecoHit, 1, hitCounter);
    cerr << "after addHit" << endl;
    /*int aSensorUniID = svdRecoHits[j]->getSensorUniID();
    SensorUniIDManager aIdConverter(aSensorUniID);
    int layerId = aIdConverter.getLayerID();
    int ladderId = aIdConverter.getLadderID();
    int sensorId = aIdConverter.getSensorID();
    B2INFO("====== NEXT HIT svd " << j << " " << layerId << " " << ladderId << " " << sensorId);*/
  //GFDetPlane aGFDetPlane = svdRecoHits[j]->getDetPlane(trackRep);
  //cout << "print svd plane" << flush;
  //aGFDetPlane.Print();
  //++iter;
  //}//end loop over SVD RecoHits

  /*track.addHit(svdRecoHits[0], 1, 3);
  track.addHit(svdRecoHits[1], 1, 1);
  track.addHit(svdRecoHits[2], 1, 2);
  track.addHit(svdRecoHits[3], 1, 3);*/

  //hitCounter = 1;
  /* list<short unsigned int> indexList = trackToCDCRecoHits[i]->getToIndices();
   int nCdcRecoHits = cdcRecoHits->GetEntries();
   for (int j = 0; j not_eq nCdcRecoHits; j++) {

     // list<short unsigned int>::iterator iter;
     // for (iter = indexList.begin(); iter != indexList.end(); iter++) {
     BOOST_FOREACH(int hit, indexList) {
       if (j == hit) {
         //B2INFO("Add CDC Reco Hit");
         hitCounter++;
         track.addHit(cdcRecoHits[j], 2, hitCounter);
         //GFDetPlane aGFDetPlane = cdcRecoHits[j]->getDetPlane(trackRep);
         //cout << "print cdc plane" << flush;
         //aGFDetPlane.Print();
       }
     }
   }//end loop over cdc RecoHits
  */

  /*track.addHit(cdcRecoHits[0], 2, 0);
  track.addHit(cdcRecoHits[1], 2, 1);
  track.addHit(cdcRecoHits[2], 2, 2);
  track.addHit(cdcRecoHits[3], 2, 3);
  track.addHit(cdcRecoHits[4], 2, 4);*/

//    B2INFO("Number of hits assigned to the track to be fitted: " << track.getNumHits());
  /*TVector3 magField = GFFieldManager::getFieldVal(vertex);
  B2INFO("Field " << magField.x() << "  " << magField.y() << "  " << magField.z());
  TVector3 testPoint(1,1,1);
  magField = GFFieldManager::getFieldVal(testPoint);
  B2INFO("Field " << magField.x() << "  " << magField.y() << "  " << magField.z());
  testPoint.SetXYZ(-0.5,0.5,-0.5);
  magField = GFFieldManager::getFieldVal(testPoint);
  B2INFO("Field " << magField.x() << "  " << magField.y() << "  " << magField.z());*/
  //Initialize fitting algorithm and process track
  /*    GFKalman k;
      //GFDaf daf;
      k.setNumIterations(2);
      k.processTrack(&track);
      //track.Print();
      cerr << "1";
      B2INFO("----> Status of fit: " << trackRep->getStatusFlag());
      B2INFO("-----> Fit Result: momentum: " << track.getMom().x() << "  " << track.getMom().y() << "  " << track.getMom().z() << " " << track.getMom().Mag());
      B2INFO("----> Chi2 of the fit: " << track.getChiSqu());
      B2INFO("----> NDF of the fit: " << track.getNDF());
      if (trackRep->getStatusFlag() == 0) {
        new(fittedTracks->AddrAt(i)) GFTrack(track);
        //chi2values.push_back(track.getChiSqu());
        //ndfs.push_back(track.getNDF());
        //absMoms.push_back(track.getMom().Mag());
      }
      //if the MCMatchParticles were created, assign the fitted momentum
      /*if (mcMatchParticles->GetEntries() != 0) {
        mcMatchParticles[mcindex]->setFitMCMomentum(track.getMom());
        mcMatchParticles[mcindex]->setChi2(track.getChiSqu());
      }*/

  //track.releaseHits();


// }//end loop over tracks

  cerr << "end of event in genfitter2" << endl;
}

void GenFitter2Module::endRun()
{

}

void GenFitter2Module::terminate()
{
  //dataOut << "# absMoms "
  /*int vecSize = absMoms.size();
  for ( int i = 0; i not_eq vecSize; ++i ){
    dataOut << absMoms[i] << "\t" << chi2values[i] << "\t" << ndfs[i] << "\n";
  }*/
}



