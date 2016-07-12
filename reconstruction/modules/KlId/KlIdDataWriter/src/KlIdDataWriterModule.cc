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
#include <tracking/dataobjects/TrackClusterSeparation.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <TTree.h>
#include <TFile.h>
#include <genfit/Exception.h>
#include <cstring>

#include "reconstruction/modules/KlId/KlIdKLMTMVAExpert/helperFunctions.h"

using namespace KlIdHelpers;
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
  StoreArray<RecoTrack>::required();
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
  m_treeKLM   -> Branch("KLMInitialtrackSepAngle",  & m_KLMInitialTrackSepAngle);
  m_treeKLM   -> Branch("KLMTrackRotationAngle",    & m_KLMTrackRotationAngle);
  m_treeKLM   -> Branch("KLMTrackClusterSepAngle",  & m_KLMTrackClusterSepAngle);
  m_treeKLM   -> Branch("isBeamBKG",  & m_isBeamBKG);



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
  m_treeECL   -> Branch("isBeamBKG",  & m_isBeamBKG);


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
  m_readerBKG -> AddVariable("KLMTrackSepDist",          &m_KLMTrackSepDist);
  m_readerBKG -> AddVariable("KLMTrackSepAngle",         &m_KLMTrackSepAngle);

  m_readerBKG -> AddVariable("KLMInitialtrackSepAngle",  &m_KLMInitialTrackSepAngle);
  m_readerBKG -> AddVariable("KLMTrackRotationAngle",    &m_KLMTrackRotationAngle);
  m_readerBKG -> AddVariable("KLMTrackClusterSepAngle",  &m_KLMTrackClusterSepAngle);


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
  StoreArray<RecoTrack> genfitTracks;
  StoreArray<ECLCluster> eclClusters;
  klmClusters.requireRelationTo(mcParticles);

// ------------------ KLM CLUSTERS

  for (const KLMCluster& cluster : klmClusters) {

    // needed later
    const TVector3& clusterPos = cluster.getClusterPosition();

    // get various KLMCluster vars
    m_KLMglobalZ = clusterPos.Z();
    m_KLMnCluster = klmClusters.getEntries();
    m_KLMnLayer = cluster.getLayers();
    m_KLMnInnermostLayer = cluster.getInnermostLayer();
    m_KLMtime = cluster.getTime();
    m_KLMinvM = cluster.getMomentum().M2();
    m_KLMenergy = cluster.getMomentum().E();
    m_KLMhitDepth = cluster.getClusterPosition().Mag2();
    // find nearest ecl cluster and calculate angular distance
    ECLCluster* closestECLCluster = nullptr;


    // find nearest ecl cluster and calculate angular distance
    pair<ECLCluster*, double> closestECLAndDist = findClosestECLCluster(clusterPos);
    closestECLCluster = get<0>(closestECLAndDist);
    m_KLMECLDist = get<1>(closestECLAndDist);


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
    tuple<const KLMCluster*, double, double> closestKLMAndDist = findClosestKLMCluster(clusterPos);
    m_KLMnextCluster = get<1>(closestKLMAndDist);
    m_KLMavInterClusterDist = get<2>(closestKLMAndDist);


    // find mc truth
    // go thru all particles mothers up to Y4s and check if its a Klong
    MCParticle* part = cluster.getRelatedTo<MCParticle>();
    m_KLMTruth = mcParticleIsKlong(part);
    m_isBeamBKG = mcParticleIsBeamBKG(part);


    // calculate eucl. distance klmcluster <-> nearest track
    // extrapolate genfit trackfit result to their ends and find the
    tuple<RecoTrack*, double, const TVector3*> closestTrackAndDistance
      = findClosestTrack(clusterPos);
    m_KLMtrackDist = get<1>(closestTrackAndDistance);
    const TVector3* poca = get<2>(closestTrackAndDistance);

    if (poca and closestECLCluster) {
      const TVector3& trackECLClusterDist = closestECLCluster->getPosition() - *poca;
      m_KLMECLtrackDist = trackECLClusterDist.Mag2();
    } else {
      m_KLMECLtrackDist = -999;
    }


    TrackClusterSeparation* trackSep = cluster.getRelatedTo<TrackClusterSeparation>();
    m_KLMTrackSepDist = trackSep->getDistance();
    m_KLMTrackSepAngle = trackSep->getTrackClusterAngle();

    m_KLMInitialTrackSepAngle = trackSep->getTrackClusterInitialSeparationAngle();
    m_KLMTrackRotationAngle = trackSep->getTrackRotationAngle();
    m_KLMTrackClusterSepAngle = trackSep->getTrackClusterSeparationAngle();



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
    const TVector3& clusterPos = cluster.getclusterPosition();


    // find mc truth
    // go thru all particles mothers up to Y4s and check if its a Klong
    MCParticle* part = cluster.getRelatedTo<MCParticle>();
    m_isBeamBKG = mcParticleIsBeamBKG(part);
    m_ECLTruth = mcParticleIsKlong(part);

    //find closest track
    tuple<RecoTrack*, double, const TVector3*> closestTrackAndDistance
      = findClosestTrack(clusterPos);

    m_ECLtrackDist = get<1>(closestTrackAndDistance);


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











