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
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <generators/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>

#include <GFTrackCand.h>

#include <boost/foreach.hpp>

#include <TRandom.h>

#include <utility>
#include <list>
#include <cmath>


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
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //Parameter definition
  addParam("UseClusters", m_useClusters, "Set true if you want to use PXD/SVD clusters instead of PXD/SVD trueHits", bool(false));

  //choose which hits to use, all hits assigned to the track candidate will be used in the fit
  addParam("UsePXDHits", m_usePXDHits, "Set true if PXDHits or PXDClusters should be used", bool(true));
  addParam("UseSVDHits", m_useSVDHits, "Set true if SVDHits or SVDClusters should be used", bool(true));
  addParam("UseCDCHits", m_useCDCHits, "Set true if CDCHits should be used", bool(true));

  addParam("MinimalHitNumber", m_numberOfHits, "Set the smallest number of hits a track must have to create a track candidate", 3);

  //choose for which particles a track candidate should be created
  //this is just an attempt to find out what is the most suitable way to select particles, if you have other/better ideas, communicate it to the tracking group...
  addParam("WhichParticles", m_whichParticles, "List of keywords to mark what properties particles must have to get a track candidate. If several properties are given all of them must be true: \"primary\" particle must come from the generator, \"PXD\" particle must have hits in PXD, \"SVD\" particle must have hits in SVD, \"CDC\" particle must have hits in CDC", vector<string>(1, "primary"));
  addParam("EnergyCut", m_energyCut, "Track candidates are only created for MCParticles with energy larger than this cut ", double(0.1));
  addParam("Neutrals", m_neutrals, "Set true if track candidates should be created also for neutral particles", bool(false));

  //smearing of MCMomentum
  addParam("Smearing", m_smearing, "Smearing of MCMomentum/MCVertex prior to storing it in GFTrackCandidate (in %). A negative value will switch off smearing. This is also the default.", -1.0);

  // names of output containers
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName, "Name of collection holding the GFTrackCandidates (output)", string(""));
}


MCTrackFinderModule::~MCTrackFinderModule()
{

}


void MCTrackFinderModule::initialize()
{
  //output store arrays have to be registered in initialize()
  StoreArray<GFTrackCand>::registerPersistent(m_gfTrackCandsColName);
  RelationArray::registerPersistent<GFTrackCand, MCParticle>(m_gfTrackCandsColName, "");



  m_particleProperties = 0;
  const int nProperties = m_whichParticles.size();
  for (int i = 0; i not_eq nProperties; ++i) {
    if (m_whichParticles[i] == "primary") {
      m_particleProperties += 1;
    } else if (m_whichParticles[i] == "PXD") {
      m_particleProperties += 2;
    } else if (m_whichParticles[i] == "SVD") {
      m_particleProperties += 4;
    } else if (m_whichParticles[i] == "CDC") {
      m_particleProperties += 8;
    } else {
      B2FATAL("Invalid values were given to the MCTrackFinder parameter WhichParticles");
    }
  }


}

void MCTrackFinderModule::beginRun()
{
  m_notEnoughtHitsCounter = 0;
  m_noTrueHitCounter = 0;
}


void MCTrackFinderModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(100, "*******   MCTrackFinderModule processing event number: " << eventCounter << " *******");

  //all the input containers. First: MCParticles
  StoreArray<MCParticle> mcParticles;
  const int nMcParticles = mcParticles.getEntries();
  B2DEBUG(100, "MCTrackFinder: total Number of MCParticles: " << nMcParticles);

  //PXD trueHits
  StoreArray<PXDTrueHit> pxdTrueHits;
  const int nPXDHits = pxdTrueHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of PXDTrueHits: " << nPXDHits);

  RelationArray mcPartToPXDTrueHits(mcParticles, pxdTrueHits);
  const int nMcPartToPXDHits = mcPartToPXDTrueHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between MCParticles and PXDHits: " << nMcPartToPXDHits);

  RelationIndex<MCParticle, PXDTrueHit> relMcPxdTrueHit;

  //PXD clusters
  StoreArray<PXDCluster> pxdClusters;
  const int nPXDClusters = pxdClusters.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of PXDClusters: " << nPXDClusters);

  RelationArray pxdClusterToMCParticle(pxdClusters, mcParticles);
  const int nPxdClusterToMCPart = pxdClusterToMCParticle.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between PXDCluster and MCParticles: " << nPxdClusterToMCPart);

  //SVD truehits
  StoreArray<SVDTrueHit> svdTrueHits;
  const int nSVDHits = svdTrueHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of SVDDHits: " << nSVDHits);

  RelationArray mcPartToSVDTrueHits(mcParticles, svdTrueHits);
  const int nMcPartToSVDHits = mcPartToSVDTrueHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between MCParticles and SVDHits: " << nMcPartToSVDHits);

  RelationIndex<MCParticle, SVDTrueHit> relMcSvdTrueHit;

  //SVD clusters
  StoreArray<SVDCluster> svdClusters;
  const int nSVDClusters = svdClusters.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of SVDClusters: " << nSVDClusters);

  RelationArray svdClusterToMCParticle(svdClusters, mcParticles);
  const int nSvdClusterToMCPart = svdClusterToMCParticle.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between SVDCluster and MCParticles: " << nSvdClusterToMCPart);

  //CDC
  StoreArray<CDCHit> cdcHits;
  const int nCDCHits = cdcHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of CDCHits: " << nCDCHits);

  RelationArray mcPartToCDCHits(mcParticles, cdcHits);
  const int nMcPartToCDCHits = mcPartToCDCHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between MCParticles and CDCHits: " << nMcPartToCDCHits);

  StoreArray<CDCSimHit> cdcSimHits("");
  const int nCDCSimHits = cdcSimHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of CDCSimHits: " << nCDCSimHits);

  RelationArray cdcSimHitToHitRel(cdcSimHits, cdcHits);
  const int nCdcSimHitToHitRel = cdcSimHitToHitRel.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between CDCSimHit and CDCHits: " << nCdcSimHitToHitRel);


  //register StoreArray which will be filled by this module
  StoreArray<GFTrackCand> trackCandidates(m_gfTrackCandsColName);
  trackCandidates.create();
  RelationArray gfTrackCandToMCPart(trackCandidates, mcParticles);

  //an auxiliary variable to discard neutrals if necessary (assume that no particles with charge -999 exist)
  float forbiddenCharge = -999;
  if (m_neutrals == false) {forbiddenCharge = 0;}

  // loop over MCParticles.
  // it would be nice to optimize this, because there are actually ~1000 secondary MCParticles for each primary MCParticle
  for (int iPart = 0; iPart < nMcParticles; ++iPart) {
    MCParticle* aMcParticlePtr = mcParticles[iPart];
    //set the property mask for this particle and compare it to the one gernated from user input
    int mcParticleProperties = 0;
    if (aMcParticlePtr->hasStatus(MCParticle::c_PrimaryParticle)) {
      mcParticleProperties += 1;
    }
    if (aMcParticlePtr->hasStatus(MCParticle::c_SeenInPXD)) {
      mcParticleProperties += 2;
    }
    if (aMcParticlePtr->hasStatus(MCParticle::c_SeenInSVD)) {
      mcParticleProperties += 4;
    }
    if (aMcParticlePtr->hasStatus(MCParticle::c_SeenInCDC)) {
      mcParticleProperties += 8;
    }
    // check all properties that the mcparticle should have in one line.
    if ((mcParticleProperties bitand m_particleProperties) != m_particleProperties) {
      B2DEBUG(100, "PDG: " << aMcParticlePtr->getPDG() <<  " actual properties " <<  mcParticleProperties << " demanded properties " << m_particleProperties);
      continue; //goto next mcParticle, do not make track candidate
    }
    //make links only for interesting MCParticles: energy cut, which subdetector was reached and a 'dirty hack' to avoid atoms which are unknown to GenFit and check for neutrals
    if (aMcParticlePtr->getEnergy() < m_energyCut ||  abs(aMcParticlePtr->getPDG()) > 100000000 || aMcParticlePtr->getCharge() == forbiddenCharge) {
      continue; //goto next mcParticle, do not make track candidate
    }

    B2DEBUG(100, "Search a  track for the MCParticle with index: " << iPart << " (PDG: " << aMcParticlePtr->getPDG() << ")");

    // create a list containing the indices to the PXDHits that belong to one track
    vector<int> pxdHitsIndices;
    if (m_useClusters == false) {
      for (int i = 0; i < nMcPartToPXDHits; ++i) {
        if (mcPartToPXDTrueHits[i].getFromIndex() == unsigned(iPart)) {
          for (unsigned int j = 0; j < mcPartToPXDTrueHits[i].getToIndices().size(); j++) {
            pxdHitsIndices.push_back(mcPartToPXDTrueHits[i].getToIndex(j));
          }
        }
      }
    } else {
      for (int i = 0; i < nPxdClusterToMCPart; ++i) {
        for (unsigned int j = 0; j < pxdClusterToMCParticle[i].getToIndices().size(); j++) {
          if (pxdClusterToMCParticle[i].getToIndex(j) == unsigned(iPart)) {
            pxdHitsIndices.push_back(pxdClusterToMCParticle[i].getFromIndex());
          }
        }
      }
    }

    // create a list containing the indices to the SVDHits that belong to one track
    vector<int> svdHitsIndices;
    if (m_useClusters == false) {
      for (int i = 0; i < nMcPartToSVDHits; ++i) {
        if (mcPartToSVDTrueHits[i].getFromIndex() == unsigned(iPart)) {
          for (unsigned int j = 0; j < mcPartToSVDTrueHits[i].getToIndices().size(); j++) {
            svdHitsIndices.push_back(mcPartToSVDTrueHits[i].getToIndex(j));
          }
        }
      }
    } else {
      for (int i = 0; i < nSvdClusterToMCPart; ++i) {
        for (unsigned int j = 0; j < svdClusterToMCParticle[i].getToIndices().size(); j++) {
          if (svdClusterToMCParticle[i].getToIndex(j) == unsigned(iPart)) {
            svdHitsIndices.push_back(svdClusterToMCParticle[i].getFromIndex());
          }
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

    if (pxdHitsIndices.size() + svdHitsIndices.size() + cdcHitsIndices.size() < unsigned(m_numberOfHits)) {
      ++m_notEnoughtHitsCounter;
      continue; //goto next mcParticle, do not make track candidate
    }
    //Now create TrackCandidate
    int counter = trackCandidates->GetLast() + 1;
    B2DEBUG(100, "Create TrackCandidate  " << counter);

    //create TrackCandidate
    new(trackCandidates->AddrAt(counter)) GFTrackCand();

    //set track parameters from MCParticle information
    TVector3 positionTrue = aMcParticlePtr->getProductionVertex();
    TVector3 momentumTrue = aMcParticlePtr->getMomentum();
    int pdg = aMcParticlePtr->getPDG();

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
    B2DEBUG(100, " --- Create relation between GFTrackCand " << counter << " and MCParticle " << iPart);

    //member variable Dip is currently used to store the purity of the tracks, for MCTracks it is always 100 %
    trackCandidates[counter]->setDip(100);

    //assign indices of the Hits from all detectors, their are distinguishable by their DetID:
    // pxd 0
    //   svd 1
    //     cdc 2
    if (m_usePXDHits && m_useClusters == false) {
      BOOST_FOREACH(int hitID, pxdHitsIndices) {
        VxdID aVxdId = pxdTrueHits[hitID]->getSensorID();
        float time = pxdTrueHits[hitID]->getGlobalTime();
        trackCandidates[counter]->addHit(0, hitID, double(time), aVxdId.getID());
      }
      B2DEBUG(100, "     add " << pxdHitsIndices.size() << " PXDHits");
    }

    if (m_usePXDHits && m_useClusters) {
      RelationIndex<PXDCluster, PXDTrueHit> relPxdClusterTrueHit;
      BOOST_FOREACH(int hitID, pxdHitsIndices) {

        VxdID aVxdId = pxdClusters[hitID]->getSensorID();
        RelationIndex<PXDCluster, PXDTrueHit>::range_from iterPairCluTr = relPxdClusterTrueHit.getElementsFrom(pxdClusters[hitID]);
        if (iterPairCluTr.first == iterPairCluTr.second) { // there is not trueHit! trow away hit because there is no time information for sorting
          ++m_noTrueHitCounter;
          continue;
        }
        float time = -1;
        RelationIndex<MCParticle, PXDTrueHit>::range_from iterPairMcTr = relMcPxdTrueHit.getElementsFrom(aMcParticlePtr);
        while (iterPairCluTr.first != iterPairCluTr.second && time < 0) {// make sure only a true hit is taken that really comes from the current mcParticle. This must be carefully checked because several trueHits from different real tracks can be melted into one cluster
          while (iterPairMcTr.first != iterPairMcTr.second) {
            if (iterPairMcTr.first->to == iterPairCluTr.first->to) {
              time = iterPairCluTr.first->to->getGlobalTime();
              break;
            }
            ++iterPairMcTr.first;
          }
          ++iterPairCluTr.first;
        }

        trackCandidates[counter]->addHit(0, hitID, double(time), aVxdId.getID());
      }
      B2DEBUG(100, "     add " << pxdHitsIndices.size() << " PXDClusters");
    }
    if (m_useSVDHits && m_useClusters == false) {
      BOOST_FOREACH(int hitID, svdHitsIndices) {
        VxdID aVxdId = svdTrueHits[hitID]->getSensorID();
        float time = svdTrueHits[hitID]->getGlobalTime();
        trackCandidates[counter]->addHit(1, hitID, double(time), aVxdId.getID());
      }
      B2DEBUG(100, "     add " << svdHitsIndices.size() << " SVDHits");
    }
    if (m_useSVDHits && m_useClusters) {
      RelationIndex<SVDCluster, SVDTrueHit> relSvdClusterTrueHit;
      BOOST_FOREACH(int hitID, svdHitsIndices) {
        VxdID aVxdId = svdClusters[hitID]->getSensorID();
        RelationIndex<SVDCluster, SVDTrueHit>::range_from iterPairCluTr = relSvdClusterTrueHit.getElementsFrom(svdClusters[hitID]);
        if (iterPairCluTr.first == iterPairCluTr.second) { // there is not trueHit! trow away hit because there is no time information for sorting
          ++m_noTrueHitCounter;
          continue;
        }
        float time = -1;
        RelationIndex<MCParticle, SVDTrueHit>::range_from iterPairMcTr = relMcSvdTrueHit.getElementsFrom(aMcParticlePtr);
        while (iterPairCluTr.first != iterPairCluTr.second && time < 0) {// make sure only a true hit is taken that really comes from the current mcParticle. This must be carefully checked because several trueHits from different real tracks can be melted into one cluster
          while (iterPairMcTr.first != iterPairMcTr.second) {
            if (iterPairMcTr.first->to == iterPairCluTr.first->to) {
              time = iterPairCluTr.first->to->getGlobalTime();
              break;
            }
            ++iterPairMcTr.first;
          }
          ++iterPairCluTr.first;
        }
        trackCandidates[counter]->addHit(1, hitID, double(time), aVxdId.getID());
      }
      B2DEBUG(100, "     add " << svdHitsIndices.size() << " SVDClusters");
    }


    if (m_useCDCHits) {
      float time = -1;
      BOOST_FOREACH(int hitID, cdcHitsIndices) {
        //for the DAF algorithm within GenFit it is important to assign a planeId to each hit
        //I am still not quite sure which way is the best one, this has to be tested...
        int uniqueId = cdcHits[hitID]->getID();
        //set the time as the ordering parameter rho for genfit to do this search for any CDCSimHit that corresponds to the CDCHit and take the time from there
        for (int j = 0; j != nCdcSimHitToHitRel; ++j) {
          if (unsigned(hitID) == cdcSimHitToHitRel[j].getToIndex(0)) {
            time = cdcSimHits[cdcSimHitToHitRel[j].getFromIndex()]->getFlightTime();
          }
        }
        trackCandidates[counter]->addHit(2, hitID, time, uniqueId);
      }
      B2DEBUG(100, "    add " << cdcHitsIndices.size() << " CDCHits");
    }
    // now after all the hits belonging to one track are added to a track candidate
    // bring them into the right order inside the trackCand objects using the rho/time parameter
    //trackCandidates[counter]->Print();
    trackCandidates[counter]->sortHits();
    //trackCandidates[counter]->Print();

  }//end loop over MCParticles
}

void MCTrackFinderModule::endRun()
{
  if (m_notEnoughtHitsCounter != 0) {
    B2WARNING(m_notEnoughtHitsCounter << " tracks less than " << m_numberOfHits << " hits. No Track Candidates were created from them so they will not be passed to the track fitter");
  }
  if (m_noTrueHitCounter != 0) {
    B2WARNING(m_noTrueHitCounter << " cluster hits did not have a relation to a true hit and were therefore not included in a track candidate");
  }
}

void MCTrackFinderModule::terminate()
{
}
