/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/mctrackfinder/MCTrackFinderModule.h>

#include<framework/datastore/StoreArray.h>
#include<framework/datastore/RelationArray.h>

#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/VxdID.h>

#include <GFTrackCand.h>

#include <boost/foreach.hpp>
#include <TRandom3.h>

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
  setDescription("Uses the MC information to create GFTrackCandidates for primary MCParticles and Relations between them.  Fills the created GFTrackCandidates with all information (start values, hit indices) needed for the fitting.");

  //Parameter definition

  // names of input containers
  addParam("MCParticlesColName", m_mcParticlesColName, "Name of collection holding the MCParticles", string(""));
  //pxd specific
  addParam("PXDHitsColName", m_pxdHitColName, "Name of collection holding the PXDHits", string(""));
  addParam("MCParticlesToPXDHitsColName", m_mcParticleToPXDHits, "Name of collection holding the Relations  MCParticles->PXDHits", string(""));
  // svd specific
  addParam("SVDHitsColName", m_svdHitColName, "Name of collection holding the SVDHits", string(""));
  addParam("MCParticlesToSVDHitsColName", m_mcParticleToSVDHits, "Name of collection holding the Relations  MCParticles->SVDHits", string(""));

  // cdc specific
  addParam("CDCHitsColName", m_cdcHitColName, "Name of collection holding the CDCHits", string(""));
  addParam("MCParticlesToCDCHitsColName", m_mcParticleToCDCHits, "Name of collection holding the Relations  MCParticles->CDCHits", string(""));

  //choose which hits to use, all hits assigned to the track candidate will be used in the fit
  addParam("UsePXDHits", m_usePXDHits, "Set true if PXDHits should be used", bool(true));
  addParam("UseSVDHits", m_useSVDHits, "Set true if SVDHits should be used", bool(true));
  addParam("UseCDCHits", m_useCDCHits, "Set true if CDCHits should be used", bool(true));

  //choose for which particles a track candidate should be created
  addParam("WhichParticles", m_whichParticles, "Select for which particles a track candidate should be created: 0 for all primaries, 1 for those primaries who reach PXD, 2 for those primaries who reach SVD, 3 for those primaries who reach CDC", int(0));

  //smearing of MCMomentum (integer value)
  addParam("Smearing", m_smearing, "Smearing of MCMomentum/MCVertex prior to storing it in GFTrackCandidate (in %)", 0);

  // names of output containers
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName, "Name of collection holding the GFTrackCandidates (output)", string(""));
  addParam("GFTrackCandToMCParticleColName", m_gfTrackCandToMCParticleColName, "Name of collection holding the relations between GFTrackCandidates and MCParticles (output)", string(""));

}


MCTrackFinderModule::~MCTrackFinderModule()
{

}


void MCTrackFinderModule::initialize()
{

  StoreArray<GFTrackCand> trackCandidates(m_gfTrackCandsColName);
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  RelationArray gfTrackCandToMCPart(trackCandidates, mcParticles);

}

void MCTrackFinderModule::beginRun()
{
}


void MCTrackFinderModule::event()
{
  B2INFO("*******   MCTrackFinderModule  *******");
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  int nMcParticles = mcParticles.getEntries();
  B2DEBUG(149, "MCTrackFinder: total Number of MCParticles: " << nMcParticles);
  if (nMcParticles == 0) B2WARNING("MCTrackFinder: MCParticlesCollection is empty!");

  //PXD
  StoreArray<PXDTrueHit> pxdTrueHits(m_pxdHitColName);
  int nPXDHits = pxdTrueHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of PXDHits: " << nPXDHits);
  if (nPXDHits == 0) B2WARNING("MCTrackFinder: PXDHitsCollection is empty!");

  RelationArray mcPartToPXDTrueHits(mcParticles, pxdTrueHits);
  int nMcPartToPXDHits = mcPartToPXDTrueHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and PXDHits: " << nMcPartToPXDHits);
  if (nMcPartToPXDHits == 0) B2WARNING("MCTrackFinder: MCParticlesToPXDHitsCollection is empty!");


  //SVD
  StoreArray<SVDTrueHit> svdTrueHits(m_svdHitColName);
  int nSVDHits = svdTrueHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of SVDDHits: " << nSVDHits);
  if (nSVDHits == 0) B2WARNING("MCTrackFinder: SVDHitsCollection is empty!");

  RelationArray mcPartToSVDTrueHits(mcParticles, svdTrueHits);
  int nMcPartToSVDHits = mcPartToSVDTrueHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and SVDHits: " << nMcPartToSVDHits);
  if (nMcPartToSVDHits == 0) B2WARNING("MCTrackFinder: MCParticlesToSVDHitsCollection is empty!");


  //CDC
  StoreArray<CDCHit> cdcHits(m_cdcHitColName);
  int nCDCHits = cdcHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of CDCHits: " << nCDCHits);
  if (nCDCHits == 0) B2WARNING("MCTrackFinder: CDCHitsCollection is empty!");

  RelationArray mcPartToCDCHits(mcParticles, cdcHits);
  int nMcPartToCDCHits = mcPartToCDCHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and CDCHits: " << nMcPartToCDCHits);
  if (nMcPartToCDCHits == 0) B2WARNING("MCTrackFinder: MCParticlesToCDCHitsCollection is empty!");

  //register StoreArray which will be filled by this module
  StoreArray<GFTrackCand> trackCandidates(m_gfTrackCandsColName);
  //StoreArray<Relation> trackCandsToMCParticles(m_gfTrackCandToMCParticleColName);
  RelationArray gfTrackCandToMCPart(trackCandidates, mcParticles);

  //set the proper status
  int status = 0;
  if (m_whichParticles == 0) status = 1;   //primaries
  if (m_whichParticles == 1) status = 16;  //seen in PXD
  if (m_whichParticles == 2) status = 32;  //seen in SVD
  if (m_whichParticles == 3) status = 64;  //seen in CDC
  if (m_whichParticles > 3 || m_whichParticles < 0) {
    B2WARNING("Invalid parameter! Track Candidates for all primary particles will be created.")
    status = 1;
  }
  // loop over MCParticles.
  // it would be nice to optimize this, because there are actually ~1000 secondary MCParticles for each primary MCParticle
  for (int iPart = 0; iPart < nMcParticles; ++iPart) {
    //make links only for interesting MCParticles, for the moment: primaries seen in a subdetector
    if (mcParticles[iPart]->hasStatus(status) == true && mcParticles[iPart]->hasStatus(1) == true) {
      B2INFO("Search a  track for the MCParticle with index: " << iPart << " (PDG: " << mcParticles[iPart]->getPDG() << ")");

      // create a list containing the indices to the PXDHits that belong to one track
      vector<int> pxdHitsIndices;
      for (int i = 0; i < nMcPartToPXDHits; ++i) {
        if (mcPartToPXDTrueHits[i].getFromIndex() == unsigned(iPart)) {
          for (unsigned int j = 0; j < mcPartToPXDTrueHits[i].getToIndices().size(); j++) {
            pxdHitsIndices.push_back(mcPartToPXDTrueHits[i].getToIndex(j));
          }
        }
      }

      // create a list containing the indices to the SVDHits that belong to one track
      vector<int> svdHitsIndices;
      for (int i = 0; i < nMcPartToSVDHits; ++i) {
        if (mcPartToSVDTrueHits[i].getFromIndex() == unsigned(iPart)) {
          for (unsigned int j = 0; j < mcPartToSVDTrueHits[i].getToIndices().size(); j++) {
            svdHitsIndices.push_back(mcPartToSVDTrueHits[i].getToIndex(j));
          }
        }
      }

      // create a list containing the indices to the SVDHits that belong to one track
      vector<int> cdcHitsIndices;
      for (int i = 0; i < nMcPartToCDCHits; ++i) {
        if (mcPartToCDCHits[i].getFromIndex() == unsigned(iPart)) {
          for (unsigned int j = 0; j < mcPartToCDCHits[i].getToIndices().size(); j++) {
            cdcHitsIndices.push_back(mcPartToCDCHits[i].getToIndex(j));
          }
        }
      }


      //Now create TrackCandidate
      int counter = trackCandidates->GetLast() + 1;
      B2DEBUG(100, "Create TrackCandidate  " << counter);

      //create TrackCandidate
      new(trackCandidates->AddrAt(counter)) GFTrackCand();


      //before assigning the Hits to the trackCandidate some additional geometry information is needed
      CDCGeometryPar * cdcgPtr = NULL;
      if (m_useCDCHits) {
        cdcgPtr = CDCGeometryPar::Instance();
        // //CDCGeometryPar & cdcg(*cdcgPtr);  //cannot be used anymore (says Moritz)

        //set the values needed as start values for the fit in the GFTrackCandidate from the MCParticle information
        //variables stored in the GFTrackCandidates are: vertex position, momentum, pdg value, indices for the Hits
        //the Id of the MCParticle is also stored

        //an alternative: use as vertex position not the true position, but the coordinates of the first CDCHit (can be also done for SVD or PXD)
        /*
        int minLayerId = 999;
        int minIndex  =  999;
        for (unsigned int i = 0; i<cdcHitsIndices.size(); i++){
          if(cdcHits[i]->getILayer()<minLayerId){
             minLayerId = cdcHits[i]->getILayer();
             minIndex = i;
          }
        }

        TVector3 position = (cdcgPtr->wireForwardPosition(cdcHits[minIndex]->getILayer(), cdcHits[minIndex]->getIWire()) + cdcgPtr->wireBackwardPosition(cdcHits[minIndex]->getILayer(), cdcHits[minIndex]->getIWire())) * 0.5;
        */
      }
      //set track parameters from MCParticle information
      TVector3 position = mcParticles[iPart]->getProductionVertex();
      TVector3 momentum = mcParticles[iPart]->getMomentum();
      int pdg = mcParticles[iPart]->getPDG();

      //it may have positive effect on the fit not to start with exactly precise true values (or it may be just interesting to study this)
      //one can smear the starting momentum values with a gaussian
      //this calculation is always performed, but with the default value of m_smearing = 0 it has no effect on momentum and position (true values are taken)
      TRandom3 *random = new TRandom3((UInt_t)12345);
      TVector3 smearedMomentum;
      TVector3 smearedPosition;
      double smearing = double(m_smearing) / 100;  //the module parameter m_smearing goes from 0 to 100, smearing should go from 0 to 1

      double smearedPX = random->Gaus(momentum.x(), smearing * momentum.x());
      double smearedPY = random->Gaus(momentum.y(), smearing * momentum.y());
      double smearedPZ = random->Gaus(momentum.z(), smearing * momentum.z());
      smearedMomentum.SetXYZ(smearedPX, smearedPY, smearedPZ);

      double smearedX = random->Gaus(position.x(), smearing * position.x());
      double smearedY = random->Gaus(position.y(), smearing * position.y());
      double smearedZ = random->Gaus(position.z(), smearing * position.z());
      smearedPosition.SetXYZ(smearedX, smearedY, smearedZ);

      //Errors for the position/momentum values can also be passed to GFTrackCandidate
      //Default values in Genfit are (1.,1.,1.,), they seem to be not good!!
      //The best way to set the 'correct' errors has to be investigated....
      TVector3 posError;
      posError.SetXYZ(1.0, 1.0, 2.0);
      TVector3 momError;
      momError.SetXYZ(0.1, 0.1, 0.2);

      //Finally set the complete track seed
      trackCandidates[counter]->setComplTrackSeed(smearedPosition, smearedMomentum, pdg, posError, momError);

      //Save the MCParticleID in the TrackCandidate
      trackCandidates[counter]->setMcTrackId(iPart);

      //create relation between the track candidates and the mcParticle (redundant to saving the MCId)

      gfTrackCandToMCPart.add(counter, iPart);

      B2INFO(" --- Create relation between GFTrackCand " << counter << " and MCParticle " << iPart);

      //member variable Dip is currently used to store the purity of the tracks, for MCTracks it is always 100 %
      trackCandidates[counter]->setDip(100);


      //assign indices of the Hits from all detectors, their are distinguishable by their DetID:
      // pxd 0
      //   svd 1
      //     cdc 2
      if (m_usePXDHits) {

        BOOST_FOREACH(int hitID, pxdHitsIndices) {

          int sensorID = pxdTrueHits[hitID]->getSensorID();
          VxdID aVXDId = VxdID(sensorID);
          int ladderId = aVXDId.getLadder();
          int layerId = aVXDId.getLayer();
          //addHit(detectorID, hitID, rho (distance from the origin to sort hits), planeId (Id of the sensor, needed for DAF))
          trackCandidates[counter]->addHit(0, hitID, layerId, ladderId);

        }
        B2INFO("     add " << pxdHitsIndices.size() << " PXDHits");

      }
      if (m_useSVDHits) {

        BOOST_FOREACH(int hitID, svdHitsIndices) {
          int sensorID = svdTrueHits[hitID]->getSensorID();
          VxdID aVXDId = VxdID(sensorID);
          int ladderId = aVXDId.getLadder();
          int layerId = aVXDId.getLayer();
          //addHit(detectorID, hitID, rho (distance from the origin to sort hits), planeId (Id of the sensor, needed for DAF))
          trackCandidates[counter]->addHit(1, hitID, layerId, ladderId);
        }
        B2INFO("     add " << svdHitsIndices.size() << " SVDHits");

      }

      if (m_useCDCHits) {
        int layerId = -999;  //absolute layerId of the hit (from 0 to 55)
        float rho = -999.0;  //distance from the hit wire to the origin, needed to sort the hits
        BOOST_FOREACH(int hitID, cdcHitsIndices) {


          //calculate the layerId from information stored in the CDCHit
          if (cdcHits[hitID]->getISuperLayer() == 0) layerId = cdcHits[hitID]->getILayer();
          else layerId = 8 + (cdcHits[hitID]->getISuperLayer() - 1) * 6 + cdcHits[hitID]->getILayer();

          //for the DAF algorithm within GenFit it is important to assign a planeId to each hit
          //one can choose the layerId as the planeId, this would mean that hits from the same layer will 'compete' to be the 'best matching hit' in this layer
          //one can also give to each hit a unique planeId, so that e.g. two correct hits in the same layer get similar weights (without 'competition')
          //I am still not quite sure which way is the best one, this has to be tested...
          int uniqueId = layerId * 10000 + cdcHits[hitID]->getIWire();

          //calculate the distance to origin
          TVector3 distance = (cdcgPtr->wireForwardPosition(layerId, cdcHits[hitID]->getIWire()) + cdcgPtr->wireBackwardPosition(layerId, cdcHits[hitID]->getIWire())) * 0.5;
          rho = distance.Mag();

          trackCandidates[counter]->addHit(2, hitID, rho, uniqueId);


        }

        B2INFO("    add " << cdcHitsIndices.size() << " CDCHits");
      }


    } //endif


  }//end loop over MCParticles


}

void MCTrackFinderModule::endRun()
{
}

void MCTrackFinderModule::terminate()
{
}

