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
// // #include <vxd/simulation/SensitiveDetector.h>

#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

//C++ std lib
#include <vector>
#include <map>
#include <utility>
#include <string>
using std::vector;
using std::map;
using std::pair;
using std::string;
// #include <fstream>
#include <cmath>
using std::sin;
using std::cos;
using std::sqrt;

#include <iostream>
using std::cerr;
using std::endl;

//root stuff
#include <TRandom.h>
//Boost-packages:
#include <boost/foreach.hpp>

using namespace Belle2;

REG_MODULE(VXDSimpleClusterizer)

VXDSimpleClusterizerModule::VXDSimpleClusterizerModule() : Module()
{
  setDescription("The VXDSimpleClusterizerModule generates PXD/SVD Clusters using TrueHits, energy threshold and gaussian smearing as well. Its purpose is fast clusterizing for tracking test procedures, using standardized PXD/SVD-Cluster");

  addParam("energyThresholdU", m_energyThresholdU, "particles with energy deposit in U lower than this will not create a cluster in SVD (GeV)", double(17.4E-6));
  addParam("energyThresholdV", m_energyThresholdV, "particles with energy deposit in V lower than this will not create a cluster in SVD (GeV)", double(28.6E-6));
  addParam("energyThreshold", m_energyThreshold, "particles with energy deposit lower than this will not create a cluster in PXD (GeV)", double(7E-6));
  addParam("onlyPrimaries", m_onlyPrimaries, "if true use only primary particles from the generator no particles created by Geant4", false);
  addParam("uniSigma", m_uniSigma, "you can define the sigma of the smearing. Standard value is the sigma of the unifom distribution for 0-1: 1/sqrt(12)", double(1. / sqrt(12.)));
  addParam("setMeasSigma", m_setMeasSigma, "if positive value (in cm) is given it will be used as the sigma to smear the Clusters otherwise pitch/uniSigma will be used", -1.0);
}

VXDSimpleClusterizerModule::~VXDSimpleClusterizerModule()
{
}

void VXDSimpleClusterizerModule::initialize()
{
  //output containers, will be created when registered here
  // since clusterArrays are inconsistently defined, both versions are implemented
  StoreArray<PXDCluster>::registerPersistent();
  StoreArray<SVDCluster>::registerPersistent();


  RelationArray::registerPersistent<PXDCluster, MCParticle>("", "");
  RelationArray::registerPersistent<MCParticle, PXDCluster>("", "");
  RelationArray::registerPersistent<SVDCluster, MCParticle>("", "");
  RelationArray::registerPersistent<MCParticle, SVDCluster>("", "");

  RelationArray::registerPersistent<PXDCluster, PXDTrueHit>("", "");
  RelationArray::registerPersistent<PXDTrueHit, PXDCluster>("", "");
  RelationArray::registerPersistent<SVDCluster, SVDTrueHit>("", "");
  RelationArray::registerPersistent<SVDTrueHit, SVDCluster>("", "");

}

void VXDSimpleClusterizerModule::beginRun()
{
  string paramValue;
  if (m_onlyPrimaries == true) { paramValue = "true, means that there are no secondary hits (for 1-track events this means no ghost hits guaranteed)"; } else { paramValue = "false, means that secondary hits can occur and increase the rate of ghost hits"; }
  B2INFO("VXDSimpleClusterizer: parameter onlyPrimaries is set to " << paramValue)
}

void VXDSimpleClusterizerModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

  B2DEBUG(1, "*******  VXDSimpleClusterizerModule processing event number: " << eventMetaDataPtr->getEvent() << " *******");
  //all the input containers. First: MCParticles
  StoreArray<MCParticle> mcParticles("");
  int nMcParticles = mcParticles.getEntries();
  if (nMcParticles == 0) {B2DEBUG(100, "MCTrackFinder: MCParticlesCollection is empty!");}
  //PXD
  StoreArray<PXDTrueHit> pxdTrueHits("");
  int nPxdTrueHits = pxdTrueHits.getEntries();
  if (nPxdTrueHits == 0) {B2DEBUG(100, "MCTrackFinder: PXDHitsCollection is empty!");}

  RelationIndex<MCParticle, PXDTrueHit> relIndexPxdTH2McP(mcParticles, pxdTrueHits, "");

  //SVD
  StoreArray<SVDTrueHit> svdTrueHits("");
  int nSvdTrueHits = svdTrueHits.getEntries();
  if (nSvdTrueHits == 0) {B2DEBUG(100, "MCTrackFinder: SVDHitsCollection is empty!");}

  RelationIndex<MCParticle, SVDTrueHit> relIndexSvdTH2McP(mcParticles, svdTrueHits, "");

  //output containers

  StoreArray<PXDCluster> pxdClusters("");
  if (!pxdClusters.isValid())  pxdClusters.create();
  StoreArray<SVDCluster> svdClusters("");
  if (!svdClusters.isValid())  svdClusters.create();

  RelationArray relPXDClusterMCParticle(pxdClusters, mcParticles);
  RelationArray relPXDClusterTrueHit(pxdClusters, pxdTrueHits);
  RelationArray relSVDClusterMCParticle(svdClusters, mcParticles);
  RelationArray relSVDClusterTrueHit(svdClusters, svdTrueHits);
  RelationArray relMCParticlePXDCluster(mcParticles, pxdClusters);
  RelationArray relTrueHitPXDCluster(pxdTrueHits, pxdClusters);
  RelationArray relMCParticleSVDCluster(mcParticles, svdClusters);
  RelationArray relTrueHitSVDCluster(svdTrueHits, svdClusters);

  // since there are many clusters produced by the same particle, we have to collect them before creating the relations. This feature is needed for the MCtrackFinder
  typedef map<unsigned int, vector<unsigned int> > relationMap; // needed for the storing loops at the end of the event
  typedef pair<unsigned int, vector<unsigned int> > mapEntry;
  relationMap relationMapMcP2PxdCls;
  relationMap relationMapMcP2SvdCls;

  double sigmaU = m_setMeasSigma;
  double sigmaV = m_setMeasSigma;

///////////////////////////////////////////////////// NOW THE PXD
  for (unsigned int currentTrueHit = 0; int (currentTrueHit) not_eq nPxdTrueHits; ++currentTrueHit) {

    const PXDTrueHit* aPxdTrueHit = pxdTrueHits[currentTrueHit];
    relIndexPxdTH2McP.getFirstElementTo(aPxdTrueHit)->from->fixParticleList();
    unsigned int particleID = relIndexPxdTH2McP.getFirstElementTo(aPxdTrueHit)->from->getArrayIndex(); // WARNING:possible trap, might change in future revisions
    double energy = aPxdTrueHit->getEnergyDep();

    B2DEBUG(100, " PXD, current TrueHit has an energy deposit of " << energy * 1000.0 << "MeV ")
    if (energy < m_energyThreshold) { //ignore hit if energy deposit is too small
      B2DEBUG(100, " PXD, TrueHit discarded because of energy deposit too small")
      m_weakPXDHitCtr++;
      continue;
    }

    if (m_onlyPrimaries == true) { // ingore hits not comming from primary particles (e.g material effects particles)
      if (relIndexPxdTH2McP.getFirstElementTo(aPxdTrueHit)->from->hasStatus(MCParticle::c_PrimaryParticle) == false) {
        continue; // jump to next pxdTrueHit
      }
    }

    //smear the pxdTrueHit and get needed variables for storing
    VxdID aVXDId = aPxdTrueHit->getSensorID();
    double uTrue = aPxdTrueHit->getU();
    double vTrue = aPxdTrueHit->getV();
    double u = -20;
    double v = -20;
    if (m_setMeasSigma < 0.0) {
      const PXD::SensorInfo& geometry = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(aVXDId));
      sigmaU = geometry.getUPitch(uTrue) * m_uniSigma;
      sigmaV = geometry.getVPitch(vTrue) * m_uniSigma;
    }
    B2DEBUG(1000, "sigU sigV: " << sigmaU << " " << sigmaV);

    u = gRandom->Gaus(uTrue, sigmaU);
    v = gRandom->Gaus(vTrue, sigmaV);

    // Save as new 2D-PXD-cluster
    unsigned int clusterIndex = pxdClusters.getEntries();
    new(pxdClusters.nextFreeAddress()) PXDCluster(aVXDId, u, v, 0, 0, 1, 1, 1, 1, 1);

    //add relations:
    relPXDClusterTrueHit.add(clusterIndex, currentTrueHit);
    relTrueHitPXDCluster.add(currentTrueHit, clusterIndex);
    relPXDClusterMCParticle.add(clusterIndex, particleID);
    relationMapMcP2PxdCls[particleID].push_back(clusterIndex);
  }

////////////////////////////////////////////////  NOW THE SVD
  for (unsigned int currentTrueHit = 0; int (currentTrueHit) not_eq nSvdTrueHits; ++currentTrueHit) {

    const SVDTrueHit* aSvdTrueHit = svdTrueHits[currentTrueHit];
    relIndexSvdTH2McP.getFirstElementTo(aSvdTrueHit)->from->fixParticleList();
    unsigned int particleID = relIndexSvdTH2McP.getFirstElementTo(aSvdTrueHit)->from->getArrayIndex(); // WARNING:possible trap, might change in future revisions
    double energy = aSvdTrueHit->getEnergyDep();

    B2DEBUG(100, " SVD, current TrueHit has an energy deposit of " << energy * 1000.0 << "MeV ")
    if (energy < (m_energyThresholdU + m_energyThresholdV)) { //ignore hit if energy deposity is too snall
      m_weakSVDHitCtr++;
      B2DEBUG(100, " SVD, TrueHit discarded because of energy deposit too small")
      continue;
    }

    if (m_onlyPrimaries == true) { // ingore hits not comming from primary particles (e.g material effects particles)
      if (relIndexSvdTH2McP.getFirstElementTo(aSvdTrueHit)->from->hasStatus(MCParticle::c_PrimaryParticle) == false) {
        continue; // jump to next svdTrueHit
      }
    }

    // smear the SvdTrueHit and get needed variables for storing
    VxdID aVXDId = aSvdTrueHit->getSensorID();
    double uTrue = aSvdTrueHit->getU();
    double vTrue = aSvdTrueHit->getV();
    double u = -20;
    double v = -20;
    if (m_setMeasSigma < 0.0) {
      const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(aVXDId));
      sigmaU = geometry.getUPitch(uTrue) * m_uniSigma;
      sigmaV = geometry.getVPitch(vTrue) * m_uniSigma;
    }
    B2DEBUG(1000, "sigU sigV: " << sigmaU << " " << sigmaV);

    u = gRandom->Gaus(uTrue, sigmaU);
    v = gRandom->Gaus(vTrue, sigmaV);

    // Save as two new 1D-SVD-clusters
    unsigned int clusterIndex = svdClusters.getEntries();
    double timeStamp = svdTrueHits[currentTrueHit]->getGlobalTime();
    new(svdClusters.nextFreeAddress()) SVDCluster(aVXDId, true, u, timeStamp, 0, 1, 1, 3); // in a typical situation 3-5 Strips are excited per Hit -> set to 3
    new(svdClusters.nextFreeAddress()) SVDCluster(aVXDId, false, v, timeStamp, 0, 1, 1, 3);

    // add relations
    vector<unsigned int> clusterVector;
    clusterVector.push_back(clusterIndex);
    clusterVector.push_back(clusterIndex + 1);
    relTrueHitSVDCluster.add(currentTrueHit, clusterVector);
    relSVDClusterTrueHit.add(clusterIndex, currentTrueHit);
    relSVDClusterTrueHit.add(clusterIndex + 1, currentTrueHit);
    relSVDClusterMCParticle.add(clusterIndex, particleID);
    relSVDClusterMCParticle.add(clusterIndex + 1, particleID);
    relationMapMcP2SvdCls[particleID].push_back(clusterIndex);
    relationMapMcP2SvdCls[particleID].push_back(clusterIndex + 1);
  }

  B2DEBUG(10, "------------------------------------------------------");
  // exporting RelationMaps to relationArrays...
  BOOST_FOREACH(mapEntry aRelation, relationMapMcP2PxdCls) {
    relMCParticlePXDCluster.add(aRelation.first, aRelation.second);
    B2DEBUG(20, "mcParticle " << aRelation.first << " has " << aRelation.second.size() << " relations to PXD clusters");
  }
  BOOST_FOREACH(mapEntry aRelation, relationMapMcP2SvdCls) {
    relMCParticleSVDCluster.add(aRelation.first, aRelation.second);
    B2DEBUG(20, "mcParticle " << aRelation.first << " has " << aRelation.second.size() << " relations to SVD clusters");
  }

  B2DEBUG(10, "VXDSimpleClusterizerModule: Number of PXDHits: " << nPxdTrueHits);
  B2DEBUG(10, "VXDSimpleClusterizerModule: Number of SVDDHits: " << nSvdTrueHits);
  B2DEBUG(10, "VXDSimpleClusterizerModule: total Number of MCParticles: " << nMcParticles);
  B2DEBUG(10, "pxdClusters.getEntries()" << pxdClusters.getEntries());
  B2DEBUG(10, "svdClusters.getEntries()" << svdClusters.getEntries());
  B2DEBUG(10, "relPXDClusterMCParticle.getEntries()" << relPXDClusterMCParticle.getEntries());
  B2DEBUG(10, "relPXDClusterTrueHit.getEntries())" << relPXDClusterTrueHit.getEntries());
  B2DEBUG(10, "relSVDClusterMCParticle.getEntries()" << relSVDClusterMCParticle.getEntries());
  B2DEBUG(10, "relSVDClusterTrueHit.getEntries()" << relSVDClusterTrueHit.getEntries());
  B2DEBUG(10, "relationMapMcP2PxdCls.size()" << relationMapMcP2PxdCls.size());
  B2DEBUG(10, "relationMapMcP2SvdCls.size()" << relationMapMcP2SvdCls.size());
  B2DEBUG(10, "------------------------------------------------------");
}

void VXDSimpleClusterizerModule::endRun()
{
  B2DEBUG(1, "EndRun: SimpleClusterizerModule discarded " << m_weakPXDHitCtr << " PXDTrueHits and " << m_weakSVDHitCtr << " SVDTrueHits total");
}

void VXDSimpleClusterizerModule::terminate()
{
}
