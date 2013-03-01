/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/simplebackground/CDCSimpleBackgroundModule.h>

#include <framework/datastore/StoreArray.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/CDCHit.h>

#include <time.h>
#include <climits>

using namespace std;
using namespace Belle2;
using namespace CDC;


REG_MODULE(CDCSimpleBackground)

CDCSimpleBackgroundModule::CDCSimpleBackgroundModule() : Module()
{
  setDescription("The CDCSimpleBackgroundModule generates random hits in the CDC (approximatly 1/r distribution) to emulate background. It adds digitized CDCHits to the CDCHits array.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("CDCHitsColName", m_cdcHitsColName, "Name of collection holding the digitized CDCHits, new CDCHits will be added to this array. Empty for default name.", string(""));
  addParam("BGLevelHits", m_hits, "Percentage of wires hit by background through single hits", 2.0);
  addParam("BGLevelClusters", m_clusters, "Percentage of wires hit by background through hit clusters (up to 7 hits each) ", 0.0);

}

CDCSimpleBackgroundModule::~CDCSimpleBackgroundModule()
{
}

void CDCSimpleBackgroundModule::initialize()
{
  //StoreArray with digitized CDCHits
  StoreArray<CDCHit>::registerPersistent(m_cdcHitsColName);

  //initialize random number generator
  //srand((unsigned int) 111); //to get reproducible results
  srand((unsigned int)time(0));   //to get new background in each run
}

void CDCSimpleBackgroundModule::beginRun()
{

}

void CDCSimpleBackgroundModule::event()
{
  B2INFO("*******  CDCSimpleBackgroundModule  *******");

  //StoreArray with digitized CDCHits
  StoreArray <CDCHit> cdcHits(m_cdcHitsColName);

  int NTrackHits = cdcHits.getEntries();    //hits from tracks
  int NBackgroundHits = 0;                  //number of background hits created by this module
  int NTotalHits = NTrackHits + NBackgroundHits;
  B2DEBUG(149, "CDCSimpleBackground: Number of digitized CDCHits coming from tracks: " << NTrackHits);
  if (NTrackHits == 0) B2WARNING("CDCSimpleBackground: CDCHitsCollection is empty!");

  int counter = cdcHits.getEntries(); //counter for the additional hits

  //cdc geometry parameters information
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();


  //initialize variables which describe a CDCHit
  int superlayerId = -999;        //0 - 8
  int ilayerId = -999;            //layerID within a superlayer 0 - 7
  int wireId = -999;
  unsigned short charge = USHRT_MAX; //aka ADC count
  unsigned short driftTime = USHRT_MAX; //aka TDC count

  //CDC geometry information
  int nLayers = cdcg.nWireLayers();
  int nWires = 0; //I found no direct getter for this, but it can be calculated
  for (int i = 0; i < nLayers; i++) {
    nWires += cdcg.nWiresInLayer(i);
  }

  //Information about the possible values of charge/drift time. This is the first guess!!! I have no real idea in what range these values should be (and what their units are).
  //It also not clear if background hits will have similar drift time and charge as real hits, so it is really a very rough approach...
  double maxDriftTime = USHRT_MAX;
  double maxCharge = USHRT_MAX;

  //Check if the parameters are valid
  if (m_hits < 0) B2WARNING("The parameter BGLevelHits should lie between 0 and 100, the input parameter is negative (" << m_hits << "), no background hits will be generated");

  if (m_hits > 100) {
    B2WARNING("The parameter BGLevelHits should lie between 0 and 100, The input parameter is larger than 100 (" << m_hits << "), no background hits will be generated");
  }

  if (m_clusters < 0) B2WARNING("The parameter BGLevelClusters should lie between 0 and 100, the input parameter is negative (" << m_clusters << "), no background hit clusters will be generated");

  if (m_clusters > 100) {
    B2WARNING("The parameter BGLevelClusters should lie between 0 and 100, The input parameter is larger than 100 (" << m_clusters << "), no background hit clusters will be generated");
  }

  //Generate single hits background
  if (m_hits > 0 && m_hits <= 100) {
    B2INFO("Generate single hits background ...");
    //the generated background should go approximately with 1/r
    //it is realized by making the number of background hits per layer constant
    int nHitsPerLayer = int(m_hits * nWires * 0.01 / nLayers + 0.5);   //(round up)

    for (int layerId = 0; layerId < nLayers; layerId++) { //loop over all layers (56)
      for (int i = 0; i < nHitsPerLayer; i++) { //loop to create hits
        wireId = rand() % cdcg.nWiresInLayer(layerId);  //choose a random wire

        superlayerId = (layerId - 2) / 6;          //calculate other necessary variables (ilayerId: layerId within a superlayer)
        ilayerId = (layerId - 2) % 6;
        if (superlayerId == 0) {ilayerId += 2;}    //superlayer 0 has 8 layers, all others 6

        driftTime = (unsigned short)(maxDriftTime * rand() / (RAND_MAX + 1.0));    //assign some random number to driftTime and charge
        charge = (unsigned short)(maxCharge * rand() / (RAND_MAX + 1.0));

        B2DEBUG(100, "superlayer: " << superlayerId << "  layer: " << ilayerId << "  wire: " << wireId << "  driftTime: " << driftTime << "  charge: " << charge);

        new(cdcHits.nextFreeAddress()) CDCHit(driftTime, charge, superlayerId, ilayerId, wireId);
        ++counter;
      }
    } //end loop over all layers

    NTotalHits = cdcHits.getEntries();
  }

  //Generate cluster background
  if (m_clusters > 0 && m_clusters <= 100) {
    B2INFO("Generate cluster background ...");
    int nClustersPerLayer = int(m_clusters * nWires * 0.0019 / nLayers + 0.5);   //assume that each cluster will have ~ 5 hits (average) (round up)
    for (int layerId = 0; layerId < nLayers; layerId++) {
      for (int i = 0; i < nClustersPerLayer; i++) {
        wireId = rand() % cdcg.nWiresInLayer(layerId);  //choose a random wire

        superlayerId = (layerId - 2) / 6;          //calculate other necessary variables (ilayerId: layerId within a superlayer)
        ilayerId = (layerId - 2) % 6;
        if (superlayerId == 0) {ilayerId += 2;}    //superlayer 0 has 8 layers, all others 6

        driftTime = (unsigned short)(maxDriftTime * rand() / (RAND_MAX + 1.0));    //assign some random number to driftTime and charge
        charge = (unsigned short)(maxCharge * rand() / (RAND_MAX + 1.0));

        B2DEBUG(100, "superlayer: " << superlayerId << "  layer: " << ilayerId << "  wire: " << wireId << "  driftTime: " << driftTime << "  charge: " << charge);
        int counter = cdcHits.getEntries();

        new(cdcHits.nextFreeAddress()) CDCHit(driftTime, charge, superlayerId, ilayerId, wireId);
        ++counter;

        //Now add some more Hits around this new hit
        //(This is a very straightforward and simple method without any claim to be reasonable and realistic, it is only some first approach to check the pattern recognition with some more complicated background than single hits)
        if (layerId < 5) {
          new(cdcHits.nextFreeAddress()) CDCHit(driftTime, charge, superlayerId, ilayerId + 1, wireId);
          B2DEBUG(150, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
          ++counter;
          new(cdcHits.nextFreeAddress()) CDCHit(driftTime, charge, superlayerId, ilayerId + 2, wireId);
          B2DEBUG(150, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
          ++counter;
        }
        if (layerId > 1) {
          new(cdcHits.nextFreeAddress()) CDCHit(driftTime, charge, superlayerId, ilayerId - 1, wireId);
          B2DEBUG(150, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
          ++counter;
          new(cdcHits.nextFreeAddress()) CDCHit(driftTime, charge, superlayerId, ilayerId - 2, wireId);
          B2DEBUG(150, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
          ++counter;
        }

        if (unsigned(wireId) < cdcg.nWiresInLayer(layerId) - 1) {
          new(cdcHits.nextFreeAddress()) CDCHit(driftTime, charge, superlayerId, ilayerId, wireId + 1);
          B2DEBUG(150, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
          ++counter;

        }
        if (wireId > 1) {
          new(cdcHits.nextFreeAddress()) CDCHit(driftTime, charge, superlayerId, ilayerId, wireId - 1);
          B2DEBUG(150, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
          ++counter;
        }

      }
      NTotalHits = cdcHits.getEntries();
    }
  }

  NBackgroundHits = NTotalHits - NTrackHits;
  B2INFO("----->  Total number of CDCHits: " << NTotalHits << " (Tracks: " << NTrackHits << " , Background: " << NBackgroundHits << ")");
  if (NTotalHits == NTrackHits) B2WARNING("No background CDCHits were generated by the CDCSimpleBackgroundModule");
}

void CDCSimpleBackgroundModule::endRun()
{
  B2INFO("endRun CDCSimpleBackground");
}

void CDCSimpleBackgroundModule::terminate()
{
  B2INFO("terminate CDCSimpleBackground");
}

