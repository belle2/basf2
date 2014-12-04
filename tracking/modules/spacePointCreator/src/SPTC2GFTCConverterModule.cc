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

  addParam("SpacePointTCName", m_SPTCName, "Name of the container containing the SpacePointTrackCands to convert", string(""));
  addParam("genfitTCName", m_genfitTCName, "Name of the container of the (output) genfit::TrackCands", string(""));
  addParam("PXDClusters", m_PXDClustersName, "Name of the container of the PXD Clusters", string(""));
  addParam("SVDClusters", m_SVDClustersName, "Name of the container of the SVD Clusters", string(""));
}

void SPTC2GFTCConverterModule::initialize()
{
  B2INFO("SPTC2GFTCConverter --------------- initialize() -------------------");
  // initialize the counters
  initializeCounters();

  // Register StoreArray<genfit::TrackCand> in the DataStore
  StoreArray<genfit::TrackCand> genfitTCs(m_genfitTCName);
  genfitTCs.registerPersistent(m_genfitTCName);

  StoreArray<SpacePointTrackCand> spTCs(m_SPTCName);
  spTCs.required(m_SPTCName);

  // Register Relation between the two StoreArrays
  spTCs.registerRelationTo(genfitTCs);

  StoreArray<PXDCluster>::required(m_PXDClustersName);
  StoreArray<SVDCluster>::required(m_SVDClustersName);
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

    try {
      genfit::TrackCand genfitTC;
      B2DEBUG(20, "SpacePointTrackCand " << iTC << " contains " << trackCand->getNHits() << " SpacePoints")

      std::vector<const SpacePoint*> tcSpacePoints = trackCand->getHits();
      std::vector<double> sortingParams = trackCand->getSortingParameters();

      // loop over all SpacePoints and look at their relations
      for (unsigned int iTCSP = 0; iTCSP < tcSpacePoints.size(); ++iTCSP) {
        const SpacePoint* aSP = tcSpacePoints[iTCSP];
        double sortingParam = sortingParams[iTCSP];

        auto detType = aSP->getType();

        if (detType == VXD::SensorInfoBase::PXD) {
          // the relation between SpacePoints & PXDClusters is bijective, hence it is unecessary to search for more than one relations
          PXDCluster* pxdCluster = aSP->getRelatedTo<PXDCluster>(m_PXDClustersName);
          if (pxdCluster == NULL) {
            B2ERROR("Found no relation to a PXDCluster in StoreArray" << m_PXDClustersName << " for SpacePoint " << aSP->getArrayIndex() << " from StoreArray" << aSP->getArrayName() << ". This SpacePoint will be skipped and will not be contained in the genfit::TrackCand");
            throw (ClusterNotFound());
          }
          int hitID = pxdCluster->getArrayIndex();
          genfitTC.addHit(Const::PXD, hitID, -1, sortingParam); // setting PlaneID to -1
          B2DEBUG(60, "Added PXDCluster " << hitID << " from StoreArray " << pxdCluster->getArrayName() << " to genfit::TrackCand");
        } else if (detType == VXD::SensorInfoBase::SVD) {
          // More than one SVD Cluster can be related from a SpacePoint (maximum two), hence get all related Clusters and add them
          RelationVector<SVDCluster> svdClusters = aSP->getRelationsTo<SVDCluster>(m_SVDClustersName);
          if (svdClusters.size() > 2) { throw SpacePoint::InvalidNumberOfClusters(); }
          else if (svdClusters.size() == 0) {
            B2ERROR("Found no relation to a SVDCluster in StoreArray" << m_SVDClustersName << " for SpacePoint " << aSP->getArrayIndex() << " from StoreArray" << aSP->getArrayName() << ". This SpacePoint will be skipped and will not be contained in the genfit::TrackCand");
            throw (ClusterNotFound());
          }
          B2DEBUG(70, "Found " << svdClusters.size() << " SVD Clusters related to SpacePoint");
          for (const SVDCluster & aCluster : svdClusters) {
            int hitID = aCluster.getArrayIndex();
            genfitTC.addHit(Const::SVD, hitID, -1, sortingParam); // setting PlaneID to -1
            B2DEBUG(60, "Added SVDCluster " << hitID << " from StoreArray " << aCluster.getArrayName()  << " to genfit::TrackCand");
          }
        } else {
          throw SpacePointTrackCand::UnsupportedDetType();
        }
      }

      // set other properties of TrackCand
      genfitTC.set6DSeedAndPdgCode(trackCand->getStateSeed(), trackCand->getPdgCode());
      genfitTC.setCovSeed(trackCand->getCovSeed());

      // add genfit::TrackCand to StoreArray
      m_genfitTCCtr += 1;
//       genfitTC.Print(); // debug purposes
      B2DEBUG(15, "genfit::TrackCand contains " << genfitTC.getNHits() << "TrackCandHits.");
      genfit::TrackCand* newTC = genfitTCs.appendNew(genfitTC);
      trackCand->addRelationTo(newTC);
    } catch (SpacePointTrackCand::UnsupportedDetType& anException) {
      B2WARNING("Caught exception during conversion from SpacePointTrackCand to genfit::TrackCand: " << anException.what() << ". This SpacePointTrackCand was skipped!")
    } catch (SpacePoint::InvalidNumberOfClusters& anException) {
      B2WARNING("Caught exception during conversion from SpacePointTrackCand to genfit::TrackCand: " << anException.what() << ". This SpacePointTrackCand was skipped!")
    } catch (ClusterNotFound& anException) {
      B2WARNING("Caught exception during conversion from SpacePointTrackCand to genfit::TrackCand: " << anException.what() << ". This SpacePointTrackCand was skipped!")
    }
  }
}

void SPTC2GFTCConverterModule::terminate()
{
  B2INFO("SPTC2GFTCConverter::terminate: got " << m_SpacePointTCCtr << " SpacePointTrackCands and created " << m_genfitTCCtr << " genfit::TrackCands");
}

void SPTC2GFTCConverterModule::initializeCounters()
{
  m_SpacePointTCCtr = 0;
  m_genfitTCCtr = 0;
}