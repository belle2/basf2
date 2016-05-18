/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <reconstruction/modules/KlId/KlIdECLTMVAExpert/KlIdECLTMVAExpertModule.h>
#include <reconstruction/dataobjects/KlId.h>

#include <framework/datastore/StoreArray.h>
#include <framework/utilities/FileSystem.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <genfit/Track.h>

#include <genfit/Exception.h>
#include <cstring>

#include <TMVA/Reader.h>


using namespace Belle2;
using namespace std;

REG_MODULE(KlIdECLTMVAExpert);

KlIdECLTMVAExpertModule::KlIdECLTMVAExpertModule(): Module()
{
  setDescription("Use to calculate KlId for each ECLCluster.");

  /** Path were to find the .xml file containing the classifier trainings. */
  addParam("ClassifierPath",
           m_classifierPath,
           "Path to the .xml weight file containing the trained classifier.",
           m_classifierPath);


  /** Path were to find the .xml file containing the classifier trainings. */
  addParam("BKGClassifierPath",
           m_BKGClassifierPath,
           "Path to the .xml weight file containing the trained classifier.",
           m_BKGClassifierPath);

  /** Name of the classifier as it was declared in training file.*/
  addParam("ClassifierName",
           m_classifierName,
           "Name of classifier.",
           m_classifierName);

  /** Name of the classifier as it was declared in training file.*/
  addParam("BKGClassifierName",
           m_BKGClassifierName,
           "Name of BKG classifier.",
           m_BKGClassifierName);


}



KlIdECLTMVAExpertModule::~KlIdECLTMVAExpertModule()
{
}


// --------------------------------------Module----------------------------------------------
void KlIdECLTMVAExpertModule::initialize()
{
  StoreArray<ECLCluster>::required();

  StoreArray<KlId> klids;

  StoreArray<ECLCluster> eclClusters;
  eclClusters.registerRelationTo(klids);
  //run KLMTMVA Expert first

  //variables for the bkg classifier
  m_readerBKG->AddVariable("ECLenergy",                 &m_ECLE);
  m_readerBKG->AddVariable("ECLE9oE25",                 &m_ECLE9oE25);
  m_readerBKG->AddVariable("ECLtiming",                 &m_ECLTiming);
  m_readerBKG->AddVariable("ECLEerror",                 &m_ECLEerror);
  m_readerBKG->AddVariable("ECLdistToTrack",            &m_ECLtrackDist);
  m_readerBKG->AddVariable("ECLdeltaL",                 &m_ECLdeltaL);
  m_readerBKG->AddVariable("ECLmintrackDist",           &m_ECLminTrkDistance);

  // variables for the id classifier
  m_reader->AddVariable("ECLenergy",                 &m_ECLE);
  m_reader->AddVariable("ECLE9oE25",                 &m_ECLE9oE25);
  m_reader->AddVariable("ECLtiming",                 &m_ECLTiming);
  m_reader->AddVariable("ECLEerror",                 &m_ECLEerror);
  m_reader->AddVariable("ECLdistToTrack",            &m_ECLtrackDist);
  m_reader->AddVariable("ECLdeltaL",                 &m_ECLdeltaL);
  m_reader->AddVariable("ECLmintrackDist",           &m_ECLminTrkDistance);
  m_reader->AddVariable("ECLBKGProb",           &m_ECLBKGProb);

  m_reader->BookMVA(m_classifierName, m_classifierPath);
  m_readerBKG->BookMVA(m_BKGClassifierName, m_BKGClassifierPath);
}


void KlIdECLTMVAExpertModule::beginRun()
{
}

void KlIdECLTMVAExpertModule::endRun()
{
}

void KlIdECLTMVAExpertModule::event()
{
  /**
  Loop thru clusters.
  Calculate variables and feed to classifier
  to get Kl Id.
  Then write Kl Id to ECLCluster.
  But default training contains "FastBDT".
  */

  StoreArray<genfit::Track> genfitTracks;
  StoreArray<ECLCluster> eclClusters;
  StoreArray<KlId> klids;


  //overwritten at the end of the cluster loop
  double MVAOut;
  KlId* klid = nullptr;

  // loop thru clusters in event and classify
  for (ECLCluster& cluster : eclClusters) {


    // if cluster is related to a KLM cluster it might belong to that cluster
    // and will be interpreted as a klong in the KLM classifier.
    // So this should avoid double counting.
    if (cluster.getRelatedFrom<KLMCluster>()) {
      break;
    }


    // get various ECLCluster vars from getters
    m_ECLE              = cluster.getEnergy();
    m_ECLE9oE25         = cluster.getE9oE25();
    m_ECLTiming         = cluster.getTiming();
    m_ECLEerror         = cluster.getErrorEnergy();
    m_ECLminTrkDistance = cluster.getTemporaryMinTrkDistance();
    m_ECLdeltaL         = cluster.getTemporaryDeltaL();
    //m_ECLBKGProb      = cluster.getBKGProb();

    const TVector3& cluster_pos = cluster.getclusterPosition();

    // get distance to next genfit track
    // might be obsolete with the new ECLminTrkDist variable
    // TODO get rid of it ??
    m_ECLtrackDist = 999999;
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
    m_ECLBKGProb = m_readerBKG -> EvaluateMVA(m_BKGClassifierName);

    // normalize if not errorcode
    if (m_ECLBKGProb > -1.) {
      m_ECLBKGProb = (m_ECLBKGProb + 1) / 2.0;
    }


    // >> calculate Kl Id <<
    MVAOut = m_reader -> EvaluateMVA(m_classifierName);
    // write id into luster

    // normalize if not errorcode
    if (MVAOut > -1.) {
      MVAOut = (MVAOut + 1) / 2.0;
    }

    // use this when member is put to mdst
    // if this will ever happen ...
    //cluster.setKlId(MVAOut);



    // KlId, bkg prob, KLM, ECL
    klid = klids.appendNew(MVAOut, m_ECLBKGProb, 0, 1);
    cluster.addRelationTo(klid);


  }// for cluster in clusters
} // event


void KlIdECLTMVAExpertModule::terminate()
{
  delete m_reader;
  delete m_readerBKG;
}











