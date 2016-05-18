/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <reconstruction/modules/KlId/KlIdKLMTMVAExpert/KlIdKLMTMVAExpertModule.h>
#include <reconstruction/dataobjects/KlId.h>
#include <framework/datastore/StoreArray.h>
#include <framework/utilities/FileSystem.h>

#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <genfit/Track.h>

#include <genfit/Exception.h>
#include <cstring>

#include <TMVA/Reader.h>


using namespace Belle2;
using namespace std;

REG_MODULE(KlIdKLMTMVAExpert);

KlIdKLMTMVAExpertModule::KlIdKLMTMVAExpertModule(): Module()
{
  setDescription("Use to calculate KlId for each KLMCluster.");


  // classifier paths

  /** Path were to find the .xml file containing the classifier trainings. */
  addParam("ID ClassifierPath",
           m_IDClassifierPath,
           "Path to the .xml weight file containing the fitted classifier.",
           m_IDClassifierPath);

  /** Path were to find the .xml file containing the classifier trainings. */
  addParam("BKGClassifierPath",
           m_BKGClassifierPath,
           "Path to the .xml weight file containing the fitted classifier.",
           m_BKGClassifierPath);


  /** Path were to find the .xml file containing the classifier trainings. */
  addParam("ECLClassifierPath",
           m_ECLClassifierPath,
           "Path to the .xml weight file containing the fitted classifier.",
           m_ECLClassifierPath);


  // classifier names

  /** Name of the IDclassifier as it was declared in training file.  */
  addParam("ID ClassifierName",
           m_IDClassifierName,
           "Name of ID classifier.",
           m_IDClassifierName);

  /** Name of the BKGclassifier as it was declared in training file.  */
  addParam("BKG ClassifierName",
           m_BKGClassifierName,
           "Name of  BKG classifier.",
           m_BKGClassifierName);

  /** Name of the ECLclassifier as it was declared in training file.  */
  addParam("ECL ClassifierName",
           m_ECLClassifierName,
           "Name of ECL classifier.",
           m_ECLClassifierName);

}



KlIdKLMTMVAExpertModule::~KlIdKLMTMVAExpertModule()
{
}


// --------------------------------------Module----------------------------------------------
void KlIdKLMTMVAExpertModule::initialize()
{
  // require existence of necessary datastore obj
  StoreArray<KLMCluster>::required();
  StoreArray<genfit::Track>::required();
  StoreArray<ECLCluster>::required();

  StoreArray<ECLCluster> eclClusters;
  StoreArray<KLMCluster> klmClusters;
  klmClusters.registerRelationTo(eclClusters);

  StoreArray<KlId>::registerTransient();//Transient
  //klids.registerInDataStore();

  StoreArray<KlId> klids;
  klmClusters.registerRelationTo(klids);

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
  m_readerBKG -> AddVariable("KLMshape",                      &m_KLMshape);
  m_readerBKG -> AddVariable("KLMaverageInterClusterDist",    &m_KLMavInterClusterDist);
  m_readerBKG -> AddVariable("KLMhitDepth",                   &m_KLMhitDepth);
  // KLM-ECL Vars (ECL clusters that are related to KLM clusters)
  m_readerBKG -> AddVariable("KLMdistToNextECL",              &m_KLMECLDist);
  m_readerBKG -> AddVariable("KLMECLenergy",                  &m_KLMECLE);
  m_readerBKG -> AddVariable("KLMECLE9oE25",                  &m_KLMECLE9oE25);
  m_readerBKG -> AddVariable("KLMECLtiming",                  &m_KLMECLTiming);
  m_readerBKG -> AddVariable("KLMECLEerror",                  &m_KLMECLEerror);
  m_readerBKG -> AddVariable("KLMtrackToECL",                 &m_KLMtrackToECL);
  m_readerBKG -> AddVariable("KLMECLdeltaL",                  &m_KLMECLdeltaL);
  m_readerBKG -> AddVariable("KLMECLmintrackDist",            &m_KLMECLminTrackDist);


  // KLM ID CLASSIFIER
  m_readerID -> AddVariable("KLMnCluster",                   &m_KLMnCluster);
  m_readerID -> AddVariable("KLMnLayer",                     &m_KLMnLayer);
  m_readerID -> AddVariable("KLMnInnermostlayer",            &m_KLMnInnermostLayer);
  m_readerID -> AddVariable("KLMglobalZ",                    &m_KLMglobalZ);
  m_readerID -> AddVariable("KLMtime",                       &m_KLMtime);
  m_readerID -> AddVariable("KLMinvM",                       &m_KLMinvM);
  m_readerID -> AddVariable("KLMtrackDist",                  &m_KLMtrackDist);
  m_readerID -> AddVariable("KLMdistToNextCl",               &m_KLMnextCluster);
  m_readerID -> AddVariable("KLMshape",                      &m_KLMshape);
  m_readerID -> AddVariable("KLMaverageInterClusterDist",    &m_KLMavInterClusterDist);
  m_readerID -> AddVariable("KLMhitDepth",                   &m_KLMhitDepth);
  m_readerID -> AddVariable("KLMBKGProb"                 , &m_KLMBKGProb);
  // KLM-ECL Vars (ECL clusters that are related to KLM clusters)
  m_readerID -> AddVariable("KLMdistToNextECL",              &m_KLMECLDist);
  m_readerID -> AddVariable("KLMECLenergy",                  &m_KLMECLE);
  m_readerID -> AddVariable("KLMECLE9oE25",                  &m_KLMECLE9oE25);
  m_readerID -> AddVariable("KLMECLtiming",                  &m_KLMECLTiming);
  m_readerID -> AddVariable("KLMECLEerror",                  &m_KLMECLEerror);
  m_readerID -> AddVariable("KLMtrackToECL",                 &m_KLMtrackToECL);
  m_readerID -> AddVariable("KLMECLdeltaL",                  &m_KLMECLdeltaL);
  m_readerID -> AddVariable("KLMECLmintrackDist",            &m_KLMECLminTrackDist);
  m_readerID -> AddVariable("KLMECLBKGProb"                 , &m_KLMECLBKGProb);


  //ECL BKG CLASSIFIER
  m_readerECL->AddVariable("ECLenergy",                 &m_KLMECLE);
  m_readerECL->AddVariable("ECLE9oE25",                 &m_KLMECLE9oE25);
  m_readerECL->AddVariable("ECLtiming",                 &m_KLMECLTiming);
  m_readerECL->AddVariable("ECLEerror",                 &m_KLMECLEerror);
  m_readerECL->AddVariable("ECLdistToTrack",            &m_KLMtrackToECL);
  m_readerECL->AddVariable("ECLdeltaL",                 &m_KLMECLdeltaL);
  m_readerECL->AddVariable("ECLmintrackDist",           &m_KLMECLminTrackDist);


  // load classifier. name, path
  m_readerBKG -> BookMVA(m_BKGClassifierName,  m_BKGClassifierPath);
  m_readerID  -> BookMVA(m_IDClassifierName,   m_IDClassifierPath);
  m_readerECL -> BookMVA(m_ECLClassifierName,  m_ECLClassifierPath);
}


void KlIdKLMTMVAExpertModule::beginRun()
{
}

void KlIdKLMTMVAExpertModule::endRun()
{
}

void KlIdKLMTMVAExpertModule::event()
{
  /**
  Loop thru clusters.
  Calculate variables and feed to classifier
  to get Kl Id.
  Then write Kl Id to KLMCLuster.
  */

  StoreArray<KLMCluster> klmClusters;
  StoreArray<genfit::Track> genfitTracks;
  StoreArray<ECLCluster> eclClusters;
  StoreArray<KlId> KlIds;

  //overwritten at the end of the cluster loop
  KlId* klid = nullptr;
  double IDMVAOut;

  // loop thru clusters in event and classify
  for (KLMCluster& cluster : klmClusters) {

    const TVector3& cluster_pos = cluster.getClusterPosition();

    // get various KLMCluster vars
    m_KLMglobalZ         = cluster.getGlobalPosition().Z();
    m_KLMnCluster        = klmClusters.getEntries();
    m_KLMnLayer          = cluster.getLayers();
    m_KLMnInnermostLayer = cluster.getInnermostLayer();
    m_KLMtime            = cluster.getTime();
    m_KLMinvM            = cluster.getMomentum().M2();
    m_KLMhitDepth        = cluster.getClusterPosition().Mag2();

    // some measure of cluster shape
    if (m_KLMnInnermostLayer > 0) {
      m_KLMshape = m_KLMnLayer / (1.*m_KLMnInnermostLayer);
    } else {
      m_KLMshape = 0;
    }

    // find nearest ecl cluster and calculate distance
    m_KLMECLDist =  9999999;
    double closest_ecl_angle_dist = 99999999;
    ECLCluster* closestECLCluster = nullptr;
    for (ECLCluster& eclcluster : eclClusters) {

      const TVector3& eclcluster_pos = eclcluster.getclusterPosition();
      closest_ecl_angle_dist         = eclcluster_pos.Angle(cluster_pos);

      if (closest_ecl_angle_dist < m_KLMECLDist) {
        m_KLMECLDist = closest_ecl_angle_dist;
        //turn ref to pointer so you can check for null
        closestECLCluster = &eclcluster;
      }
    }

    if (!(closestECLCluster == nullptr)) {
      m_KLMECLE      = closestECLCluster -> getEnergy();
      m_KLMECLE9oE25 = closestECLCluster -> getE9oE25();
      m_KLMECLTerror = closestECLCluster -> getErrorTiming();
      m_KLMECLTiming = closestECLCluster -> getTiming();
      m_KLMECLEerror = closestECLCluster -> getErrorEnergy();
      m_KLMECLdeltaL = closestECLCluster->getTemporaryDeltaL();
      m_KLMECLminTrackDist = closestECLCluster->getTemporaryMinTrkDistance();


      // m_KLMECLBKGProb = closestECLCluster->getBKGProb();
      // >> calculate BKG Classifier output for ECL <<
      m_KLMECLBKGProb = m_readerECL -> EvaluateMVA(m_ECLClassifierName);

      // normalize if not errorcode
      if (m_KLMECLBKGProb > -1.) {
        m_KLMECLBKGProb = (m_KLMECLBKGProb + 1) / 2.0;
      } else {m_KLMECLBKGProb = 0;}


      // the matched ecl cluster gets this relation to avoid double counting
      cluster.addRelationTo(closestECLCluster);
    } else {
      m_KLMECLdeltaL       = -999;
      m_KLMECLminTrackDist = -999;
      m_KLMECLBKGProb      = -999;

      m_KLMECLE      = -999;
      m_KLMECLE9oE25 = -999;
      m_KLMECLTiming = -999;
      m_KLMECLTerror = -999;
      m_KLMECLEerror = -999;
    }

    // calculate distance to next cluster
    m_KLMavInterClusterDist = 0;
    m_KLMnextCluster        = 999999;
    float new_next_cl_dist  = 999999;
    for (const KLMCluster& next_cluster : klmClusters) {

      const TVector3& next_cluster_pos   = next_cluster.getClusterPosition();
      const TVector3& clust_distance_vec = next_cluster_pos - cluster_pos;
      new_next_cl_dist                   = clust_distance_vec.Mag2();
      m_KLMavInterClusterDist            = m_KLMavInterClusterDist + new_next_cl_dist;

      if ((new_next_cl_dist < m_KLMnextCluster) and not(new_next_cl_dist == 0)) {
        m_KLMnextCluster = new_next_cl_dist ;
      }
    }// for next_cluster

    if (m_KLMnCluster) {
      m_KLMavInterClusterDist = m_KLMavInterClusterDist / (1. * m_KLMnCluster);
    } else {
      m_KLMavInterClusterDist = 0;
    }


    // calculate distance cluster <-> nearest track
    // extrapolate genfit trackfit result to their ends and find the
    // closest one
    m_KLMtrackDist       = 999999;
    m_KLMtrackToECL      = 999999;
    int       num_points = 0;
    int id_of_last_point = 0;
    for (const genfit::Track& track : genfitTracks) {

      num_points       = track.getNumPoints();
      id_of_last_point = num_points - 1;

      // genfit throws an exception if track fit fails ...
      try {

        genfit::MeasuredStateOnPlane state;
        state = track.getFittedState(id_of_last_point);
        // copy: first state on plane is a const ...

        state.extrapolateToPoint(cluster_pos);
        const TVector3& track_pos_klm    = state.getPos();

        const TVector3& distance_vec_klm = cluster_pos - track_pos_klm;
        double new_dist_klm              = distance_vec_klm.Mag2();

        // overwrite old distance
        if (new_dist_klm < m_KLMtrackDist) {
          m_KLMtrackDist = new_dist_klm;
          // now calculate distance to the ecl cluster we associated by ecl. measure
          state.extrapolateToPoint(closestECLCluster->getclusterPosition());
          const TVector3& track_pos_ecl    = state.getPos();
          const TVector3& distance_vec_ecl =
            (closestECLCluster->getclusterPosition()) - track_pos_ecl;
          m_KLMtrackToECL = distance_vec_ecl.Mag2();
        }

      } catch (genfit::Exception& e) {
      }// try
    }// for gftrack


    // >> calculate BKG Classifier output for KLM <<
    m_KLMBKGProb = m_readerBKG -> EvaluateMVA(m_BKGClassifierName);

    // normalize if not errorcode
    if (m_KLMBKGProb > -1.) {
      m_KLMBKGProb = (m_KLMBKGProb + 1) / 2.0;
    } else {m_KLMBKGProb = 0; }
    //m_KLMBKGProb = cluster.getBKGProb();

    // >> calculate Kl Id <<
    IDMVAOut = m_readerID -> EvaluateMVA(m_IDClassifierName);

    // normalize if not errorcode
    if (IDMVAOut > -1.) {
      IDMVAOut = (IDMVAOut + 1) / 2.0;
    }


    // write id into cluster
    //cluster.setKlId(IDMVAOut);

    // KlId, bkg prob, KLM, ECL
    klid = KlIds.appendNew(IDMVAOut, m_KLMBKGProb, 1, 0);
    cluster.addRelationTo(klid);

  }// for cluster in clusters
} // event


void KlIdKLMTMVAExpertModule::terminate()
{
  delete m_readerID;
  delete m_readerBKG;
  delete m_readerECL;
}











