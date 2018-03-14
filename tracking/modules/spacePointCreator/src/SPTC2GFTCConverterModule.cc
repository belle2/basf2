/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/SPTC2GFTCConverterModule.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <genfit/TrackCand.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/gearbox/Const.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SPTC2GFTCConverter)

SPTC2GFTCConverterModule::SPTC2GFTCConverterModule() :
  Module()
{
  setDescription("Module for converting SpacePointTrackCand to genfit::TrackCand.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SpacePointTCName", m_SPTCName, "Name of the container containing the SpacePointTrackCands to convert", string(""));
  addParam("genfitTCName", m_genfitTCName, "Name of the container of the (output) genfit::TrackCands", string(""));
  addParam("PXDClusters", m_PXDClustersName, "Name of the container of the PXD Clusters", string(""));
  addParam("SVDClusters", m_SVDClustersName, "Name of the container of the SVD Clusters", string(""));

  initializeCounters(); // NOTE: they get initialized in initialize again!!
}

void SPTC2GFTCConverterModule::initialize()
{
  B2INFO("SPTC2GFTCConverter --------------- initialize() -------------------");
  // initialize the counters
  initializeCounters();

  // Register StoreArray<genfit::TrackCand> in the DataStore
  StoreArray<genfit::TrackCand> genfitTCs(m_genfitTCName);
  genfitTCs.registerInDataStore(m_genfitTCName, DataStore::c_ErrorIfAlreadyRegistered);

  StoreArray<SpacePointTrackCand> spTCs(m_SPTCName);
  spTCs.isRequired(m_SPTCName);

  // Register Relation between the two StoreArrays
  spTCs.registerRelationTo(genfitTCs);

  StoreArray<PXDCluster> PXDClusters(m_PXDClustersName); PXDClusters.isRequired(m_PXDClustersName);
  StoreArray<SVDCluster> SVDClusters(m_SVDClustersName); SVDClusters.isRequired(m_SVDClustersName);
}

void SPTC2GFTCConverterModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaData->getEvent();
  B2DEBUG(10, "SPTC2GFTCConverter::event() processing event " << eventCounter << " ----------");

  StoreArray<genfit::TrackCand> genfitTCs(m_genfitTCName);
  StoreArray<SpacePointTrackCand> spacePointTCs(m_SPTCName);

  int nSPTCs = spacePointTCs.getEntries();
  B2DEBUG(15, "Found " << nSPTCs << " SpacePointTrackCands in StoreArray " << spacePointTCs.getName());

  for (int iTC = 0; iTC < nSPTCs; ++iTC) {
    const SpacePointTrackCand* trackCand = spacePointTCs[iTC];
    m_SpacePointTCCtr += 1;

    genfit::TrackCand genfitTC;
    B2DEBUG(20, "SpacePointTrackCand " << iTC << " contains " << trackCand->getNHits() << " SpacePoints");

    std::vector<const SpacePoint*> tcSpacePoints = trackCand->getHits();
    std::vector<double> sortingParams = trackCand->getSortingParameters();

    // loop over all SpacePoints and look at their relations
    for (unsigned int iTCSP = 0; iTCSP < tcSpacePoints.size(); ++iTCSP) {
      const SpacePoint* aSP = tcSpacePoints[iTCSP];
      double sortingParam = sortingParams[iTCSP];

      auto detType = aSP->getType();
      int detID = detType == VXD::SensorInfoBase::SVD ? Const::SVD : Const::PXD;
      vector<int> clusterInds;

      try {

        if (detType == VXD::SensorInfoBase::PXD) { clusterInds = getRelatedClusters<PXDCluster>(aSP, m_PXDClustersName); }
        else if (detType == VXD::SensorInfoBase::SVD) { clusterInds = getRelatedClusters<SVDCluster>(aSP, m_SVDClustersName); }
        else throw SpacePointTrackCand::UnsupportedDetType();

      } catch (std::runtime_error& anE) {
        B2WARNING("Caught exception during creation of a genfit::TrackCand: " << anE.what());
        m_skippedSPsCtr++;
        continue; // with next SpacePoint
      } catch (...) {
        B2WARNING("Caught unknown exception during conversion from SPTC to GFTC!");
        throw;
      }

      for (int hitID : clusterInds) {
        genfitTC.addHit(detID, hitID, -1, sortingParam);
        B2DEBUG(60, "Added Cluster " << hitID << " with detID " << detID << " to genfit::TrackCand");
      }
    }

    // set other properties of TrackCand
    genfitTC.set6DSeedAndPdgCode(trackCand->getStateSeed(), trackCand->getPdgCode());
    genfitTC.setCovSeed(trackCand->getCovSeed());
    genfitTC.setMcTrackId(trackCand->getMcTrackID());

    // add genfit::TrackCand to StoreArray
    m_genfitTCCtr++;
    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 150, PACKAGENAME())) genfitTC.Print(); // debug purposes
    B2DEBUG(15, "genfit::TrackCand contains " << genfitTC.getNHits() << " TrackCandHits.");
    genfit::TrackCand* newTC = genfitTCs.appendNew(genfitTC);
    trackCand->addRelationTo(newTC);
    B2DEBUG(15, "Added relation between SPTC " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName() <<
            " and GFTC.");
  }
}

void SPTC2GFTCConverterModule::terminate()
{
  stringstream output;
  output << "SPTC2GFTCConverter::terminate: got " << m_SpacePointTCCtr << " SpacePointTrackCands and created " << m_genfitTCCtr <<
         " genfit::TrackCands";
  if (m_skippedSPsCtr) output << ". " << m_skippedSPsCtr << " SpacePoints were skipped!";
  B2INFO(output.str());
}

// ========================================== GET RELATED CLUSTERS ================================================================
template<typename ClusterType>
std::vector<int> SPTC2GFTCConverterModule::getRelatedClusters(const Belle2::SpacePoint* spacePoint, const std::string& clusterNames)
{
  std::vector<int> clusterInds;

  RelationVector<ClusterType> relatedClusters = spacePoint->getRelationsTo<ClusterType>(clusterNames);
  if (relatedClusters.size() == 0) {
    B2DEBUG(1, "Found no related Clusters for SpacePoint " << spacePoint->getArrayIndex() << " from Array " <<
            spacePoint->getArrayName());
    throw ClusterNotFound();
  } else B2ASSERT("Too many clusters!", relatedClusters.size() < 3);

  for (const ClusterType& cluster : relatedClusters) {
    clusterInds.push_back(cluster.getArrayIndex());
    B2DEBUG(60, "Cluster " << cluster.getArrayIndex() << " from Array " << cluster.getArrayName() << " is related to SpacePoint " <<
            spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
  }

  return clusterInds;
}

void SPTC2GFTCConverterModule::initializeCounters()
{
  m_SpacePointTCCtr = 0;
  m_genfitTCCtr = 0;
  m_skippedSPsCtr = 0;
}
