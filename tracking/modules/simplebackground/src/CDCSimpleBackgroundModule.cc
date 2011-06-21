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

#include <cdc/dataobjects/CDCHit.h>

#include <time.h>

using namespace std;
using namespace Belle2;


REG_MODULE(CDCSimpleBackground)

CDCSimpleBackgroundModule::CDCSimpleBackgroundModule() : Module()
{
  setDescription("The CDCSimpleBackgroundModule generates random hits in the CDC to emulate background. It adds digitized CDCHits to the CDCHits array.");

  addParam("CDCHitsColName", m_cdcHitsColName, "Name of collection holding the digitized CDCHits, new CDCHits will be added to this array", string("CDCHits"));
  addParam("BGLevelHits", m_hits, "Percentage of wires hit by background through single hits", 2.0);
  addParam("BGLevelClusters", m_clusters, "Percentage of wires hit by background through hit clusters (up to 7 hits each) ", 0.0);


}

CDCSimpleBackgroundModule::~CDCSimpleBackgroundModule()
{
}

void CDCSimpleBackgroundModule::initialize()
{

}

void CDCSimpleBackgroundModule::beginRun()
{

}

void CDCSimpleBackgroundModule::event()
{
  B2INFO("*******  CDCSimpleBackgroundModule  *******");

  //StoreArray with digitized CDCHits
  StoreArray <CDCHit> cdcHits(m_cdcHitsColName);
  int NTrackHits = cdcHits.getEntries();
  int NBackgroundHits = 0;
  int NTotalHits = NTrackHits + NBackgroundHits;
  B2DEBUG(149, "CDCSimpleBackground: Number of digitized CDCHits coming from tracks: " << NTrackHits);
  if (NTrackHits == 0) B2WARNING("CDCSimpleBackground: CDCHitsCollection is empty!");

  //initialize random number generator
  //srand((unsigned int) 111); //to get reproducible results
  srand((unsigned)time(0));   //to get new background in each run

  int counter = cdcHits->GetLast() + 1; //counter for new hits

  //initialize variables which describe a CDCHit
  int layerId;
  int superlayerId;
  int wireId;
  double charge;
  double driftTime;

  //CDC geometry information, should come from the common geometry database
  int NWires = 14336;
  int nSuperlayers = 9;

  int WiresPerSL[9];
  WiresPerSL[0] = 160;
  WiresPerSL[1] = 160;
  WiresPerSL[2] = 192;
  WiresPerSL[3] = 224;
  WiresPerSL[4] = 256;
  WiresPerSL[5] = 288;
  WiresPerSL[6] = 320;
  WiresPerSL[7] = 352;
  WiresPerSL[8] = 384;

  int LayersPerSL[9];
  LayersPerSL[0] = 8;
  LayersPerSL[1] = 6;
  LayersPerSL[2] = 6;
  LayersPerSL[3] = 6;
  LayersPerSL[4] = 6;
  LayersPerSL[5] = 6;
  LayersPerSL[6] = 6;
  LayersPerSL[7] = 6;
  LayersPerSL[8] = 6;

  //Information about the possible values of charge/drift time. This is the first guess!!! It has to be investigated which values are realistic here...
  double maxDriftTime = 10000;
  double maxCharge = 100;

  //Check if the parameters are valid
  if (m_hits < 0) B2WARNING("The parameter BGLevelHits should lie between 0 and 100, the input parameter is negative (" << m_hits << "), no background hits will be generated");

  if (m_hits > 100) {
    B2WARNING("The parameter BGLevelHits should lie between 0 and 100, The input parameter is larger than 100 (" << m_hits << "), it will be changed to 100");
    m_hits = 100;
  }

  if (m_clusters < 0) B2WARNING("The parameter BGLevelClusters should lie between 0 and 100, the input parameter is negative (" << m_clusters << "), no background hit clusters will be generated");

  if (m_clusters > 100) {
    B2WARNING("The parameter BGLevelClusters should lie between 0 and 100, The input parameter is larger than 100 (" << m_clusters << "), it will be changed to 100");
    m_clusters = 100;
  }

  //Generate single hits background
  if (m_hits > 0 && m_hits <= 100) {
    B2INFO("Generate single hits background ...");
    int nHits = int(m_hits * NWires * 0.01); //calculate the number of hits
    B2INFO("Create " << nHits << " new CDCHits");

    for (int i = 0; i < nHits; i++) { //loop to create hits
      superlayerId = rand() % nSuperlayers;
      layerId = rand() % LayersPerSL[superlayerId];
      wireId = rand() % WiresPerSL[superlayerId];
      driftTime = maxDriftTime * rand() / (RAND_MAX + 1.0);
      charge = maxCharge * rand() / (RAND_MAX + 1.0);
      B2DEBUG(100, "superlayer: " << superlayerId << "  layer: " << layerId << "  wire: " << wireId << "  driftTime: " << driftTime << "  charge: " << charge);

      new(cdcHits->AddrAt(counter)) CDCHit(driftTime, charge, superlayerId, layerId, wireId);
      ++counter;
    }

    NTotalHits = cdcHits.getEntries();
  }

  //Generate cluster background
  if (m_clusters > 0 && m_clusters <= 100) {
    B2INFO("Generate cluster background ...");
    int nClusters = int(m_clusters * NWires * 0.00167); //assume that each cluster will have 6 hits (average)
    B2INFO("Create " << nClusters << " new clusters of CDCHits");

    for (int i = 0; i < nClusters; i++) {
      superlayerId = rand() % nSuperlayers;
      layerId = rand() % LayersPerSL[superlayerId];
      wireId = rand() % WiresPerSL[superlayerId];
      driftTime = maxDriftTime * rand() / (RAND_MAX + 1.0);
      charge = maxCharge * rand() / (RAND_MAX + 1.0);

      B2DEBUG(100, "superlayer: " << superlayerId << "  layer: " << layerId << "  wire: " << wireId << "  driftTime: " << driftTime << "  charge: " << charge);
      int counter = cdcHits->GetLast() + 1;

      new(cdcHits->AddrAt(counter)) CDCHit(driftTime, charge, superlayerId, layerId, wireId);
      ++counter;

      //Now add some more Hits around this new hit
      //(This is a very straightforward and simple method without any claim to be reasonable and realistic, it is only some first approach to check the pattern recognition with some more complicated background than single hits)
      if (layerId < 5) {
        new(cdcHits->AddrAt(counter)) CDCHit(driftTime, charge, superlayerId, layerId + 1, wireId);
        B2DEBUG(100, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
        ++counter;
        new(cdcHits->AddrAt(counter)) CDCHit(driftTime, charge, superlayerId, layerId + 2, wireId);
        B2DEBUG(100, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
        ++counter;
      }
      if (layerId > 1) {
        new(cdcHits->AddrAt(counter)) CDCHit(driftTime, charge, superlayerId, layerId - 1, wireId);
        B2DEBUG(100, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
        ++counter;
        new(cdcHits->AddrAt(counter)) CDCHit(driftTime, charge, superlayerId, layerId - 2, wireId);
        B2DEBUG(100, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
        ++counter;
      }

      if (wireId < WiresPerSL[superlayerId] - 1) {
        new(cdcHits->AddrAt(counter)) CDCHit(driftTime, charge, superlayerId, layerId, wireId + 1);
        B2DEBUG(100, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
        ++counter;

      }
      if (wireId > 1) {
        new(cdcHits->AddrAt(counter)) CDCHit(driftTime, charge, superlayerId, layerId, wireId + 1);
        B2DEBUG(100, "--- add neigbour Hit: superlayer: " << cdcHits[counter]->getISuperLayer() << "  layer: " << cdcHits[counter]->getILayer() << "  wire: " << cdcHits[counter]->getIWire());
        ++counter;
      }

    }
    NTotalHits = cdcHits.getEntries();
  }

  NBackgroundHits = NTotalHits - NTrackHits;
  B2INFO("----->  Total number of CDCHits: " << NTotalHits << " (Tracks: " << NTrackHits << " , Background: " << NBackgroundHits << ")");
  if (NTotalHits == NTrackHits) B2WARNING("No background CDCHits were generated by the CDCSimpleBackgroundModule");
}

void CDCSimpleBackgroundModule::endRun()
{

}

void CDCSimpleBackgroundModule::terminate()
{

}

