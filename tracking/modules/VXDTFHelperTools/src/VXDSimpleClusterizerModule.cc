/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/VXDTFHelperTools/VXDSimpleClusterizerModule.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

//C++ std lib
#include <vector>
using std::vector;

#include <string>
using std::string;

#include <cmath>
using std::sin;
using std::cos;
using std::sqrt;

//root stuff
#include <TRandom.h>

using namespace Belle2;

REG_MODULE(VXDSimpleClusterizer)

VXDSimpleClusterizerModule::VXDSimpleClusterizerModule() : Module()
{
  InitializeVariables();


  setDescription("The VXDSimpleClusterizerModule generates PXD/SVD Clusters using TrueHits. Energy-deposit threshold and gaussian smearing can be chosen, non-primary-particles can be filtered as well. Its purpose is fast clusterizing for tracking test procedures, using standardized PXD/SVD-Cluster");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("energyThresholdU", m_energyThresholdU,
           "particles with energy deposit in U lower than this will not create a cluster in SVD (GeV)", double(17.4E-6));
  addParam("energyThresholdV", m_energyThresholdV,
           "particles with energy deposit in V lower than this will not create a cluster in SVD (GeV)", double(28.6E-6));
  addParam("energyThreshold", m_energyThreshold,
           "particles with energy deposit lower than this will not create a cluster in PXD (GeV)", double(7E-6));
  addParam("onlyPrimaries", m_onlyPrimaries, "if true use only primary particles from the generator no particles created by Geant4",
           false);
  addParam("uniSigma", m_uniSigma,
           "you can define the sigma of the smearing. Standard value is the sigma of the unifom distribution for 0-1: 1/sqrt(12)",
           double(1. / sqrt(12.)));
  addParam("setMeasSigma", m_setMeasSigma,
           "if positive value (in cm) is given it will be used as the sigma to smear the Clusters otherwise pitch/uniSigma will be used",
           -1.0);
  addParam("PXDTrueHits", m_pxdTrueHitsName,
           "PXDTrueHit collection name", string(""));
  addParam("SVDTrueHits", m_svdTrueHitsName,
           "SVDTrueHit collection name", string(""));
  addParam("MCParticles", m_mcParticlesName,
           "MCParticle collection name", string(""));
  addParam("PXDClusters", m_pxdClustersName,
           "PXDCluster collection name", string(""));
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));
}


void VXDSimpleClusterizerModule::initialize()
{
  //input containers:
  m_mcParticles.isOptional(m_mcParticlesName);
  m_pxdTrueHits.isOptional(m_pxdTrueHitsName);
  m_svdTrueHits.isOptional(m_svdTrueHitsName);

  // initializing StoreArrays for clusters. needed to give them the names set by parameters
  m_pxdClusters.registerInDataStore(m_pxdClustersName, DataStore::c_ErrorIfAlreadyRegistered);
  m_svdClusters.registerInDataStore(m_svdClustersName, DataStore::c_ErrorIfAlreadyRegistered);

  if (m_pxdTrueHits.isOptional() == true) {

    //Relations to cluster objects only if the ancestor relations exist:
    m_pxdClusters.registerRelationTo(m_pxdTrueHits);
    m_pxdClustersName = m_pxdClusters.getName();
    m_pxdTrueHitsName = m_pxdTrueHits.getName();

    if (m_mcParticles.isOptional() == true) {
      m_pxdClusters.registerRelationTo(m_mcParticles);
      m_mcParticlesName = m_mcParticles.getName();
    }
  }

  if (m_svdTrueHits.isOptional() == true) {

    //Relations to cluster objects only if the ancestor relations exist:
    m_svdClusters.registerRelationTo(m_svdTrueHits);
    m_svdClustersName = m_svdClusters.getName();
    m_svdTrueHitsName = m_svdTrueHits.getName();

    if (m_mcParticles.isOptional() == true) {
      m_svdClusters.registerRelationTo(m_mcParticles);
      m_mcParticlesName = m_mcParticles.getName();
    }
  }
}



void VXDSimpleClusterizerModule::beginRun()
{
  string paramValue;
  if (m_onlyPrimaries == true) {
    paramValue = "true, means that there are no secondary hits (for 1-track events this means no ghost hits guaranteed)";
  } else {
    paramValue = "false, means that secondary hits can occur and increase the rate of ghost hits";
  }
  B2INFO("VXDSimpleClusterizer: parameter onlyPrimaries is set to " << paramValue);

  InitializeVariables();
}



void VXDSimpleClusterizerModule::event()
{
  // counter for cases when a trueHit god discarded:
  int discardedPXDEdeposit = 0, discardedSVDEdeposit = 0, discardedPXDFake = 0, discardedSVDFake = 0;

  const StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

  B2DEBUG(5, "*******  VXDSimpleClusterizerModule processing event number: " << eventMetaDataPtr->getEvent() << " *******");


  //check all the input containers. First: MCParticles
  int nMcParticles = m_mcParticles.getEntries();
  if (nMcParticles == 0) {B2DEBUG(100, "MCTrackFinder: MCParticlesCollection is empty!");}
  //PXD
  int nPxdTrueHits = m_pxdTrueHits.getEntries();
  if (nPxdTrueHits == 0) {B2DEBUG(100, "MCTrackFinder: PXDHitsCollection is empty!");}
  //SVD
  int nSvdTrueHits = m_svdTrueHits.getEntries();
  if (nSvdTrueHits == 0) {B2DEBUG(100, "MCTrackFinder: SVDHitsCollection is empty!");}
  B2DEBUG(175, "found " << nMcParticles << "/" << nPxdTrueHits << "/" << nSvdTrueHits << " mcParticles, pxdTrueHits, svdTrueHits");


  double sigmaU = m_setMeasSigma;
  double sigmaV = m_setMeasSigma;


///////////////////////////////////////////////// NOW THE PXD
  for (unsigned int currentTrueHit = 0; int (currentTrueHit) not_eq nPxdTrueHits; ++currentTrueHit) {
    B2DEBUG(175, "begin PXD current TrueHit: " << currentTrueHit << " of nPxdTrueHits total: " << nPxdTrueHits);

    const PXDTrueHit* aPxdTrueHit = m_pxdTrueHits[currentTrueHit];
    const MCParticle* aMcParticle = aPxdTrueHit->getRelatedFrom<MCParticle>();
    unsigned int particleID = std::numeric_limits<unsigned int>::max();

    if (aMcParticle != nullptr) { particleID = aMcParticle->getArrayIndex(); }

    double energy = aPxdTrueHit->getEnergyDep();


    if (m_onlyPrimaries == true) { // ingore hits not comming from primary particles (e.g material effects particles)
      if (aMcParticle == nullptr or aMcParticle->hasStatus(MCParticle::c_PrimaryParticle) == false) {
        m_fakePXDHitCtr++;
        discardedPXDFake++;
        continue; // jump to next pxdTrueHit
      }
    }

    B2DEBUG(100, " PXD, current TrueHit " << currentTrueHit << " connected to " << particleID << " has an energy deposit of " <<
            energy * 1000.0 << "MeV ");
    if (energy < m_energyThreshold) { //ignore hit if energy deposit is too small
      B2DEBUG(100, " PXD, TrueHit discarded because of energy deposit too small");
      m_weakPXDHitCtr++;
      discardedPXDEdeposit++;
      continue;
    }

    //smear the pxdTrueHit and get needed variables for storing
    VxdID aVXDId = aPxdTrueHit->getSensorID();
    double uTrue = aPxdTrueHit->getU();
    double vTrue = aPxdTrueHit->getV();
    double u = -20;
    double v = -20;
    if (m_setMeasSigma < 0.0) {
      const VXD::SensorInfoBase* sensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(aVXDId);
      sigmaU = sensorInfo->getUPitch(uTrue) * m_uniSigma;
      sigmaV = sensorInfo->getVPitch(vTrue) * m_uniSigma;
    }
    B2DEBUG(175, "sigU sigV: " << sigmaU << " " << sigmaV);

    if (m_setMeasSigma != 0) {
      u = gRandom->Gaus(uTrue, sigmaU);
      v = gRandom->Gaus(vTrue, sigmaV);
    } else {
      u = uTrue;
      v = vTrue;
    }

    if (m_setMeasSigma == 0 and m_uniSigma == 0) {
      // in this case, the hits will not be smeared, but still we need some measurement error-values to be able to do some fitting... WARNING currently arbritary values here, better solution recommended!
      sigmaU = 0.000001;
      sigmaV = 0.000001;
    }

    /** Constructor.
     * @param sensorID Sensor compact ID.
     * @param uPosition Cluster u coordinate (r-phi).
     * @param vPosition Cluster v coordinate (z).
     * @param uError Error (estimate) of uPosition.
     * @param vError Error (estiamte) of vPosition.
     * @param uvRho u-v error correlation coefficient.
     * @param clsCharge The cluster charge.
     * @param seedCharge The charge of the cluster seed.
     * @param clsSize size of the cluster in pixels.
     * @param uSize number of pixel columns contributing to the cluster.
     * @param vSize number of pixel rows contributing to the cluster.
     *     unsigned short m_uStart;    Start column of the cluster
     * unsigned short m_vStart;   Start row of the cluster
     */
    // Save as new 2D-PXD-cluster
    PXDCluster* newCluster = m_pxdClusters.appendNew(aVXDId, u, v, sigmaU, sigmaV, 0, 1, 1, 1, 1, 1, 1, 1);
    // add relations
    newCluster->addRelationTo(m_pxdTrueHits[currentTrueHit]);

    if (particleID != std::numeric_limits<unsigned int>::max()) {
      newCluster->addRelationTo(m_mcParticles[particleID]);
      B2DEBUG(20, "mcParticle " << particleID << " has " << aMcParticle->getRelationsTo<PXDCluster>().size() <<
              " relations to PXD clusters");
    }
  }



////////////////////////////////////////////////  NOW THE SVD
  for (unsigned int currentTrueHit = 0; int (currentTrueHit) not_eq nSvdTrueHits; ++currentTrueHit) {
    B2DEBUG(175, "begin SVD current TrueHit: " << currentTrueHit << " of nSvdTrueHits total: " << nSvdTrueHits);

    const SVDTrueHit* aSvdTrueHit = m_svdTrueHits[currentTrueHit];
    const MCParticle* aMcParticle = aSvdTrueHit->getRelatedFrom<MCParticle>();
    unsigned int particleID = std::numeric_limits<unsigned int>::max();

    if (m_onlyPrimaries == true) { // ingore hits not comming from primary particles (e.g material effects particles)
      if (aMcParticle == nullptr or aMcParticle->hasStatus(MCParticle::c_PrimaryParticle) == false) {
        m_fakeSVDHitCtr++;
        discardedSVDFake++;
        continue; // jump to next svdTrueHit
      }
    }

    if (aMcParticle != nullptr) { particleID = aMcParticle->getArrayIndex(); }
    double energy = aSvdTrueHit->getEnergyDep();

    B2DEBUG(100, " SVD, current TrueHit " << currentTrueHit << " connected to " << particleID << " has an energy deposit of " <<
            energy * 1000.0 << "MeV ");
    if (energy < (m_energyThresholdU + m_energyThresholdV)) { //ignore hit if energy deposity is too snall
      m_weakSVDHitCtr++;
      discardedSVDEdeposit++;
      B2DEBUG(100, " SVD, TrueHit discarded because of energy deposit too small");
      continue;
    }


    // smear the SvdTrueHit and get needed variables for storing
    VxdID aVXDId = aSvdTrueHit->getSensorID();
    double uTrue = aSvdTrueHit->getU();
    double vTrue = aSvdTrueHit->getV();
    double u = -20;
    double v = -20;
    if (m_setMeasSigma < 0.0) {
      const VXD::SensorInfoBase* sensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(aVXDId);
      sigmaU = sensorInfo->getUPitch(uTrue) * m_uniSigma;
      sigmaV = sensorInfo->getVPitch(vTrue) * m_uniSigma;
    }
    B2DEBUG(150, "sigU sigV: " << sigmaU << " " << sigmaV);

    if (m_setMeasSigma != 0) {
      u = gRandom->Gaus(uTrue, sigmaU);
      v = gRandom->Gaus(vTrue, sigmaV);
    } else {
      u = uTrue;
      v = vTrue;
    }

    if (m_setMeasSigma == 0) {
      // in this case, the hits will not be smeared, but still we need some measurement error-values to be able to do some fitting... WARNING currently arbritary values here, better solution recommended!
      sigmaU = 0.000001;
      sigmaV = 0.000001;
    }

    // Save as two new 1D-SVD-clusters
    double timeStamp = m_svdTrueHits[currentTrueHit]->getGlobalTime();

    /** Constructor.
     * @param sensorID Sensor compact ID.
     * @param isU True if u strips, otherwise false.
     * @param position Seed strip coordinate.
     * @param positionSigma Error in strip coordinate.
     * @param clsTime The average of waveform maxima times of strips in the cluster.
     * @param clsTimeSigma The standard deviation of waveform maxima times.
     * @param clsCharge The cluster charge in electrons.
     * @param seedCharge The charge of the seed strip in electrons.
     * @param clsSize The size of the cluster in the corresponding strip pitch units.
     * @param clsSNR The cluster charge SNR
     */

    SVDCluster* newClusterU =  m_svdClusters.appendNew(aVXDId, true, u, sigmaU, timeStamp, 0, 1, 1,
                                                       3, 1); // in a typical situation 3-5 Strips are excited per Hit -> set to 3
    // add relations to u-cluster
    newClusterU->addRelationTo(m_svdTrueHits[currentTrueHit]);

    SVDCluster* newClusterV = m_svdClusters.appendNew(aVXDId, false, v, sigmaV, timeStamp, 0, 1, 1, 3, 1);
    // add relations to v-cluster
    newClusterV->addRelationTo(m_svdTrueHits[currentTrueHit]);

    if (particleID != std::numeric_limits<unsigned int>::max()) {
      newClusterU->addRelationTo(m_mcParticles[particleID]);
      newClusterV->addRelationTo(m_mcParticles[particleID]);
      B2DEBUG(20, "mcParticle " << particleID << " has " << aMcParticle->getRelationsTo<SVDCluster>().size() <<
              " relations to SVD clusters");
    }

  }

  B2DEBUG(10, "------------------------------------------------------");

  B2DEBUG(10, "VXDSimpleClusterizerModule: Number of PXDHits: " << nPxdTrueHits);
  B2DEBUG(10, "VXDSimpleClusterizerModule: Number of SVDDHits: " << nSvdTrueHits);
  B2DEBUG(10, "VXDSimpleClusterizerModule: total Number of MCParticles: " << nMcParticles);
  B2DEBUG(10, "pxdClusters.getEntries()" << m_pxdClusters.getEntries());
  B2DEBUG(10, "svdClusters.getEntries()" << m_svdClusters.getEntries());
  B2DEBUG(10, "------------------------------------------------------");

  B2DEBUG(1, "VXDSimpleClusterizer - event " << eventMetaDataPtr->getEvent() << ":\n" << "of " << nPxdTrueHits << "/" << nSvdTrueHits
          << " PXD-/SVDTrueHits, " << discardedPXDEdeposit << "/" << discardedSVDEdeposit << " hits were discarded bec. of low E-deposit & "
          << discardedPXDFake << "/" << discardedSVDFake << " hits were discarded bec. of being a fake. " << m_pxdClusters.getEntries() << "/"
          << m_svdClusters.getEntries() << " Clusters were stored.\n");
}



void VXDSimpleClusterizerModule::endRun()
{
  B2INFO("VXDSimpleClusterizerModule::EndRun:\nSimpleClusterizerModule discarded " << m_weakPXDHitCtr << " PXDTrueHits and " <<
         m_weakSVDHitCtr << " SVDTrueHits because of low E-deposit-threshold and discarded "  << m_fakePXDHitCtr << " PXDTrueHits and " <<
         m_fakeSVDHitCtr << " SVDTrueHits because they were fake");
}
