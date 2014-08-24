/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#include <testbeam/vxd/modules/tbspacePointCreator/TBSpacePointCreatorTestModule.h>

#include <tracking/spacePointCreation/SpacePointHelperFunctions.h>

#include <testbeam/vxd/tracking/spacePointCreation/TBSpacePoint.h>

#include <framework/logging/Logger.h>

#include <genfit/Track.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/RKTrackRep.h>


using namespace std;
using namespace Belle2;


REG_MODULE(TBSpacePointCreatorTest)

TBSpacePointCreatorTestModule::TBSpacePointCreatorTestModule() :
  Module(),
  m_spMetaInfo("", DataStore::c_Persistent)
{
  setDescription("Tester module for the validity of the TBSpacePointCreatorModule. TODO: at the moment, the genfit-output can only verified visually (by checking, whether the detector types match the number of dimensions stored in the trackPoint)! when full reco chain is working, this testerModule should be extended! -> verification that input cluster(s) is/are converted to genfit-stuff shall be resilient!");

  // 1. Collections.
  addParam("PXDClusters", m_pxdClustersName,
           "PXDCluster collection name", string(""));
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));
  addParam("TelClusters", m_telClustersName,
           "TelCluster collection name", string(""));
  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", string(""));

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string(""));
//   addParam("OnlySingleClusterSpacePoints", m_onlySingleClusterSpacePoints,
//            "standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters", bool(false));
}



void TBSpacePointCreatorTestModule::initialize()
{
  // prepare all store- and relationArrays:
  m_spacePoints.isRequired(m_spacePointsName);
  m_pxdClusters.isOptional(m_pxdClustersName);
  m_svdClusters.isOptional(m_svdClustersName);
  m_telClusters.isOptional(m_telClustersName);
  m_spMetaInfo.isRequired();


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

  // retrieve name for spacePoint too (faster than doing everything in the event):
  m_spacePointsName = m_spacePoints.getName();


  B2INFO("TBSpacePointCreatorTestModule(" << m_nameOfInstance << ")::initialize: names found for containers:\n" <<
         "pxdClusters: " << m_pxdClustersName <<
         "\nsvdClusters: " << m_svdClustersName <<
         "\ntelClusters: " << m_telClustersName <<
         "\nspacePoints: " << m_spacePointsName <<
         "\nspacePointsMetaInfo-storeObjPtr: " << m_spMetaInfo.getName() <<
         "\nspacePointsMetaInfo # of names stored: " << m_spMetaInfo->getnNames())

  B2WARNING("TODO: at the moment, the genfit-output can only verified visually (by checking, whether the detector types match the number of dimensions stored in the trackPoint)! when full reco chain is working, this testerModule should be extended! -> verification that input cluster(s) is/are converted to genfit-stuff shall be resilient!")
}



void TBSpacePointCreatorTestModule::event()
{

  uint nPXDClusters = m_pxdClusters.getEntries();
  uint nSVDClusters = m_svdClusters.getEntries();
  uint nTelClusters = m_telClusters.getEntries();
  uint nSpacePoints = m_spacePoints.getEntries();

  if (nSVDClusters == 0 and nTelClusters == 0 and nPXDClusters == 0 and nSpacePoints != 0)
  { B2ERROR("TBSpacePointCreatorTestModule(" << m_nameOfInstance << "): there are " << nSpacePoints << " spacePoints although there are no Clusters! Check settings!") }

  if (nSpacePoints == 0 and (nSVDClusters != 0 or nTelClusters != 0 or nPXDClusters != 0))
  { B2ERROR("TBSpacePointCreatorTestModule(" << m_nameOfInstance << "): there are no spacePoints although there are " << nPXDClusters << "/" << nSVDClusters << "/" << nTelClusters << "  PXD/SVD/Tel Clusters! Check settings!") }

  if (nSpacePoints == 0 or (nSVDClusters == 0 and nTelClusters == 0 and nPXDClusters == 0)) {
    B2WARNING("TBSpacePointCreatorTestModule(" << m_nameOfInstance << "): no input-data for testing in current event! Skipping it...")
    return;
  }


  B2DEBUG(2, "TBSpacePointCreatorTestModule(" << m_nameOfInstance << "): starting event with " << nSpacePoints << " spacePoints")
  for (unsigned int i = 0; i < nSpacePoints; ++i) {
    B2DEBUG(2, " Executing SpacePoint " << i << endl)
    const SpacePoint* sp = m_spacePoints[i];

    vector<unsigned int> indices; // WARNING: nothing is happening with them yet -> write some sort of test!
    string clusterContainer = "";

    if (sp->getType() == VXD::SensorInfoBase::SensorType::SVD) {
      B2DEBUG(2, " SpacePoint " << i << " is attached to SVDCluster(s) of StoreArray " << sp->getClusterStoreName())
      for (const SVDCluster & aCluster : sp->getRelationsTo<SVDCluster>(sp->getClusterStoreName())) {
        indices.push_back(aCluster.getArrayIndex());
        clusterContainer = aCluster.getArrayName();

        B2DEBUG(2, " SpacePoint " << i <<
                " got pointer to SVDCluster with index " << aCluster.getArrayIndex() <<
                " stored in Array " << aCluster.getArrayName())
      }
    } else if (sp->getType() == VXD::SensorInfoBase::SensorType::PXD) {
      B2DEBUG(2, " SpacePoint " << i << " is attached to PXDCluster of StoreArray " << sp->getClusterStoreName())
      const PXDCluster* aCluster = sp->getRelatedTo<PXDCluster>(sp->getClusterStoreName());
      indices.push_back(aCluster->getArrayIndex());
      clusterContainer = aCluster->getArrayName();

      B2DEBUG(2, " SpacePoint " << i <<
              " got pointer to PXDCluster with index " << aCluster->getArrayIndex() <<
              " stored in Array " << aCluster->getArrayName())
    } else if (sp->getType() == VXD::SensorInfoBase::SensorType::TEL) {
      B2DEBUG(2, " SpacePoint " << i << " is attached to TelCluster of StoreArray " << sp->getClusterStoreName())
      const TelCluster* aCluster = sp->getRelatedTo<TelCluster>(sp->getClusterStoreName());
      indices.push_back(aCluster->getArrayIndex());
      clusterContainer = aCluster->getArrayName();

      B2DEBUG(2, " SpacePoint " << i <<
              " got pointer to TelCluster with index " << aCluster->getArrayIndex() <<
              " stored in Array " << aCluster->getArrayName())
    } else { B2ERROR(" SpacePoint is of unknown type " << sp->getType()) }


    B2DEBUG(1, "SpacePoint " << i <<
            " got sensorType: " << sp->getType() <<
            ", VxdID: " << VxdID(sp->getVxdID()) <<
            ", storeName for Cluster(says SpacePoint): " << sp->getClusterStoreName() <<
            ", storeName for Cluster(says Cluster): " << clusterContainer)

  }


  B2DEBUG(1, "testGenfitCompatibility: feed the track with spacePoints ported to genfit compatible stuff")
  genfit::AbsTrackRep* trackRep = new genfit::RKTrackRep(211);
  genfit::Track track(trackRep, m_spacePoints[0]->getPosition(), TVector3(23., 42., 5.));
  // feed the track with spacePoints ported to genfit compatible stuff:
  // is rather complicated since the assignment operator is protected:
  std::vector< std::pair<VXD::SensorInfoBase::SensorType, genfit::AbsMeasurement*> > hitOutput;
  for (auto & aSP : m_spacePoints) {
    std::vector<genfit::PlanarMeasurement> tempMeasurements = aSP.getGenfitCompatible();
    for (genfit::PlanarMeasurement & measurement : tempMeasurements) {
      hitOutput.push_back({aSP.getType(), measurement.clone()});
    }
  }

  for (unsigned i = 0; i < hitOutput.size(); i++) {
    track.insertMeasurement(hitOutput[i].second);
    genfit::TrackPoint* point = track.getPointWithMeasurement(i);
    genfit::AbsMeasurement* rawPoint = point->getRawMeasurement();
    B2DEBUG(2, " executing AbsMeasurement " << i << " with detectorID(PXD = 0,SVD=1,TEL=2,VXD=-1) : " << hitOutput[i].first << ":\n")
    point->Print();
    B2DEBUG(2, " converted absMeasurement is of detID: " << rawPoint->getDetId() << ", hitID: " << rawPoint->getHitId())
  }
}
