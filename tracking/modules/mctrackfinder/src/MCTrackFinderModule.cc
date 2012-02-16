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

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <generators/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/VxdID.h>

#include <GFTrackCand.h>

#include <boost/foreach.hpp>

#include <TRandom.h>

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
  //this is just an attempt to find out what is the most suitable way to select particles, if you have other/better ideas, communicate it to the tracking group...
  addParam("WhichParticles", m_whichParticles, "Select for which particles a track candidate should be created: 0 for primaries, 1 for all tracks which created hits in the PXD, 2 for all tracks which created hits in the SVD, 3 for all tracks which created hits in the CDC", int(0));
  addParam("EnergyCut", m_energyCut, "Track candidates are only created for MCParticles with energy larger than this cut ", double(0.1));
  addParam("Neutrals", m_neutrals, "Set true if track candidates should be created also for neutral particles", bool(true));

  //smearing of MCMomentum (integer value)
  addParam("Smearing", m_smearing, "Smearing of MCMomentum/MCVertex prior to storing it in GFTrackCandidate (in %). A negative value will switch off smearing. This is also the default.", -1.0);

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
  m_notEnoughtHitsCounter = 0;
}


void MCTrackFinderModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();
  B2INFO("*******   MCTrackFinderModule processing event number: " << eventCounter << " *******");

  //all the input containers. First: MCParticles
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  int nMcParticles = mcParticles.getEntries();
  B2DEBUG(149, "MCTrackFinder: total Number of MCParticles: " << nMcParticles);
  if (nMcParticles == 0) {B2INFO("MCTrackFinder: MCParticlesCollection is empty!");}

  //PXD
  StoreArray<PXDTrueHit> pxdTrueHits(m_pxdHitColName);
  int nPXDHits = pxdTrueHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of PXDHits: " << nPXDHits);
  if (nPXDHits == 0) {B2INFO("MCTrackFinder: PXDHitsCollection is empty!");}
  RelationArray mcPartToPXDTrueHits(mcParticles, pxdTrueHits);
  int nMcPartToPXDHits = mcPartToPXDTrueHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and PXDHits: " << nMcPartToPXDHits);
  if (nMcPartToPXDHits == 0) B2INFO("MCTrackFinder: MCParticlesToPXDHitsCollection is empty!");

  //SVD
  StoreArray<SVDTrueHit> svdTrueHits(m_svdHitColName);
  int nSVDHits = svdTrueHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of SVDDHits: " << nSVDHits);
  if (nSVDHits == 0) {B2INFO("MCTrackFinder: SVDHitsCollection is empty!");}
  RelationArray mcPartToSVDTrueHits(mcParticles, svdTrueHits);
  int nMcPartToSVDHits = mcPartToSVDTrueHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and SVDHits: " << nMcPartToSVDHits);
  if (nMcPartToSVDHits == 0) {B2INFO("MCTrackFinder: MCParticlesToSVDHitsCollection is empty!");}

  //CDC
  StoreArray<CDCHit> cdcHits(m_cdcHitColName);
  int nCDCHits = cdcHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of CDCHits: " << nCDCHits);
  if (nCDCHits == 0) {B2INFO("MCTrackFinder: CDCHitsCollection is empty!");}
  RelationArray mcPartToCDCHits(mcParticles, cdcHits);
  int nMcPartToCDCHits = mcPartToCDCHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between MCParticles and CDCHits: " << nMcPartToCDCHits);
  if (nMcPartToCDCHits == 0) {B2INFO("MCTrackFinder: MCParticlesToCDCHitsCollection is empty!");}
  StoreArray<CDCSimHit> cdcSimHits("");
  int nCDCSimHits = cdcSimHits.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of CDCHits: " << nCDCSimHits);
  if (nCDCSimHits == 0) {B2INFO("MCTrackFinder: CDCSimHitsCollection is empty!");}
  RelationArray cdcSimHitToHitRel(cdcSimHits, cdcHits);
  int nCdcSimHitToHitRel = cdcSimHitToHitRel.getEntries();
  B2DEBUG(149, "MCTrackFinder: Number of relations between CDCSimHit and CDCHits: " << nCdcSimHitToHitRel);
  if (nCdcSimHitToHitRel == 0) {B2INFO("MCTrackFinder: MCParticlesToCDCHitsCollection is empty!");}

  //register StoreArray which will be filled by this module
  StoreArray<GFTrackCand> trackCandidates(m_gfTrackCandsColName);
  RelationArray gfTrackCandToMCPart(trackCandidates, mcParticles);

  //set the proper status
  int status = 0;
  if (m_whichParticles == 0) {status = MCParticle::c_PrimaryParticle;}   //primaries
  if (m_whichParticles == 1) {status = MCParticle::c_SeenInPXD;}  //seen in PXD
  if (m_whichParticles == 2) {status = MCParticle::c_SeenInSVD;} //seen in SVD
  if (m_whichParticles == 3) {status = MCParticle::c_SeenInCDC;}//seen in CDC
  if (m_whichParticles > 3 || m_whichParticles < 0) {
    B2WARNING("Invalid parameter! Track Candidates for primary particles will be created.");
    status = 1;
  }
  //an auxiliary variable to discard neutrals if necessary (assume that no particles with charge -999 exist)
  float forbiddenCharge = -999;
  if (m_neutrals == false) {forbiddenCharge = 0;}

  // loop over MCParticles.
  // it would be nice to optimize this, because there are actually ~1000 secondary MCParticles for each primary MCParticle
  for (int iPart = 0; iPart < nMcParticles; ++iPart) {
    //make links only for interesting MCParticles: energy cut, which subdetector was reached and a 'dirty hack' to avoid atoms which are unknown to GenFit and check for neutrals
    if (mcParticles[iPart]->getEnergy() > m_energyCut && mcParticles[iPart]->hasStatus(status) == true && abs(mcParticles[iPart]->getPDG()) < 100000000 && mcParticles[iPart]->getCharge() != forbiddenCharge) {
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
      // create a list containing the indices to the CDCHits that belong to one track
      vector<int> cdcHitsIndices;
      for (int i = 0; i < nMcPartToCDCHits; ++i) {
        if (mcPartToCDCHits[i].getFromIndex() == unsigned(iPart)) {
          for (unsigned int j = 0; j < mcPartToCDCHits[i].getToIndices().size(); j++) {
            cdcHitsIndices.push_back(mcPartToCDCHits[i].getToIndex(j));
          }
        }
      }
      if (pxdHitsIndices.size() + svdHitsIndices.size() + cdcHitsIndices.size() < 3) {
        ++m_notEnoughtHitsCounter; // do not try do make a track candidate
      } else {
        //Now create TrackCandidate
        int counter = trackCandidates->GetLast() + 1;
        B2DEBUG(100, "Create TrackCandidate  " << counter);

        //create TrackCandidate
        new(trackCandidates->AddrAt(counter)) GFTrackCand();

        //set track parameters from MCParticle information
        TVector3 positionTrue = mcParticles[iPart]->getProductionVertex();
        TVector3 momentumTrue = mcParticles[iPart]->getMomentum();
        int pdg = mcParticles[iPart]->getPDG();

        //it may have positive effect on the fit not to start with exactly precise true values (or it may be just interesting to study this)
        //one can smear the starting momentum values with a gaussian
        //this calculation is always performed, but with the default value of m_smearing = 0 it has no effect on momentum and position (true values are taken)
        TVector3 momentum;
        TVector3 position;
        if (m_smearing > 0.0) {
          double smearing = m_smearing / 100.0;  //the module parameter m_smearing goes from 0 to 100, smearing should go from 0 to 1

          double smearedPX = gRandom->Gaus(momentumTrue.x(), smearing * momentumTrue.x());
          double smearedPY = gRandom->Gaus(momentumTrue.y(), smearing * momentumTrue.y());
          double smearedPZ = gRandom->Gaus(momentumTrue.z(), smearing * momentumTrue.z());
          momentum.SetXYZ(smearedPX, smearedPY, smearedPZ);

          double smearedX = gRandom->Gaus(positionTrue.x(), smearing * positionTrue.x());
          double smearedY = gRandom->Gaus(positionTrue.y(), smearing * positionTrue.y());
          double smearedZ = gRandom->Gaus(positionTrue.z(), smearing * positionTrue.z());
          position.SetXYZ(smearedX, smearedY, smearedZ);
        } else {
          position = positionTrue;
          momentum = momentumTrue;
        }
        //Errors for the position/momentum values can also be passed to GFTrackCandidate
        //Default values in Genfit are (1.,1.,1.,), they seem to be not good!!
        //The best way to set the 'correct' errors has to be investigated....
        TVector3 posError;
        posError.SetXYZ(1.0, 1.0, 2.0);
        TVector3 momError;
        momError.SetXYZ(0.1, 0.1, 0.2);

        //Finally set the complete track seed
        trackCandidates[counter]->setComplTrackSeed(position, momentum, pdg, posError, momError);

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
            float time = pxdTrueHits[hitID]->getGlobalTime();
            VxdID aVXDId = VxdID(sensorID);
            int uniqueSensorId = aVXDId.getID();
            trackCandidates[counter]->addHit(0, hitID, double(time), uniqueSensorId);
          }
          B2INFO("     add " << pxdHitsIndices.size() << " PXDHits");
        }
        if (m_useSVDHits) {
          BOOST_FOREACH(int hitID, svdHitsIndices) {
            int sensorID = svdTrueHits[hitID]->getSensorID();
            float time = svdTrueHits[hitID]->getGlobalTime();
            VxdID aVXDId = VxdID(sensorID);
            int uniqueSensorId = aVXDId.getID();
            //addHit(detectorID, hitID, rho (distance from the origin to sort hits), planeId (Id of the sensor, needed for DAF))
            trackCandidates[counter]->addHit(1, hitID, double(time), uniqueSensorId);
          }
          B2INFO("     add " << svdHitsIndices.size() << " SVDHits");
        }
        if (m_useCDCHits) {
          int layerId = -999;  //absolute layerId of the hit (from 0 to 55)
          double time = -1.0;  // global time of flight when hit was created

          BOOST_FOREACH(int hitID, cdcHitsIndices) {
            //calculate the layerId from information stored in the CDCHit
            if (cdcHits[hitID]->getISuperLayer() == 0) layerId = cdcHits[hitID]->getILayer();
            else layerId = 8 + (cdcHits[hitID]->getISuperLayer() - 1) * 6 + cdcHits[hitID]->getILayer();
            //for the DAF algorithm within GenFit it is important to assign a planeId to each hit
            //one can choose the layerId as the planeId, this would mean that hits from the same layer will 'compete' to be the 'best matching hit' in this layer
            //one can also give to each hit a unique planeId, so that e.g. two correct hits in the same layer get similar weights (without 'competition')
            //I am still not quite sure which way is the best one, this has to be tested...
            int uniqueId = layerId * 10000 + cdcHits[hitID]->getIWire();
            //set the time as the ordering parameter rho for genfit to do this search for any CDCSimHit that corresponds to the CDCHit and take the time from there
            for (int j = 0; j != nCdcSimHitToHitRel; ++j) {
              if (unsigned(hitID) == cdcSimHitToHitRel[j].getToIndex(0)) {
                time = cdcSimHits[cdcSimHitToHitRel[j].getFromIndex()]->getFlightTime();
              }
            }
            trackCandidates[counter]->addHit(2, hitID, time, uniqueId);
          }
          B2INFO("    add " << cdcHitsIndices.size() << " CDCHits");
        }
        // now after all the hits belonging to one track are added to a track candidate
        // bring them into the right order inside the trackCand objects using the rho parameter
        //trackCandidates[counter]->sortHits(); // this is not yet present in the genfit version used by basf2. After an updaet of the externals it can be used
      } //endif
    }
  }//end loop over MCParticles
}

void MCTrackFinderModule::endRun()
{
  if (m_notEnoughtHitsCounter != 0) {
    B2WARNING(m_notEnoughtHitsCounter << " tracks had 2 or less hits. No Track Candidates were created from them so they will not be passed to the track fitter");
  }
}

void MCTrackFinderModule::terminate()
{
}
