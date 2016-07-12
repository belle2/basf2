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
#include <tracking/dataobjects/TrackClusterSeparation.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/Exception.h>
#include <cstring>

#include <TMVA/Reader.h>

#include "reconstruction/modules/KlId/KlIdKLMTMVAExpert/helperFunctions.h"

using namespace KlIdHelpers;
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
           m_ECLBKGClassifierPath,
           "Path to the .xml weight file containing the fitted classifier.",
           m_ECLBKGClassifierPath);


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
  StoreArray<RecoTrack>::required();
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
  m_readerBKG -> AddVariable("KLMaverageInterClusterDist",    &m_KLMavInterClusterDist);
  m_readerBKG -> AddVariable("KLMhitDepth",                   &m_KLMhitDepth);
  m_readerBKG -> AddVariable("KLMTrackSepDist",          & m_KLMTrackSepDist);
  m_readerBKG -> AddVariable("KLMTrackSepAngle",         & m_KLMTrackSepAngle);

  m_readerBKG -> AddVariable("KLMInitialtrackSepAngle",  &m_KLMInitialTrackSepAngle);
  m_readerBKG -> AddVariable("KLMTrackRotationAngle",    &m_KLMTrackRotationAngle);
  m_readerBKG -> AddVariable("KLMTrackClusterSepAngle",  &m_KLMTrackClusterSepAngle);


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
  m_readerID -> AddVariable("KLMaverageInterClusterDist",    &m_KLMavInterClusterDist);
  m_readerID -> AddVariable("KLMhitDepth",                   &m_KLMhitDepth);
  m_readerID -> AddVariable("KLMBKGProb"                 , &m_KLMBKGProb);
  m_readerID -> AddVariable("KLMECLBKGProb"                 , &m_KLMECLBKGProb);
  m_readerID -> AddVariable("KLMTrackSepDist",          & m_KLMTrackSepDist);
  m_readerID -> AddVariable("KLMTrackSepAngle",         & m_KLMTrackSepAngle);

  m_readerID -> AddVariable("KLMInitialtrackSepAngle",  &m_KLMInitialTrackSepAngle);
  m_readerID -> AddVariable("KLMTrackRotationAngle",    &m_KLMTrackRotationAngle);
  m_readerID -> AddVariable("KLMTrackClusterSepAngle",  &m_KLMTrackClusterSepAngle);


  // KLM-ECL Vars (ECL clusters that are related to KLM clusters)
  m_readerID -> AddVariable("KLMdistToNextECL",              &m_KLMECLDist);
  m_readerID -> AddVariable("KLMECLenergy",                  &m_KLMECLE);
  m_readerID -> AddVariable("KLMECLE9oE25",                  &m_KLMECLE9oE25);
  m_readerID -> AddVariable("KLMECLtiming",                  &m_KLMECLTiming);
  m_readerID -> AddVariable("KLMECLEerror",                  &m_KLMECLEerror);
  m_readerID -> AddVariable("KLMtrackToECL",                 &m_KLMtrackToECL);
  m_readerID -> AddVariable("KLMECLdeltaL",                  &m_KLMECLdeltaL);
  m_readerID -> AddVariable("KLMECLmintrackDist",            &m_KLMECLminTrackDist);


  //ECL BKG CLASSIFIER
  m_readerECLBKG->AddVariable("ECLenergy",                 &m_KLMECLE);
  m_readerECLBKG->AddVariable("ECLE9oE25",                 &m_KLMECLE9oE25);
  m_readerECLBKG->AddVariable("ECLtiming",                 &m_KLMECLTiming);
  m_readerECLBKG->AddVariable("ECLEerror",                 &m_KLMECLEerror);
  m_readerECLBKG->AddVariable("ECLdistToTrack",            &m_KLMtrackToECL);
  m_readerECLBKG->AddVariable("ECLdeltaL",                 &m_KLMECLdeltaL);
  m_readerECLBKG->AddVariable("ECLmintrackDist",           &m_KLMECLminTrackDist);


  // load classifier. name, path
  m_readerBKG -> BookMVA(m_BKGClassifierName,  m_BKGClassifierPath);
  m_readerID  -> BookMVA(m_IDClassifierName,   m_IDClassifierPath);
  m_readerECLBKG -> BookMVA(m_ECLClassifierName,  m_ECLBKGClassifierPath);
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
  StoreArray<RecoTrack> genfitTracks;
  StoreArray<ECLCluster> eclClusters;
  StoreArray<KlId> KlIds;

  //overwritten at the end of the cluster loop
  KlId* klid = nullptr;
  double IDMVAOut;

  // loop thru clusters in event and classify
  for (KLMCluster& cluster : klmClusters) {

    const TVector3& clusterPos = cluster.getClusterPosition();

    // get various KLMCluster vars
    m_KLMglobalZ         = clusterPos.Z();
    m_KLMnCluster        = klmClusters.getEntries();
    m_KLMnLayer          = cluster.getLayers();
    m_KLMnInnermostLayer = cluster.getInnermostLayer();
    m_KLMtime            = cluster.getTime();
    m_KLMinvM            = cluster.getMomentum().M2();
    m_KLMhitDepth        = cluster.getClusterPosition().Mag2();

    // find nearest ecl cluster and calculate distance
    pair<ECLCluster*, double> closestECLAndDist = findClosestECLCluster(clusterPos);
    ECLCluster* closestECLCluster = get<0>(closestECLAndDist);
    m_KLMECLDist = get<1>(closestECLAndDist);


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
      m_KLMECLBKGProb = m_readerECLBKG -> EvaluateMVA(m_ECLClassifierName);

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
    tuple<const KLMCluster*, double, double> closestKLMAndDist = findClosestKLMCluster(clusterPos);
    m_KLMnextCluster = get<1>(closestKLMAndDist);
    m_KLMavInterClusterDist = get<2>(closestKLMAndDist);


    // calculate eucl. distance klmcluster <-> nearest track
    // extrapolate genfit trackfit result to their ends and find the
    tuple<RecoTrack*, double, const TVector3*> closestTrackAndDistance
      = findClosestTrack(clusterPos);
    m_KLMtrackDist = get<1>(closestTrackAndDistance);
    const TVector3* poca = get<2>(closestTrackAndDistance);

    if (poca and closestECLCluster) {
      const TVector3& trackECLClusterDist = closestECLCluster->getPosition() - *poca;
      m_KLMtrackToECL = trackECLClusterDist.Mag2();
    } else {
      m_KLMtrackToECL = -999;
    }


    TrackClusterSeparation* trackSep = cluster.getRelatedTo<TrackClusterSeparation>();
    m_KLMTrackSepDist         = trackSep->getDistance();
    m_KLMTrackSepAngle        = trackSep->getTrackClusterAngle();

    m_KLMInitialTrackSepAngle = trackSep->getTrackClusterInitialSeparationAngle();
    m_KLMTrackRotationAngle   = trackSep->getTrackRotationAngle();
    m_KLMTrackClusterSepAngle = trackSep->getTrackClusterSeparationAngle();


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
  delete m_readerECLBKG;
}





