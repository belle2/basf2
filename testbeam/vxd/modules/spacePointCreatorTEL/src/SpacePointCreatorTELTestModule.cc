/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#include <testbeam/vxd/modules/spacePointCreatorTEL/SpacePointCreatorTELTestModule.h>

#include <testbeam/vxd/tracking/spacePointCreation/TBSpacePoint.h>

#include <framework/dataobjects/EventMetaData.h>

#include <framework/logging/Logger.h>

#include <genfit/Track.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/RKTrackRep.h>


using namespace std;
using namespace Belle2;


REG_MODULE(SpacePointCreatorTELTest)

SpacePointCreatorTELTestModule::SpacePointCreatorTELTestModule() :
  Module()
{
  setDescription("Tester module for the validity of the TBSpacePointCreatorModule. TODO: at the moment, the genfit-output can only verified visually (by checking, whether the detector types match the number of dimensions stored in the trackPoint)! when full reco chain is working, this testerModule should be extended! -> verification that input cluster(s) is/are converted to genfit-stuff shall be resilient!");

  vector<string> containerSpapacePointsName = { string("") };

  // 1. Collections.
  addParam("PXDClusters", m_pxdClustersName,
           "PXDCluster collection name", string(""));
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));
  addParam("TelClusters", m_telClustersName,
           "TelCluster collection name", string(""));
  addParam("AllSpacePointContainers", m_containerSpacePointsName,
           "SpacePoints collection name", containerSpapacePointsName);

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string(""));
//   addParam("OnlySingleClusterSpacePoints", m_onlySingleClusterSpacePoints,
//            "standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters", bool(false));
}



void SpacePointCreatorTELTestModule::initialize()
{
  // prepare all store- and relationArrays:
  m_pxdClusters.isOptional(m_pxdClustersName);
  m_svdClusters.isOptional(m_svdClustersName);
  m_telClusters.isOptional(m_telClustersName);


  if (m_pxdClusters.isOptional() == true) {
    // retrieve name again (faster than doing everything in the event):
    m_pxdClustersName = m_pxdClusters.getName();
  }


  if (m_svdClusters.isOptional() == true) {
    // retrieve name again (faster than doing everything in the event):
    m_svdClustersName = m_svdClusters.getName();
  }

  if (m_telClusters.isOptional() == true) {
    // retrieve name again (faster than doing everything in the event):
    m_telClustersName = m_telClusters.getName();
  }

  for (auto aName : m_containerSpacePointsName) {
    m_allSpacePointStoreArrays.push_back({/* aName,*/ StoreArray<SpacePoint>(aName) });
  }


  B2INFO("SpacePointCreatorTELTestModule(" << m_nameOfInstance << ")::initialize: names found for containers:\n" <<
         "pxdClusters: " << m_pxdClustersName <<
         "\nsvdClusters: " << m_svdClustersName <<
         "\ntelClusters: " << m_telClustersName);

  B2WARNING("TODO: at the moment, the genfit-output can only verified visually (by checking, whether the detector types match the number of dimensions stored in the trackPoint)! when full reco chain is working, this testerModule should be extended! -> verification that input cluster(s) is/are converted to genfit-stuff shall be resilient!");
}



void SpacePointCreatorTELTestModule::event()
{
  B2DEBUG(1, "\n\nSpacePointCreatorTestModule(" << m_nameOfInstance << " event " << StoreObjPtr<EventMetaData>("EventMetaData",
          DataStore::c_Event)->getEvent() << "): got " << m_pxdClusters.getEntries() << "/" << m_svdClusters.getEntries() << "/" <<
          m_telClusters.getEntries() << " PXD/SVD/TelClusters in this event\n\n");


  for (StoreArray<SpacePoint>& aStoreArrayInterface : m_allSpacePointStoreArrays) {
    B2DEBUG(1, " Entering storeArray<SpacePoint> " << aStoreArrayInterface.getName() << " with " << aStoreArrayInterface.getEntries() <<
            " spacePoints");

    for (unsigned int i = 0; i < uint(aStoreArrayInterface.getEntries()); ++i) {
      B2DEBUG(2, " Executing SpacePoint " << i);
      const SpacePoint* sp = aStoreArrayInterface[i];

      vector<int> indices; // WARNING: nothing is happening with them yet -> write some sort of test!
      string clusterContainer = "";

      if (sp->getType() == VXD::SensorInfoBase::SensorType::SVD) {
        B2DEBUG(2, " SpacePoint " << i << " is attached to SVDCluster(s) of StoreArray " << sp->getArrayName());

        for (const SVDCluster& aCluster : sp->getRelationsTo<SVDCluster>()) {
          indices.push_back(aCluster.getArrayIndex());
          clusterContainer = aCluster.getArrayName();

          B2DEBUG(2, " SpacePoint " << i <<
                  " got pointer to SVDCluster with index " << aCluster.getArrayIndex() <<
                  " stored in Array " << aCluster.getArrayName());
        }
      } else if (sp->getType() == VXD::SensorInfoBase::SensorType::PXD) {
        B2DEBUG(2, " SpacePoint " << i << " is attached to PXDCluster of StoreArray " << sp->getArrayName());

        for (const PXDCluster& aCluster : sp->getRelationsTo<PXDCluster>()) {

          indices.push_back(aCluster.getArrayIndex());

          clusterContainer = aCluster.getArrayName();

          B2DEBUG(2, " SpacePoint " << i <<
                  " got pointer to PXDCluster with index " << aCluster.getArrayIndex() <<
                  " stored in Array " << aCluster.getArrayName());

        }
      } else if (sp->getType() == VXD::SensorInfoBase::SensorType::TEL) {
        B2DEBUG(2, " SpacePoint " << i << " is attached to TelCluster of StoreArray " << sp->getArrayName());

        for (const TelCluster& aCluster : sp->getRelationsTo<TelCluster>()) {

          indices.push_back(aCluster.getArrayIndex());
          clusterContainer = aCluster.getArrayName();

          B2DEBUG(2, " SpacePoint " << i <<
                  " got pointer to TelCluster with index " << aCluster.getArrayIndex() <<
                  " stored in Array " << aCluster.getArrayName());

        }

      } else { B2ERROR(" SpacePoint is of unknown type " << sp->getType()); }


      B2DEBUG(1, "SpacePoint " << i <<
              " got sensorType: " << sp->getType() <<
              ", VxdID: " << VxdID(sp->getVxdID()) <<
              ", storeName for Cluster(says SpacePoint): " << sp->getArrayName() <<
              ", storeName for Cluster(says Cluster): " << clusterContainer);
    }

    if (aStoreArrayInterface.getEntries() == 0) {
      B2WARNING("given storeArray has no entries! skipping storeArray " << aStoreArrayInterface.getName());
      continue;
    }
    B2DEBUG(1, "testGenfitCompatibility: feed the track with spacePoints ported to genfit compatible stuff");
    genfit::AbsTrackRep* trackRep = new genfit::RKTrackRep(211);

    genfit::Track track(trackRep, aStoreArrayInterface[0]->getPosition(), TVector3(23., 42., 5.));

    // feed the track with spacePoints ported to genfit compatible stuff:
    // is rather complicated since the assignment operator is protected:
    std::vector< std::pair<VXD::SensorInfoBase::SensorType, genfit::AbsMeasurement*> > hitOutput;

    for (auto& aSP : aStoreArrayInterface) {
      std::vector<genfit::PlanarMeasurement> tempMeasurements = aSP.getGenfitCompatible();

      for (genfit::PlanarMeasurement& measurement : tempMeasurements) {
        hitOutput.push_back({aSP.getType(), measurement.clone()});
      }
    }

    for (unsigned i = 0; i < hitOutput.size(); i++) {
      track.insertMeasurement(hitOutput[i].second);
      genfit::TrackPoint* point = track.getPointWithMeasurement(i);
      genfit::AbsMeasurement* rawPoint = point->getRawMeasurement();
      B2DEBUG(2, " executing AbsMeasurement " << i << " with detectorID(PXD = 0,SVD=1,TEL=2,VXD=-1) : " << hitOutput[i].first << ":\n");
      point->Print();
      B2DEBUG(2, " converted absMeasurement is of detID: " << rawPoint->getDetId() << ", hitID: " << rawPoint->getHitId());
    }
  }

}
