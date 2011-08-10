/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcMCmatching/CDCMCMatchingModule.h>

#include <framework/dataobjects/Relation.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <generators/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include "GFTrackCand.h"

#include <cstdlib>
#include <iomanip>
#include <string>
#include <boost/foreach.hpp>

#include <iostream>

using namespace std;
using namespace Belle2;

REG_MODULE(CDCMCMatching)

CDCMCMatchingModule::CDCMCMatchingModule() :
    Module()
{
  setDescription("Matches the GFTrackCandidates with MCTruth to evaluate the performance of pattern recognition. Assigns to each GFTrackCandidate an ID of the MCParticles which contributed the largest amount of hits to this track candidate.");

  //input
  addParam("MCParticlesColName", m_mcParticlesCollectionName, "Name of collection holding the MCParticles", string("MCParticles"));
  addParam("CDCRecoHitsColName", m_cdcRecoHitsCollectionName, "CDCRecoHits collection (should be created by CDCRecoHitMaker module)", string("CDCRecoHits"));
  addParam("MCParticlesToCDCRecoHitsColName", m_mcPartToCDCRecoHits, "Name of collection holding the relations between MCParticles and CDCRecoHits", string("MCParticleToCDCRecoHits"));

  addParam("GFTrackCandidatesColName", m_gfTrackCandsCollectionName, "Name of collection holding the GFTrackCandidates (output of the pattern recognition)", string("GFTrackCandidates_PatternReco"));

  //output
  addParam("GFTrackCandsToMCParticlesColName", m_gfTrackCandsToMCParticles, "Name of collection holding the relations between the GFTrackCandidates and the matched MCParticles (output of this module)", string("GFTrackCandidateToMCParticle"));


}

CDCMCMatchingModule::~CDCMCMatchingModule()
{
}

void CDCMCMatchingModule::initialize()
{

}

void CDCMCMatchingModule::beginRun()
{

}

void CDCMCMatchingModule::event()
{

  B2INFO("**********   CDCMCMatchingModule  ************");
  StoreArray<MCParticle> mcParticles(m_mcParticlesCollectionName);
  B2INFO("CDCMCMatching: total Number of MCParticles: " << mcParticles.GetEntries());
  if (mcParticles.GetEntries() == 0) B2WARNING("CDCMCMatching: MCParticlesCollection is empty!");

  StoreArray<CDCRecoHit> cdcRecoHits(m_cdcRecoHitsCollectionName);
  B2INFO("CDCMCMatching: Number of CDCRecoHits: " << cdcRecoHits.GetEntries());
  if (cdcRecoHits.GetEntries() == 0) B2WARNING("CDCMCMatching: CDCRecoHitsCollection is empty!");

  StoreArray<Relation> mcPartToCDCRecoHits(m_mcPartToCDCRecoHits);
  B2INFO("CDCMCMatching: Number of relations between MCParticles and CDCRecoHits: " << mcPartToCDCRecoHits.GetEntries());
  if (mcPartToCDCRecoHits.GetEntries() == 0) B2WARNING("CDCMCMatching: MCParticlesToCDCRecoHitsCollection is empty!");

  StoreArray<GFTrackCand> gfTrackCandidates(m_gfTrackCandsCollectionName);
  B2INFO("CDCMCMatching: Number of GFTrackCandidates: " << gfTrackCandidates.GetEntries());
  if (gfTrackCandidates.GetEntries() == 0) B2WARNING("CDCMCMatching: GFTrackCandidatesCollection is empty!");


  //Create a relation between the track candidate and their most probable 'mother' MC particle
  StoreArray<Relation> gfTrackCandToMCPart(m_gfTrackCandsToMCParticles);

  if (gfTrackCandidates.GetEntries() != 0) {
    for (int i = 0; i < gfTrackCandidates.GetEntries(); i++) { //loop over all TrackCandidates

      vector<unsigned int> cdcHitsIndexList = gfTrackCandidates[i]->GetHitIDs(2);   //indices of CDCRecoHits for the candidate
      vector <pair<int, int> > mcParticleContributions;  // vector to store pairs <MCParticleId, Number of Hits from this MCParticle>
      int bestMCId = -999; //index of the matched MCParticle

      B2INFO("Track Candidate " << i << " has " << cdcHitsIndexList.size() << " CDCRecoHits");

      //this is a complex loop, will maybe became easier with new relations...
      for (unsigned int j = 0; j < cdcHitsIndexList.size(); j++) { //loop over all RecoHits
        for (int k = 0; k < mcPartToCDCRecoHits.GetEntries(); k++) {

          BOOST_FOREACH(unsigned int mchit, mcPartToCDCRecoHits[k]->getToIndices()) {
            if (cdcHitsIndexList.at(j) == mchit) {

              addMCParticle(mcParticleContributions, mcPartToCDCRecoHits[k]->getFromIndex()); //add the index of the MCParticle to the pair vector
            }
          }
        }
      } //end loop over all RecoHits


      bestMCId = getBestMCId(mcParticleContributions); //evaluate the MCParticle with the largest contribution
      gfTrackCandidates[i]->setMcTrackId(bestMCId);    //assign the ID of this MCParticle to the candidate

      //create a relation between the track candidate and the MCParticle (redundat to the ID assignment)
      new(gfTrackCandToMCPart->AddrAt(i)) Relation(gfTrackCandidates, mcParticles, i, bestMCId);

      cdcHitsIndexList.clear();
      mcParticleContributions.clear();
    }
  }

}

void CDCMCMatchingModule::endRun()
{

}

void CDCMCMatchingModule::terminate()
{

}


void CDCMCMatchingModule::addMCParticle(vector <pair<int, int> >  & mcParticleContributions, int mcId)
{

  bool already = false;   //boolean to mark if this mcId is already there

  for (unsigned int i = 0; i < mcParticleContributions.size(); i++) { //loop over mcParticles which are already there

    if (mcParticleContributions.at(i).first == mcId) { //if there is already an entry for this mcParticleId
      mcParticleContributions.at(i).second = mcParticleContributions.at(i).second + 1;  //increase the number of hits from this mcParticle by 1
      already = true;
    }
  }// end loop over mcParticles

  if (already == false) {                 //if this is the first entry for this mcParticleId, create new entry
    pair <int, int> newEntry(mcId, 1);
    mcParticleContributions.push_back(newEntry);
  }
}

int CDCMCMatchingModule::getBestMCId(vector <pair<int, int> >  mcParticleContributions)
{
  {

    double max = 0;     //variable to mark the highest number of hits
    int indexMax = 0;   //variable to mart the index of the mcParticle which contributed the highest number of hits
    int bestMCId = -999;

    //Search for maximum
    for (unsigned int i = 0; i < mcParticleContributions.size(); i++) {
      if (mcParticleContributions.at(i).second > max) {
        max = mcParticleContributions.at(i).second;
        indexMax = i;
      }
    }

    //Assign results
    bestMCId = mcParticleContributions.at(indexMax).first;
    B2INFO("MCParticle with ID " << bestMCId << " contributed " << max << " hits to this track candidate");

    return bestMCId;


  }
}

