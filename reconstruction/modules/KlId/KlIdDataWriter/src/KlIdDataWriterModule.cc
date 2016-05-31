/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <reconstruction/modules/KlId/KlIdDataWriter/KlIdDataWriterModule.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <genfit/Track.h>
#include <tracking/dataobjects/TrackClusterSeparation.h>

#include <TTree.h>
#include <TFile.h>
#include <genfit/Exception.h>
#include <cstring>

using namespace Belle2;
using namespace std;

REG_MODULE(KlIdDataWriter);

KlIdDataWriterModule::KlIdDataWriterModule(): Module() // constructor kan nkeine argumente nehmen
{
  setDescription("Used to write flat ntuple for KlId TMVA trainings for both ECL and KLM KlID. Training macro is located in reconstruction/modules/KlId/KlId<ECL><KLM>TMVAExpert.");

  addParam("outPath", m_outPath, "path to put the root file", m_outPath);
}



KlIdDataWriterModule::~KlIdDataWriterModule()
{
}


// --------------------------------------Module----------------------------------------------
void KlIdDataWriterModule::initialize()
{
  // require existence of necessary datastore obj
  StoreArray<KLMCluster>::required();
  StoreArray<MCParticle>::required();
  StoreArray<genfit::Track>::required();
  StoreArray<ECLCluster>::required();

  StoreArray<ECLCluster> eclClusters;
  StoreArray<KLMCluster> klmClusters;
  StoreArray<MCParticle> mcParticles;
  klmClusters.requireRelationTo(mcParticles);
  klmClusters.registerRelationTo(eclClusters);


  // initialize root tree to write stuff into
  m_f = new TFile(m_outPath.c_str(), "recreate");
  m_treeKLM = new TTree("KLMdata", "KLMdata");
  m_treeECL = new TTree("ECLdata", "ECLdata");

  // KLM
  m_treeKLM -> Branch("KLMnCluster",                & m_KLMnCluster);
  m_treeKLM -> Branch("KLMnLayer",                  & m_KLMnLayer);
  m_treeKLM -> Branch("KLMnInnermostlayer",         & m_KLMnInnermostLayer);
  m_treeKLM -> Branch("KLMglobalZ",                 & m_KLMglobalZ);
  m_treeKLM -> Branch("KLMtime",                    & m_KLMtime);
  m_treeKLM -> Branch("KLMinvM",                    & m_KLMinvM);
  m_treeKLM -> Branch("KLMtrackDist",               & m_KLMtrackDist);
  m_treeKLM -> Branch("KLMTruth",                   & m_KLMTruth);
  m_treeKLM -> Branch("KLMdistToNextCl",            & m_KLMnextCluster);
  m_treeKLM -> Branch("KLMenergy",                  & m_KLMenergy);
  m_treeKLM -> Branch("KLMaverageInterClusterDist", & m_KLMavInterClusterDist);
  m_treeKLM -> Branch("KLMhitDepth",                & m_KLMhitDepth);

  m_treeKLM   -> Branch("KLMdistToNextECL",         & m_KLMECLDist);
  m_treeKLM   -> Branch("KLMtrackToECL",            & m_KLMECLtrackDist);
  m_treeKLM   -> Branch("KLMECLEerror",             & m_KLMECLEerror);
  m_treeKLM   -> Branch("KLMECLenergy",             & m_KLMECLE);
  m_treeKLM   -> Branch("KLMECLE9oE25",             & m_KLMECLE9oE25);
  m_treeKLM   -> Branch("KLMECLtiming",             & m_KLMECLTiming);
  m_treeKLM   -> Branch("KLMECLTerror",             & m_KLMECLTerror);
  m_treeKLM   -> Branch("KLMECLdeltaL",             & m_KLMECLdeltaL);
  m_treeKLM   -> Branch("KLMECLmintrackDist",       & m_KLMECLminTrackDist);
  m_treeKLM   -> Branch("KLMBKGProb",               & m_KLMBKGProb);
  m_treeKLM   -> Branch("KLMECLBKGProb",            & m_KLMECLBKGProb);
  m_treeKLM   -> Branch("KLMTrackSepDist",          & m_KLMTrackSepDist);
  m_treeKLM   -> Branch("KLMTrackSepAngle",         & m_KLMTrackSepAngle);


  //ECL
  m_treeECL   -> Branch("ECLenergy",                & m_ECLE);
  m_treeECL   -> Branch("ECLE9oE25",                & m_ECLE9oE25);
  m_treeECL   -> Branch("ECLtiming",                & m_ECLTiming);
  m_treeECL   -> Branch("ECLR",                     & m_ECLR);
  m_treeECL   -> Branch("ECLEerror",                & m_ECLEerror);
  m_treeECL   -> Branch("ECLTruth",                 & m_ECLTruth);
  m_treeECL   -> Branch("ECLdistToTrack",           & m_ECLtrackDist);

  m_treeECL   -> Branch("ECLdeltaL",                & m_ECLdeltaL);
  m_treeECL   -> Branch("ECLmintrackDist",          & m_ECLminTrkDistance);
  m_treeECL   -> Branch("ECLBKGProb",               & m_ECLBKGProb);


  // KLM BKG CLASSIFIER
  // declare vars as they were declared in trainingi, all as floats!
  m_readerBKG -> AddVariable("KLMnCluster",                   &m_KLMnCluster);
  m_readerBKG -> AddVariable("KLMnLayer",                     &m_KLMnLayer);
  m_readerBKG -> AddVariable("KLMnInnermostlayer",            &m_KLMnInnermostLayer);
  m_readerBKG -> AddVariable("KLMglobalZ",                    &m_KLMglobalZ);
  m_readerBKG -> AddVariable("KLMtime",                       &m_KLMtime);
  m_readerBKG -> AddVariable("KLMinvM",                       &m_KLMinvM);
  m_readerBKG -> AddVariable("KLMtrackDist",                  &m_KLMtrackDist);
  m_readerBKG -> AddVariable("KLMdistToNextCl",               &m_KLMnextCluster);
  m_readerBKG -> AddVariable("KLMaverageInterClusterDist",    &m_KLMavInterClusterDist);
  m_readerBKG -> AddVariable("KLMhitDepth",                   &m_KLMhitDepth);
  m_readerBKG -> AddVariable("KLMTrackSepDist",          & m_KLMTrackSepDist);
  m_readerBKG -> AddVariable("KLMTrackSepAngle",         & m_KLMTrackSepAngle);

// KLM-ECL Vars (ECL clusters that are related to KLM clusters)
  m_readerBKG -> AddVariable("KLMdistToNextECL",              &m_KLMECLDist);
  m_readerBKG -> AddVariable("KLMECLenergy",                  &m_KLMECLE);
  m_readerBKG -> AddVariable("KLMECLE9oE25",                  &m_KLMECLE9oE25);
  m_readerBKG -> AddVariable("KLMECLtiming",                  &m_KLMECLTiming);
  m_readerBKG -> AddVariable("KLMECLEerror",                  &m_KLMECLEerror);
  m_readerBKG -> AddVariable("KLMtrackToECL",                 &m_KLMECLtrackDist);
  m_readerBKG -> AddVariable("KLMECLdeltaL",                  &m_KLMECLdeltaL);
  m_readerBKG -> AddVariable("KLMECLmintrackDist",            &m_KLMECLminTrackDist);



  //variables for the classification of associated ECL bkg Prob
  m_readerKLMECL->AddVariable("ECLenergy",                 &m_KLMECLE);
  m_readerKLMECL->AddVariable("ECLE9oE25",                 &m_KLMECLE9oE25);
  m_readerKLMECL->AddVariable("ECLtiming",                 &m_KLMECLTiming);
  m_readerKLMECL->AddVariable("ECLEerror",                 &m_KLMECLEerror);
  m_readerKLMECL->AddVariable("ECLdistToTrack",            &m_KLMECLtrackDist);
  m_readerKLMECL->AddVariable("ECLdeltaL",                 &m_KLMECLdeltaL);
  m_readerKLMECL->AddVariable("ECLmintrackDist",           &m_KLMECLminTrkDistance);

  //variables for the bkg classifier
  //can use same classifier here but have to declare other vars
  m_readerECL->AddVariable("ECLenergy",                 &m_ECLE);
  m_readerECL->AddVariable("ECLE9oE25",                 &m_ECLE9oE25);
  m_readerECL->AddVariable("ECLtiming",                 &m_ECLTiming);
  m_readerECL->AddVariable("ECLEerror",                 &m_ECLEerror);
  m_readerECL->AddVariable("ECLdistToTrack",            &m_ECLtrackDist);
  m_readerECL->AddVariable("ECLdeltaL",                 &m_ECLdeltaL);
  m_readerECL->AddVariable("ECLmintrackDist",           &m_ECLminTrkDistance);


  // load classifier. name, path
  m_readerBKG -> BookMVA(m_BKGClassifierName,  m_BKGClassifierPath);
  m_readerECL -> BookMVA(m_ECLClassifierName,  m_ECLClassifierPath);
  m_readerKLMECL -> BookMVA(m_ECLClassifierName,  m_ECLClassifierPath);


}//init


void KlIdDataWriterModule::beginRun()
{
}

void KlIdDataWriterModule::endRun()
{
}

void KlIdDataWriterModule::event()
{
  // objects needed
  StoreArray<MCParticle> mcParticles;
  StoreArray<KLMCluster> klmClusters;
  StoreArray<genfit::Track> genfitTracks;
  StoreArray<ECLCluster> eclClusters;
  klmClusters.requireRelationTo(mcParticles);

// ------------------ KLM CLUSTERS

  for (const KLMCluster& cluster : klmClusters) {

    // needed later
    const TVector3& cluster_pos = cluster.getClusterPosition();

    // get various KLMCluster vars
    m_KLMglobalZ = cluster.getGlobalPosition().Z();
    m_KLMnCluster = klmClusters.getEntries();
    m_KLMnLayer = cluster.getLayers();
    m_KLMnInnermostLayer = cluster.getInnermostLayer();
    m_KLMtime = cluster.getTime();
    m_KLMinvM = cluster.getMomentum().M2();
    m_KLMenergy = cluster.getMomentum().E();
    m_KLMhitDepth = cluster.getClusterPosition().Mag2();
    // find nearest ecl cluster and calculate angular distance
    m_KLMECLDist =  9999999;
    double closest_ecl_angle_dist = 99999999;
    ECLCluster* closestECLCluster = nullptr;

    for (ECLCluster& eclcluster : eclClusters) {

      const TVector3& eclcluster_pos = eclcluster.getclusterPosition();

      //find nearest cluster by angle measure
      closest_ecl_angle_dist = eclcluster_pos.Angle(cluster_pos);
      if (closest_ecl_angle_dist < m_KLMECLDist) {
        m_KLMECLDist = closest_ecl_angle_dist;
        //turn ref to pointer so you can check for null
        closestECLCluster = &eclcluster;
      }
    }
    //TODO introduce cut to definition of near
    if (!(closestECLCluster == nullptr)) {

      m_KLMECLE      = closestECLCluster->getEnergy();
      m_KLMECLE9oE25 = closestECLCluster->getE9oE25();
      m_KLMECLEerror = closestECLCluster->getErrorEnergy();
      m_KLMECLTerror = closestECLCluster->getErrorTiming();

      // new KLMECL vars
      // names might change
      m_KLMECLdeltaL = closestECLCluster->getTemporaryDeltaL();;
      m_KLMECLminTrackDist = closestECLCluster->getTemporaryMinTrkDistance();
      m_KLMECLTiming = closestECLCluster->getTiming();

      // >> calculate BKG Classifier output for ECL <<
      m_KLMECLBKGProb = m_readerKLMECL -> EvaluateMVA(m_ECLClassifierName);
      // normalize if not errorcode
      if (m_KLMECLBKGProb > -1.) {
        m_KLMECLBKGProb = (m_KLMECLBKGProb + 1) / 2.0;
      } else {m_KLMECLBKGProb = 0;}

      // needed to add those ecl clusters to KLM particles that have a high
      // kl probability and no relation
      cluster.addRelationTo(closestECLCluster);
    } else {

      m_KLMECLdeltaL = -999;
      m_KLMECLminTrackDist = -999;
      m_KLMECLBKGProb = -999;

      m_KLMECLE      = -999;
      m_KLMECLE9oE25 = -999;
      m_KLMECLTiming = -999;
      m_KLMECLTerror = -999;
      m_KLMECLEerror = -999;
    }


    // calculate distance to next cluster and average inter cluster distance
    m_KLMavInterClusterDist = 0;
    m_KLMnextCluster = 999999;
    float new_next_cl_dist = 999999;
    for (const KLMCluster& next_cluster : klmClusters) {

      const TVector3& next_cluster_pos = next_cluster.getClusterPosition();
      const TVector3& clust_distance_vec = next_cluster_pos - cluster_pos;

      new_next_cl_dist = clust_distance_vec.Mag2();
      m_KLMavInterClusterDist = m_KLMavInterClusterDist + new_next_cl_dist;
      if ((new_next_cl_dist < m_KLMnextCluster) and not(new_next_cl_dist == 0)) {
        m_KLMnextCluster = new_next_cl_dist ;
      }
    }// for next_cluster
    if (m_KLMnCluster) {
      m_KLMavInterClusterDist = m_KLMavInterClusterDist / (1. * m_KLMnCluster);
    } else {
      m_KLMavInterClusterDist = 0;
    }


    // find mc truth
    // go thru all particles mothers up to Y4s and check if its a Klong
    m_KLMTruth = 0;
    MCParticle* particle = cluster.getRelatedTo<MCParticle>();
    if (particle == nullptr) {
      m_KLMTruth = -2; // this is the case for beambkg
    } else {
      while (!(particle -> getMother() == nullptr)) {
        if (particle -> getPDG() == 130) {
          m_KLMTruth = 1;
          break;
        }
        particle = particle -> getMother();
      }// while
    }

    // calculate eucl. distance klmcluster <-> nearest track
    // extrapolate genfit trackfit result to their ends and find the
    // closest one
    m_KLMtrackDist = 999999;
    m_KLMECLtrackDist = 999999;
    double trackToECL = 999999; // would have to get the vector twice otherwise
    int       num_points = 0;
    int id_of_last_point = 0;
    for (const genfit::Track& track : genfitTracks) {

      num_points = track.getNumPoints();
      id_of_last_point = num_points - 1;

      // genfit throws an exception if track fit fails ...
      try {

        genfit::MeasuredStateOnPlane state;
        state = track.getFittedState(id_of_last_point);
        // copy: first state on plane is a const ...

        state.extrapolateToPoint(cluster_pos);
        const TVector3& track_pos_klm = state.getPos();

        const TVector3& distance_vec_klm = cluster_pos - track_pos_klm;
        double new_dist_klm = distance_vec_klm.Mag2();

        // overwrite old distance
        if (new_dist_klm < m_KLMtrackDist) {
          m_KLMtrackDist = new_dist_klm;
          // now calculate distance to the ecl cluster we associated by ecl. measure
          state.extrapolateToPoint(closestECLCluster->getclusterPosition());
          const TVector3& track_pos_ecl = state.getPos();
          const TVector3& distance_vec_ecl =
            (closestECLCluster->getclusterPosition()) - track_pos_ecl;

          //inf check
          trackToECL =  distance_vec_ecl.Mag2();

          if ((trackToECL < 999999) & (trackToECL > 0)) {
            m_KLMECLtrackDist = trackToECL;
          }

        }

        //count number of failed fits in event
      } catch (genfit::Exception& e) {
      }// try
    }// for gftrack

    TrackClusterSeparation* trackSep = cluster.getRelatedTo<TrackClusterSeparation>();
    m_KLMTrackSepDist = trackSep->getDistance();
    m_KLMTrackSepAngle = trackSep->getTrackAngle();



    // >> calculate BKG Classifier output for KLM <<
    m_KLMBKGProb = m_readerBKG -> EvaluateMVA(m_BKGClassifierName);

    // normalize if not errorcode
    if (m_KLMBKGProb > -1.) {
      m_KLMBKGProb = (m_KLMBKGProb + 1) / 2.0;
    } else {m_KLMBKGProb = 0; }


    m_treeKLM -> Fill();
  }// for klmcluster in klmclusters

// ---------------   ECL CLUSTERS

  // loop thru eclclusters in event and calculate vars
  for (const ECLCluster& cluster : eclClusters) {

    // get various ECLCluster vars from getters

    // new vasr
    m_ECLminTrkDistance = cluster.getTemporaryMinTrkDistance();
    m_ECLdeltaL = cluster.getTemporaryDeltaL();


    m_ECLE = cluster.getEnergy();
    m_ECLE9oE25 = cluster.getE9oE25();
    m_ECLTiming = cluster.getTiming();
    m_ECLR = cluster.getR();
    m_ECLEerror = cluster.getErrorEnergy();
    const TVector3& cluster_pos = cluster.getclusterPosition();


    // find mc truth
    // go thru all particles mothers up to Y4s and check if its a Klong
    m_ECLTruth = 0;
    MCParticle* particle = cluster.getRelatedTo<MCParticle>();
    if (particle == nullptr) {
      m_ECLTruth = -2; // this is the case for beambkg
    } else {
      while (!(particle -> getMother() == nullptr)) {
        if (particle -> getPDG() == 130) {
          m_ECLTruth = 1;
          break;
        }
        particle = particle -> getMother();
      }// while
    }

    m_ECLtrackDist = 999999;
    int       num_points = 0;
    int id_of_last_point = 0;
//    m_nFailedTrackFits = 0;
    for (const genfit::Track& track : genfitTracks) {

      num_points = track.getNumPoints();
      id_of_last_point = num_points - 1;

      // genfit throws an exception if track fit fails ...
      try {

        genfit::MeasuredStateOnPlane state;
        state = track.getFittedState(id_of_last_point);
        // copy: first state on plane is a const ...

        state.extrapolateToPoint(cluster_pos);
        const TVector3& track_pos_ecl = state.getPos();

        const TVector3& distance_vec_ecl = cluster_pos - track_pos_ecl;
        double new_dist_ecl = distance_vec_ecl.Mag2();

        // overwrite old distance
        if (new_dist_ecl < m_ECLtrackDist) {
          m_ECLtrackDist = new_dist_ecl;
        }

      } catch (genfit::Exception& e) {
      }// try
    }// for gftrack


    // get bkg classification from bvkg classifier...
    m_ECLBKGProb = m_readerECL -> EvaluateMVA(m_ECLClassifierName);
    // normalize if not errorcode
    if (m_ECLBKGProb > -1.) {
      m_ECLBKGProb = (m_ECLBKGProb + 1) / 2.0;
    }


    // finally fill tree
    m_treeECL -> Fill();


  }// for ecl cluster in clusters

} // event


void KlIdDataWriterModule::terminate()
{

  // close root files
  m_f    -> cd(); // neccesarry (frame work has another open tree)
  m_treeKLM -> Write();
  m_treeECL -> Write();
  m_f    -> Close();

  delete m_readerBKG;
  delete m_readerECL;
  delete m_readerKLMECL;
}











