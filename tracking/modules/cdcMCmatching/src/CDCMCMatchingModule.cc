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
#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>

#include <framework/logging/Logger.h>

#include <generators/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCHit.h>
#include "GFTrackCand.h"

#include "framework/gearbox/Const.h"

#include <cstdlib>
#include <string>


using namespace std;
using namespace Belle2;

REG_MODULE(CDCMCMatching)

CDCMCMatchingModule::CDCMCMatchingModule() :
  Module()
{
  setDescription("Matches the GFTrackCandidates with MCTruth to evaluate the performance of pattern recognition. Assigns to each GFTrackCandidate an ID of the MCParticles which contributed the largest amount of hits to this track candidate.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  //the collection names as parameters may soon be obsolete if we will always use the default names created automatically

  //input
  addParam("MCParticlesColName", m_mcParticlesCollectionName, "Name of collection holding the MCParticles", string(""));
  addParam("CDCHitsColName", m_cdcHitsCollectionName, "CDCHits collection ", string(""));
  addParam("MCParticlesToCDCHitsColName", m_mcPartToCDCHits, "Name of collection holding the relations between MCParticles and CDCHits", string(""));

  addParam("GFTrackCandidatesColName", m_gfTrackCandsCollectionName, "Name of collection holding the GFTrackCandidates (output of the pattern recognition)", string("GFTrackCands_PatternReco"));

}

CDCMCMatchingModule::~CDCMCMatchingModule()
{
}

void CDCMCMatchingModule::initialize()
{
  StoreArray<GFTrackCand>::required(m_gfTrackCandsCollectionName);
  RelationArray::registerPersistent<GFTrackCand, MCParticle>(m_gfTrackCandsCollectionName, m_mcParticlesCollectionName);

}

void CDCMCMatchingModule::beginRun()
{

}

void CDCMCMatchingModule::event()
{

  B2INFO("**********   CDCMCMatchingModule  ************");
  StoreArray<MCParticle> mcParticles(m_mcParticlesCollectionName);
  B2DEBUG(149, "CDCMCMatching: total Number of MCParticles: " << mcParticles.getEntries());
  if (mcParticles.getEntries() == 0) B2WARNING("CDCMCMatching: MCParticlesCollection is empty!");

  StoreArray<CDCHit> cdcHits(m_cdcHitsCollectionName);
  B2DEBUG(149, "CDCMCMatching: Number of CDCHits: " << cdcHits.getEntries());
  if (cdcHits.getEntries() == 0) B2WARNING("CDCMCMatching: CDCHitsCollection is empty!");

  RelationArray mcPartToCDCHits(mcParticles, cdcHits);
  B2INFO("CDCMCMatching: Number of Relations between MCParticles and CDCHits: " << mcPartToCDCHits.getEntries());
  if (mcPartToCDCHits.getEntries() == 0) B2WARNING("CDCMCMatching: MCParticlesToCDCHitsCollection is empty!");


  StoreArray<GFTrackCand> gfTrackCandidates(m_gfTrackCandsCollectionName);
  B2INFO("CDCMCMatching: Number of GFTrackCandidates: " << gfTrackCandidates.getEntries());
  if (gfTrackCandidates.getEntries() == 0) B2WARNING("CDCMCMatching: GFTrackCandidatesCollection is empty!");


  //Create a relation between the track candidate and their most probable 'mother' MC particle
  RelationArray gfTrackCandToMCPart(gfTrackCandidates, mcParticles);

  for (int i = 0; i < gfTrackCandidates.getEntries(); i++) { //loop over all TrackCandidates

    vector<int> cdcHitsIndexList = gfTrackCandidates[i]->getHitIDs(Const::CDC);   //indices of CDCHits for the candidate
    vector <pair<int, int> > mcParticleContributions;  // vector to store pairs <MCParticleId, Number of Hits from this MCParticle>
    pair <int, float> bestMCId(-999, 0.0); //index and the hit fraction of the matched MCParticle

    B2INFO("Track Candidate " << i << " has " << cdcHitsIndexList.size() << " CDCHits");

    //now we have to find out which MCParticle created these hits
    for (unsigned int j = 0; j < cdcHitsIndexList.size(); j++) { //loop over all Hits

      for (int k = 0; k < mcPartToCDCHits.getEntries(); k++) {   //loop over all MCParticle<->CDCHit relations

        for (unsigned int mchit = 0; mchit < mcPartToCDCHits[k].getToIndices().size(); mchit++) { //loop over all hits to which the MCParticle points

          if (cdcHitsIndexList.at(j) == (int)mcPartToCDCHits[k].getToIndex(mchit)) {         //if the hit to which this MCParticle points is in the list

            addMCParticle(mcParticleContributions, mcPartToCDCHits[k].getFromIndex()); //add the index of the MCParticle to the pair vector

          }
        }
      }
    } //end loop over all Hits

    bestMCId = getBestMCId(mcParticleContributions, cdcHitsIndexList.size()); //evaluate the MCParticle with the largest contribution

    gfTrackCandidates[i]->setMcTrackId(bestMCId.first);    //assign the ID of this MCParticle to the candidate
    //setDip will not exist anymore very soon so I (Moritz) commented it out
    //gfTrackCandidates[i]->setDip(bestMCId.second);         //here I just 'misuse' one unused member variable from GFTrackCand called 'Dip' to store the 'purity' of the track

    //check if there is an MCParticle contributing to the track (-999 as ID means its random composition of background hits)
    if (bestMCId.first != -999) {
      B2INFO("Assign MCId " << bestMCId.first << " (pdg: " << mcParticles[bestMCId.first]->getPDG() << ") to track candidate " << i);
      //create a relation between the track candidate and the MCParticle (redundant to the ID assignment, but may however be useful)
      gfTrackCandToMCPart.add(i, bestMCId.first);
    } else B2WARNING("No MCParticle contributed to this track!");

    cdcHitsIndexList.clear();
    mcParticleContributions.clear();
  } //end loop over all track candidates

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

pair<int, float> CDCMCMatchingModule::getBestMCId(vector <pair<int, int> >  mcParticleContributions, int nHits)
{
  {

    double max = 0;     //variable to mark the highest number of hits
    int indexMax = 0;   //variable to mart the index of the mcParticle which contributed the highest number of hits
    int bestMCId = -999;
    float fraction = 0;
    pair <int, float> result(bestMCId, fraction);

    //Search for maximum
    for (unsigned int i = 0; i < mcParticleContributions.size(); i++) {
      if (mcParticleContributions.at(i).second > max) {
        max = mcParticleContributions.at(i).second;
        indexMax = i;
      }
    }

    if (mcParticleContributions.size() > 0) { //check if there are contributions at all
      //Assign results
      bestMCId = mcParticleContributions.at(indexMax).first;

      fraction = 100.0 * float(max) / float(nHits);

      B2INFO("--- MCParticle with ID " << bestMCId << " contributed " << max << " hits ( " << fraction << "% ) ");

      result.first = bestMCId ;
      result.second = fraction;
    } else {
      B2INFO("--- No MCParticle contributed to this track! (missing relations?)");
      result.first = -999;
      result.second = 0.0;
    }

    return result;


  }
}




