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

#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/hitcdc/CDCSimHit.h>
#include <svd/dataobjects/SVDHit.h>
#include <pxd/dataobjects/PXDHit.h>

#include "GFTrackCand.h"
#include <cdc/geocdc/CDCGeometryPar.h>

#include <boost/foreach.hpp>
#include "TRandom3.h"

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
  addParam("MCParticlesColName", m_mcParticlesColName, "Name of collection holding the MCParticles", string("MCParticles"));
  //pxd specific
  addParam("PXDHitsColName", m_pxdHitColName, "Name of collection holding the PXDHits", string(DEFAULT_PXDHITS));
  addParam("MCParticlesToPXDHitsColName", m_mcParticleToPXDHits, "Name of collection holding the Relations  MCParticles->PXDHits", string(DEFAULT_PXDHITSREL));
  // svd specific
  addParam("SVDHitsColName", m_svdHitColName, "Name of collection holding the SVDHits", string(DEFAULT_SVDHITS));
  addParam("MCParticlesToSVDHitsColName", m_mcParticleToSVDHits, "Name of collection holding the Relations  MCParticles->SVDHits", string(DEFAULT_SVDHITSREL));

  // cdc specific
  addParam("MCParticlesToCDCSimHitsColName", m_mcParticleToCDCSimHits, "Name of collection holding the Relations  MCParticles->CDCSimHits", string("MCPartToCDCSimHits"));
  addParam("CDCSimHitsColName", m_cdcSimHitColName, "Name of collection holding the CDCSimHits", string("CDCSimHits"));
  addParam("CDCSimHitsToCDCHitsColName", m_cdcSimHitsToCDCHits, "Name of collection holding the Relations  CDCSimHits->CDCHits", string("SimHitToCDCHits"));
  addParam("CDCHitsColName", m_cdcHitColName, "Name of collection holding the CDCHits", string("CDCHits"));

  //choose which hits to use, all hits assigned to the track candidate will be used in the fit
  addParam("UsePXDHits", m_usePXDHits, "Set true if PXDHits should be used", bool(true));
  addParam("UseSVDHits", m_useSVDHits, "Set true if SVDHits should be used", bool(true));
  addParam("UseCDCHits", m_useCDCHits, "Set true if CDCHits should be used by", bool(true));
  addParam("UseOnlyAxial", m_onlyAxial, "Set it true if you want to use only axial CDCHits", bool(false));

  //smearing of MCMomentum (integer value)
  addParam("Smearing", m_smearing, "Smearing of MCMomentum prior to storing it in GFTrackCandidate (in %)", 0);

  // names of output containers
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName, "Name of collection holding the GFTrackCandidates (output)", string("GFTrackCandidates"));
  addParam("GFTrackCandToMCParticleColName", m_gfTrackCandToMCParticleColName, "Name of collection holding the relations between GFTrackCandidates and MCParticles (output)", string("GFTrackCandidateToMCParticle"));

}


MCTrackFinderModule::~MCTrackFinderModule()
{

}


void MCTrackFinderModule::initialize()
{

}

void MCTrackFinderModule::beginRun()
{
}


void MCTrackFinderModule::event()
{
  B2INFO("*******   MCTrackFinderModule  *******");
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  int nMcParticles = mcParticles.getEntries();
  B2INFO("MCTrackFinder: total Number of MCParticles: " << nMcParticles);
  if (nMcParticles == 0) B2WARNING("MCTrackFinder: MCParticlesCollection is empty!");


  //PXD
  StoreArray<PXDHit> pxdHits(m_pxdHitColName);
  int nPXDHits = pxdHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of PXDHits: " << nPXDHits);
  if (nPXDHits == 0) B2WARNING("MCTrackFinder: PXDHitsCollection is empty!");

  StoreArray<Relation>   mcParticleToPXDHits(m_mcParticleToPXDHits);
  int nMcPartToPXDHits = mcParticleToPXDHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between MCParticles and PXDHits: " << nMcPartToPXDHits);
  if (nMcPartToPXDHits == 0) B2WARNING("MCTrackFinder: MCParticlesToPXDHitsCollection is empty!");

  //SVD
  StoreArray<SVDHit> svdHits(m_svdHitColName);
  int nSVDHits = svdHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of SVDHits: " << nSVDHits);
  if (nSVDHits == 0) B2WARNING("MCTrackFinder: SVDHitsCollection is empty!");

  StoreArray<Relation>   mcParticleToSVDHits(m_mcParticleToSVDHits);
  int nMcPartToSVDHits = mcParticleToSVDHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between MCParticles and SVDHits: " << nMcPartToSVDHits);
  if (nMcPartToSVDHits == 0) B2WARNING("MCTrackFinder: MCParticlesToSVDHitsCollection is empty!");

  //CDC
  //cdcDigitizer currently do not provide the MCParticle->CDCHit relation like SVD or PXD, so other relations are needed
  StoreArray<CDCSimHit> cdcSimHits(m_cdcSimHitColName);
  int nCDCSimHits = cdcSimHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of CDCSimHits: " << nCDCSimHits);
  if (nCDCSimHits == 0) B2WARNING("MCTrackFinder: CDCSimHitsCollection is empty!");

  StoreArray<CDCHit> cdcHits(m_cdcHitColName);
  int nCDCHits = cdcHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of CDCHits: " << nCDCHits);
  if (nCDCHits == 0) B2WARNING("MCTrackFinder: CDCHitsCollection is empty!");

  StoreArray<Relation>   mcParticleToCDCSimHits(m_mcParticleToCDCSimHits);
  int nMcPartToCDCSimHits = mcParticleToCDCSimHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between MCParticles and CDCSimHits: " << nMcPartToCDCSimHits);
  if (nMcPartToCDCSimHits == 0) B2WARNING("MCTrackFinder: MCParticlesToCDCSimHitsCollection is empty!");

  StoreArray<Relation>   cdcSimHitsToCDCHits(m_cdcSimHitsToCDCHits);
  int nCDCSimHitsToCDCHits = cdcSimHitsToCDCHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between CDCSimHits and CDCHits: " << nCDCSimHitsToCDCHits);
  if (nMcPartToCDCSimHits == 0) B2WARNING("MCTrackFinder: CDCSimHitsToCDCHitsCollection is empty!");


  //!!!!! should be moved to initialize in the newest framework version
  //register StoreArray which will be filled by this module
  StoreArray<GFTrackCand> trackCandidates(m_gfTrackCandsColName);
  StoreArray<Relation> trackCandsToMCParticles(m_gfTrackCandToMCParticleColName);


  //Temporary solution
  //This relation should be provided by the CDCDigi module in the future, but as I need it now for the MCMatching I create it here...
  //-------------------------------------------------------------------------------
  StoreArray<Relation> mcParticleToCDCHits("MCParticleToCDCHits");

  int testcounter = -1;
  for (int iPart = 0; iPart < nMcParticles; ++iPart) {
    list<int> cdcSimHitsIndices;;
    list<int> cdcHitsIndices;

    for (int i = 0; i < mcParticleToCDCSimHits->GetEntries(); i++) {
      if (mcParticleToCDCSimHits[i]->getFromIndex() == iPart) {
        cdcSimHitsIndices.push_back(mcParticleToCDCSimHits[i]->getToIndex());
      }
    }
    BOOST_FOREACH(int hit, cdcSimHitsIndices) {
      for (int j = 0; j < nCDCSimHitsToCDCHits; j++) {
        if (cdcSimHitsToCDCHits[j]->getFromIndex() == hit) {
          cdcHitsIndices.push_back(cdcSimHitsToCDCHits[j]->getToIndex());
        }
      }

    }
    if (cdcHitsIndices.size() > 0) {
      testcounter++;
      new(mcParticleToCDCHits->AddrAt(testcounter)) Relation(mcParticles, cdcHits, iPart, cdcHitsIndices);
    }

  }
//-----------------------------------------------------------------------------------


  // loop over MCParticles.
  // it would be nice to optimize this, because there are actually ~1000 secondary MCParticles for each primary MCParticle
  for (int iPart = 0; iPart < nMcParticles; ++iPart) {

    //make links only for interesting MCParticles, for the moment take only primary particles
    if (mcParticles[iPart]->hasStatus(1) == true) {
      B2INFO("Search a  track for the MCParticle with index: " << iPart << " (PDG: " << mcParticles[iPart]->getPDG() << ")");


      // create a list containing the indices to the PXDHits that belong to one track
      list<int> pxdHitsIndices;
      for (int i = 0; i < nMcPartToPXDHits; ++i) {
        if (mcParticleToPXDHits[i]->getFromIndex() == iPart) {
          int pxdHitIndex = mcParticleToPXDHits[i]->getToIndex();
          pxdHitsIndices.push_back(pxdHitIndex);
        }
      }

      // create a list containing the indices to the SVDHits that belong to one track
      list<int> svdHitsIndices;
      for (int i = 0; i < nMcPartToSVDHits; ++i) {
        if (mcParticleToSVDHits[i]->getFromIndex() == iPart) {
          int svdHitIndex = mcParticleToSVDHits[i]->getToIndex();
          svdHitsIndices.push_back(svdHitIndex);
        }
      }



      // create a list containing the indices to the CDCHits that belong to one track
      // this is slightly complicated at the moment than PXD and SVD
      list<int> cdcSimHitsIndices;;
      list<int> cdcHitsIndices;

      for (int i = 0; i < mcParticleToCDCSimHits->GetEntries(); i++) {
        if (mcParticleToCDCSimHits[i]->getFromIndex() == iPart) {
          cdcSimHitsIndices.push_back(mcParticleToCDCSimHits[i]->getToIndex());
        }
      }

      BOOST_FOREACH(int hit, cdcSimHitsIndices) {
        for (int j = 0; j < nCDCSimHitsToCDCHits; j++) {
          if (cdcSimHitsToCDCHits[j]->getFromIndex() == hit) {
            cdcHitsIndices.push_back(cdcSimHitsToCDCHits[j]->getToIndex());
          }
        }

      }

      //Now create TrackCandidate
      int counter = trackCandidates->GetLast() + 1;
      B2DEBUG(100, "Create TrackCandidate  " << counter);

      //create TrackCandidate
      new(trackCandidates->AddrAt(counter)) GFTrackCand();

      //set the values needed as start values for the fit in the GFTrackCandidate from the MCParticle information
      //variables stored in the GFTrackCandidates are: vertex position, momentum, pdg value, indices for the Hits
      //the Id of the MCParticle is also stored

      TVector3 position = mcParticles[iPart]->getProductionVertex();
      TVector3 momentum = mcParticles[iPart]->getMomentum();
      int pdg = mcParticles[iPart]->getPDG();

      //it may have negative effects on the fit to start with exactly precise true values
      //one can smear the starting momentum values with a gaussian
      TRandom3 *random = new TRandom3((UInt_t)12345);
      TVector3 smearedMomentum;
      double smearing = double(m_smearing) / 100;

      double smearedPX = random->Gaus(momentum.x(), smearing * momentum.x());
      double smearedPY = random->Gaus(momentum.y(), smearing * momentum.y());
      double smearedPZ = random->Gaus(momentum.z(), smearing * momentum.z());
      smearedMomentum.SetXYZ(smearedPX, smearedPY, smearedPZ);

      //Errors for the position/momentum values can also be passed to GFTrackCandidate
      //Default values in Genfit are (1.,1.,1.,)
      TVector3 posError;
      posError.SetXYZ(1.0, 1.0, 1.0);
      TVector3 momError;
      momError.SetXYZ(1.0, 1.0, 1.0);

      //Finally set the complete track seed
      trackCandidates[counter]->setComplTrackSeed(position, smearedMomentum, pdg, posError, momError);
      //Save the MCParticleID in the TrackCandidate
      trackCandidates[counter]->setMcTrackId(iPart);
      //create relation between the track candidates and the mcParticle (redundant to saving to MCId)
      new(trackCandsToMCParticles->AddrAt(counter)) Relation(trackCandidates, mcParticles, counter, iPart);
      B2INFO(" --- Create relation between MCParticle " << iPart << "   and Track Candidate " << counter);

      //member variable Dip is currently used to store the purity of the tracks, for MCTrack it is always 100 %
      trackCandidates[counter]->setDip(100);

      //before assigning the Hits to the trackCandidate some additional geometry information is needed
      //to use DAF each hit has to have a plane ID, in case of CDC the layerID can do the job, in case of SVD/PXD the ladderID (it was however not thouroughfully checked yet...)
      //for correct hit order the parameter rho is important, it is calculaed as the distance from the origin for the CDCHits, for SVD/PXD it is at the moment the layerID....
      CDCGeometryPar * cdcgp = CDCGeometryPar::Instance();
      CDCGeometryPar & cdcg(*cdcgp);

      //assign indices of the Hits from all detectors, their are distinguishable by their DetID:
      // pxd 0
      //   svd 1
      //     cdc 2
      if (m_usePXDHits) {
        BOOST_FOREACH(int hitID, pxdHitsIndices) {
          int aSensorUniID = pxdHits[hitID]->getSensorUniID();
          SensorUniIDManager aIdConverter(aSensorUniID);
          int ladderId = aIdConverter.getLadderID();
          int layerId = aIdConverter.getLayerID();

          trackCandidates[counter]->addHit(0, hitID, layerId, ladderId);
        }
        B2INFO("Add " << pxdHitsIndices.size() << " PXDHits to Track Candidate " << counter);
      }
      if (m_useSVDHits) {

        BOOST_FOREACH(int hitID, svdHitsIndices) {
          int aSensorUniID = svdHits[hitID]->getSensorUniID();
          SensorUniIDManager aIdConverter(aSensorUniID);
          int ladderId = aIdConverter.getLadderID();
          int layerId = aIdConverter.getLayerID();

          trackCandidates[counter]->addHit(1, hitID, layerId, ladderId);
        }
        B2INFO("Add " << svdHitsIndices.size() << " SVDHits to Track Candidate " << counter);
      }

      if (m_useCDCHits) {
        int layerId = -999;  //absolute layerId of the hit (from 0 to 55)
        float rho = -999.0;  //distance from the hit wire to the origin, needed to sort the hits
        BOOST_FOREACH(int hitID, cdcHitsIndices) {

          //calculate the layerId from information stored in the CDCHit
          if (cdcHits[hitID]->getISuperLayer() == 0) layerId = cdcHits[hitID]->getILayer();
          else layerId = 8 + (cdcHits[hitID]->getISuperLayer() - 1) * 6 + cdcHits[hitID]->getILayer();

          //calculate the distance to origin
          TVector3 distance = (cdcg.wireForwardPosition(layerId, cdcHits[hitID]->getIWire()) + cdcg.wireForwardPosition(layerId, cdcHits[hitID]->getIWire())) * 0.5;
          rho = distance.Mag();

          if (m_onlyAxial) {

            if (cdcHits[hitID]->getISuperLayer() % 2 == 0) {

              trackCandidates[counter]->addHit(2, hitID, rho, layerId);
            }
          } else {
            trackCandidates[counter]->addHit(2, hitID, rho, layerId);

          }
        }
        if (m_onlyAxial) B2INFO(" ....(use only axial CDCHits).... ");
        B2INFO("Add " << cdcHitsIndices.size() << " CDCHits to Track Candidate " << counter);
      }
      trackCandidates[counter]->sort();
    } //endif


  }//end loop over MCParticles


}

void MCTrackFinderModule::endRun()
{
}

void MCTrackFinderModule::terminate()
{
}

