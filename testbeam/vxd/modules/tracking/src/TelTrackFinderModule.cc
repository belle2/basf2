/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/modules/tracking/TelTrackFinderModule.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <framework/gearbox/Const.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TelTrackFinder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TelTrackFinderModule::TelTrackFinderModule() : Module(), m_vxdGeometry(VXD::GeoCache::getInstance()),
  m_mintelLadder(2), m_maxtelLadder(3)
{
  // Set module properties
  setDescription("find a track candidate including telescope clusters by taking a genfit::Track and projecting it to the telescope planes");

  // Parameter definitions
  addParam("inputTracksName", m_inputTracksName, "name of store array with input genfit::Tracks", std::string(""));
  addParam("outputTrackCandsName", m_outputTrackCandsName, "name of store array with output genfit::TrackCands", std::string(""));
  addParam("inputClustersName", m_inputClustersName, "name of store array which stores the telescope clusters", std::string(""));
  addParam("distanceCut", m_distanceCut, "cut on clusters distance to track intersection in units of the fit uncertainty [sigma]");
  addParam("minTelLadder", m_mintelLadder, "defines lower bound of the ladder numbers (NOT Layer) used for the telescope", 2);
  addParam("maxTelLadder", m_maxtelLadder, "defines upper bound of the ladder numbers (NOT Layer) used for the telescope", 3);
  addParam("minTelLayers", m_minTelLayers, "minimum number of telescope layers with valid hits required for a new track cand");

}

TelTrackFinderModule::~TelTrackFinderModule()
{
}

void TelTrackFinderModule::initialize()
{
  //the input tracks
  m_tracks.isRequired(m_inputTracksName);

  //the telescope clusters
  m_clusters.isRequired(m_inputClustersName);


  //the output track candidates
  m_trackCands.registerInDataStore(m_outputTrackCandsName);
  m_trackCands.create();
}

void TelTrackFinderModule::beginRun()
{
}

void TelTrackFinderModule::event()
{

  B2DEBUG(1, "New event with number of genfit::Tracks: " <<  m_tracks.getEntries() << std::endl);
  //loop over all the tracks
  for (int itrack = 0; itrack < m_tracks.getEntries(); itrack++) {
    //get the intersections for each Tel layer


    std::vector<int> clusterindizes;

    //loop over the telescope planes to find the intersections with this track
    std::vector<VxdID> sensorlist = m_vxdGeometry.getListOfSensors();
    int ntellayer = 0; //count the number of telescope ladders where hits have been found
    for (int isen = 0; isen < (int)sensorlist.size(); isen++) {
      VxdID theId = sensorlist[isen];

      VXD::SensorInfoBase sensorInfo = m_vxdGeometry.getSensorInfo(theId);

      if (sensorInfo.getType() != VXD::SensorInfoBase::TEL) continue; //also the szintlators will pass that cut!!
      //only select the ladder assigned to the telescope (WARNING: currently hard coded!)
      if (theId.getLadderNumber() < m_mintelLadder || theId.getLadderNumber() > m_maxtelLadder) continue;

      //check if a good intersection has been found
      bool isgood = false;
      double du(0), dv(0);
      TVector3 intersec = getTrackIntersec(theId, *(m_tracks[itrack]), isgood, du, dv);
      if (!isgood) continue;

      B2DEBUG(1, "The found intersection " << intersec.X() << " " << intersec.Y() << " " << intersec.Z());

      int nin = clusterindizes.size();
      findClusterCands(clusterindizes, theId, intersec, du, dv);
      int nout = clusterindizes.size();
      B2DEBUG(1, "Found " << (nout - nin) << " hits on sensor layer " << (std::string)theId);

      //if the number of clusters changed for that sensor
      if (nin != nout) ntellayer++;
    }//end isen

    //generate a new track cand by adding the new clusters to the old track cand
    if (ntellayer >= m_minTelLayers) {
      RelationVector<genfit::TrackCand> trackcands_from_track
        = DataStore::getRelationsToObj<genfit::TrackCand>(m_tracks[itrack], "ALL");  // there should be only one!
      if (trackcands_from_track.size() == 1) addNewTrackCand(clusterindizes, *(trackcands_from_track[0]));
    } else {
      B2DEBUG(1, "No track candidate added as only for " << ntellayer << " Layers hits have been found!");
    }
  }

}



void TelTrackFinderModule::endRun()
{
}

void TelTrackFinderModule::terminate()
{
}


TVector3 TelTrackFinderModule::getTrackIntersec(VxdID sensorID, const genfit::Track& track, bool& isgood, double& du, double& dv)
{

  //will be set true if the intersect was found
  isgood = false;
  //fit uncertainties
  du = 0.;
  dv = 0.;

  genfit::MeasuredStateOnPlane gfTrackState = track.getFittedState();

  VXD::SensorInfoBase sensorInfo = m_vxdGeometry.getSensorInfo(sensorID);


  //adopted (aka stolen) from tracking/modules/pxdClusterRescue/PXDClusterRescueROIModule
  TVector3 intersec(-999. , -999. , -999.);
  try {
    // get sensor plane
    TVector3 zeroVec(0, 0, 0);
    TVector3 uVec(1, 0, 0);
    TVector3 vVec(0, 1, 0);

    genfit::DetPlane* sensorPlane = new genfit::DetPlane();
    sensorPlane->setO(sensorInfo.pointToGlobal(zeroVec));
    sensorPlane->setUV(sensorInfo.vectorToGlobal(uVec), sensorInfo.vectorToGlobal(vVec));

    //boost pointer (will be deleted automatically ?!?!?)
    genfit::SharedPlanePtr sensorPlaneSptr(sensorPlane);

    // do extrapolation
    gfTrackState.extrapolateToPlane(sensorPlaneSptr);
  } catch (genfit::Exception& gfException) {
    B2WARNING("Fitting failed: " << gfException.getExcString());
    return intersec;
  }

  //local position
  intersec = sensorInfo.pointToLocal(gfTrackState.getPos());

  // get intersection point in local coordinates with covariance matrix
  TMatrixDSym covMatrix = gfTrackState.getCov(); // 5D with elements q/p,u',v',u,v in plane system

  // get the uncertainties from the covariance matrix
  du = std::sqrt(covMatrix(3, 3));
  dv = std::sqrt(covMatrix(4, 4));

  B2DEBUG(1, "du = " << du << " dv = " << dv);

  isgood = true;
  return intersec;
}



void TelTrackFinderModule::findClusterCands(std::vector<int>& clusterindizes, VxdID avxdid, TVector3& intersec, double du,
                                            double dv)
{
  B2DEBUG(1, "number of Clusters in the array: " << m_clusters.getEntries());

  for (int iclus = 0; iclus < m_clusters.getEntries(); iclus++) {
    //check the vxd id since the intersection is only valid for a specific vxdid
    VxdID thisid = m_clusters[iclus]->getSensorID();
    if (thisid != avxdid) continue;

    //calc difference between cluster and intersection all local coordinates
    TVector3 diff(m_clusters[iclus]->getU(), m_clusters[iclus]->getV(), 0.);
    diff -= intersec;

    B2DEBUG(1, "distance in terms of fit uncertainty: diff_u = " << diff.X() / du << " (du=" << du << ") diff_v = "
            << diff.Y() / dv << " (dv=" << dv << ")");
    //multiple clusters per layer are possible but genfit should be able to handle that!?!?
    if (fabs(diff.X() / du) < m_distanceCut && fabs(diff.Y() / dv) < m_distanceCut) clusterindizes.push_back(iclus);
  }//end iclus

}


void TelTrackFinderModule::addNewTrackCand(const  std::vector<int>& telHitIndizes, const genfit::TrackCand& trackCand)
{

  B2DEBUG(1, "initial track cand has " << trackCand.getNHits() << " hits");

  //create the new track cand by copying the old
  genfit::TrackCand newCand(trackCand);

  //add all the telescope hits
  for (int ihit = 0; ihit < (int)telHitIndizes.size(); ihit++) {
    // TelCluster class does not seem to store the time!?!?
    //float time = m_inputClusters[telHitIndizes[ihit]]->getGlobalTime();
    newCand.addHit(Const::TEST, telHitIndizes[ihit]);
  }

  //not sure if that one makes sense as no sorting parameter has been specified!?!?
  newCand.sortHits();

  B2DEBUG(1, "obtained track cand has " << newCand.getNHits() << " hits");

  m_trackCands.appendNew(newCand);

}
