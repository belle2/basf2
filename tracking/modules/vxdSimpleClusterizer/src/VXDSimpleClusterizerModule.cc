/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/vxdSimpleClusterizer/VXDSimpleClusterizerModule.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

//C++ std lib
#include <vector>
using std::vector;
#include <fstream>
using std::ofstream;
#include <cmath>
using std::sin;
using std::cos;
using std::sqrt;

#include <iostream>
using std::cerr;
using std::endl;


//root stuff
#include <TRandom.h>
//genfit stuff
#include <GFTrackCand.h>

using namespace Belle2;


REG_MODULE(VXDSimpleClusterizer)

VXDSimpleClusterizerModule::VXDSimpleClusterizerModule() : Module()
{
  setDescription("The VXDSimpleClusterizerModule generates PXD/SVD Clusters using TrueHits, energy threshold and gaussian smearing as well. Its purpose is fast clusterizing for tracking test procedures, using standardized PXD/SVD-Cluster");

  addParam("energyThresholdU", m_energyThresholdU, "particles with energy deposit in U lower than this will not create a cluster in SVD (GeV)", 17.4E-6);
  addParam("energyThresholdV", m_energyThresholdV, "particles with energy deposit in V lower than this will not create a cluster in SVD (GeV)", 28.6E-6);
  addParam("energyThreshold", m_energyThreshold, "particles with energy deposit lower than this will not create a cluster in PXD (GeV)", 5E-6);
  addParam("onlyPrimaries", m_onlyPrimaries, "if true use only primary particles from the generator no particles created by Geant4", false);
  addParam("setMeasSigma", m_setMeasSigma, "if positive value (in cm) is given it will be used as the sigma to smear the Clusters otherwise pitch/sqrt(12) will be used", -1.0);

  addParam("writeTruthToFile", m_writeTruthToFile, "write the u and v coordinate of the trueHits of one track to a text file", false);

}

VXDSimpleClusterizerModule::~VXDSimpleClusterizerModule()
{
}

void VXDSimpleClusterizerModule::initialize()
{
  //output containers, will be created when initialized here
  StoreArray<PXDCluster> pxdClusters("pxdClusters");
  StoreArray<SVDCluster> svdClusters("svdClusters");
  StoreArray<MCParticle> mcParticles("");
  StoreArray<PXDTrueHit> pxdTrueHits("");
  StoreArray<SVDTrueHit> svdTrueHits("");

  RelationArray relPXDClusterMCParticle(pxdClusters, mcParticles, "relPXDClusterMCParticle");
  RelationArray relPXDClusterTrueHit(pxdClusters, pxdTrueHits, "relPXDClusterTrueHit");
  RelationArray relSVDClusterMCParticle(svdClusters, mcParticles, "relSVDClusterMCParticle");
  RelationArray relSVDClusterTrueHit(svdClusters, svdTrueHits, "relSVDClusterTrueHit");
}

void VXDSimpleClusterizerModule::beginRun()
{

}

void VXDSimpleClusterizerModule::event()
{
  ofstream dataOut;
  if (m_writeTruthToFile == true) dataOut.open("data.txt");
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();

  B2DEBUG(1, "*******  VXDSimpleClusterizerModule processing event number: " << eventCounter << " *******");
  //all the input containers. First: MCParticles
  StoreArray<MCParticle> mcParticles("");
  int nMcParticles = mcParticles.getEntries();
  B2DEBUG(10, "VXDSimpleClusterizerModule: total Number of MCParticles: " << nMcParticles);
  if (nMcParticles == 0) {B2DEBUG(100, "MCTrackFinder: MCParticlesCollection is empty!");}
  //PXD
  StoreArray<PXDTrueHit> pxdTrueHits("");
  int nPxdTrueHits = pxdTrueHits.getEntries();
  B2DEBUG(10, "VXDSimpleClusterizerModule: Number of PXDHits: " << nPxdTrueHits);
  if (nPxdTrueHits == 0) {B2DEBUG(100, "MCTrackFinder: PXDHitsCollection is empty!");}

  RelationIndex<MCParticle, PXDTrueHit> relMcPxdTrueHit;

  //SVD
  StoreArray<SVDTrueHit> svdTrueHits("");
  int nSvdTrueHits = svdTrueHits.getEntries();
  B2DEBUG(10, "VXDSimpleClusterizerModule: Number of SVDDHits: " << nSvdTrueHits);
  if (nSvdTrueHits == 0) {B2DEBUG(100, "MCTrackFinder: SVDHitsCollection is empty!");}

  RelationIndex<MCParticle, SVDTrueHit> relMcSvdTrueHit;



  //output containers
  StoreArray<PXDCluster> pxdClusters("pxdClusters");
  StoreArray<SVDCluster> svdClusters("svdClusters");
  MCParticle* aMcParticle = mcParticles[0];

  RelationArray relPXDClusterMCParticle(pxdClusters, mcParticles, "relPXDClusterMCParticle");
  RelationArray relPXDClusterTrueHit(pxdClusters, pxdTrueHits, "relPXDClusterTrueHit");
  RelationArray relSVDClusterMCParticle(svdClusters, mcParticles, "relSVDClusterMCParticle");
  RelationArray relSVDClusterTrueHit(svdClusters, svdTrueHits, "relSVDClusterTrueHit");

  double sigmaU = m_setMeasSigma;
  double sigmaV = m_setMeasSigma;

  //smear the pxd true Hits and store them in pxdSimpleDigiHits
  int aClusterHit = 0;
  for (int i = 0; i not_eq nPxdTrueHits; ++i) {
    const PXDTrueHit* aPxdTrueHit = pxdTrueHits[i];
    float energy = aPxdTrueHit->getEnergyDep();
    if (energy < m_energyThreshold) { //ignore hit if energy deposit is too small
      continue;
    }

    if (m_onlyPrimaries == true) { // ingore hits not comming from primary particles (e.g material effects particles)
      RelationIndex<MCParticle, PXDTrueHit>::range_from iterPairMcPxd = relMcPxdTrueHit.getFrom(aMcParticle);
      while (iterPairMcPxd.first not_eq iterPairMcPxd.second) {
        if (iterPairMcPxd.first->to == aPxdTrueHit) {
          break;
        }
        ++iterPairMcPxd.first;
      }
      if (iterPairMcPxd.first == iterPairMcPxd.second) { // if the beak statement never was invoced the current pxdHit does not come from a primary particle and will not be added to the track cand
        continue;
      }
    }


    VxdID aVXDId = aPxdTrueHit->getSensorID();
    float uTrue = aPxdTrueHit->getU();
    float vTrue = aPxdTrueHit->getV();
    float u = -20;
    float v = -20;
    if (m_setMeasSigma < 0.0) {
      const PXD::SensorInfo& geometry = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(aVXDId));
      sigmaU = geometry.getUPitch(uTrue) / sqrt(12.);
      sigmaV = geometry.getVPitch(vTrue) / sqrt(12.);

    }
    B2DEBUG(1000, "sigU sigV: " << sigmaU << " " << sigmaV);

    //make a normal measurment
    u = gRandom->Gaus(uTrue, sigmaU);
    v = gRandom->Gaus(vTrue, sigmaV);

    if (m_writeTruthToFile == true) dataOut << uTrue << "\t" << vTrue << "\n";


    new(pxdClusters->AddrAt(aClusterHit)) PXDCluster(aVXDId, u, v, 0, 0, 1, 1, 1, 1, 1);

    ++aClusterHit;

  }
////////////////////////////////////////////////  NOW THE SVD
  //smear the svd true Hits and store them in svdSimpleDigiHits
  aClusterHit = 0;
  for (int i = 0; i not_eq nSvdTrueHits; ++i) {
    const SVDTrueHit* aSvdTrueHit = svdTrueHits[i];
    float energy = aSvdTrueHit->getEnergyDep();
    if (energy < m_energyThresholdU + m_energyThresholdV) { //ignore hit if energy deposity is too snall
      continue;
    }

    if (m_onlyPrimaries == true) {
      RelationIndex<MCParticle, SVDTrueHit>::range_from iterPairMcSvd = relMcSvdTrueHit.getFrom(aMcParticle);
      while (iterPairMcSvd.first not_eq iterPairMcSvd.second) {
        if (iterPairMcSvd.first->to == aSvdTrueHit) {
          break;
        }
        ++iterPairMcSvd.first;
      }
      if (iterPairMcSvd.first == iterPairMcSvd.second) { // if the beak statement never was invoced the current svdHit does not come from a primary particle and will not be added to the track cand
        continue;
      }
    }

    VxdID aVXDId = aSvdTrueHit->getSensorID();
    float uTrue = aSvdTrueHit->getU();
    float vTrue = aSvdTrueHit->getV();
    float u = -20;
    float v = -20;
    if (m_setMeasSigma < 0.0) {
      const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(aVXDId));
      sigmaU = geometry.getUPitch(uTrue) / sqrt(12);
      sigmaV = geometry.getVPitch(vTrue) / sqrt(12);
    }
    B2DEBUG(1000, "sigU sigV: " << sigmaU << " " << sigmaV);

    //make a normal measurment
    u = gRandom->Gaus(uTrue, sigmaU);
    v = gRandom->Gaus(vTrue, sigmaV);

    if (m_writeTruthToFile == true) dataOut << uTrue << "\t" << vTrue << "\n";

    new(svdClusters->AddrAt(aClusterHit)) SVDCluster(aVXDId, true, u, svdTrueHits[i]->getGlobalTime(), 0, 1, 1, 1);

    ++aClusterHit;

    new(svdClusters->AddrAt(aClusterHit)) SVDCluster(aVXDId, false, v, svdTrueHits[i]->getGlobalTime(), 0, 1, 1, 1);

    ++aClusterHit;
  }

  if (m_writeTruthToFile == true) dataOut.close();

  B2DEBUG(10, "pxdClusters.getEntries()" << pxdClusters.getEntries());
  B2DEBUG(10, "svdClusters.getEntries()" << svdClusters.getEntries());


}

void VXDSimpleClusterizerModule::endRun()
{


}

void VXDSimpleClusterizerModule::terminate()
{

}

