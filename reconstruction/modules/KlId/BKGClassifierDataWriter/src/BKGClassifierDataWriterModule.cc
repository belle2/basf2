/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * **************************************************************************/



#include <reconstruction/modules/KlId/BKGClassifierDataWriter/BKGClassifierDataWriterModule.h>

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
#include <utility>

//#include "reconstruction/modules/KlId/KlIdKLMTMVAExpert/helperFunctions.h"
#include "reconstruction/modules/KlId/KLMExpert/helperFunctions.h"

using namespace Belle2;
using namespace std;
using namespace KlIdHelpers;


// --------------------------------------Module----------------------------------------------


REG_MODULE(BKGClassifierDataWriter);

BKGClassifierDataWriterModule::BKGClassifierDataWriterModule(): Module() // constructor kan nkeine argumente nehmen
{
  setDescription("Used to write flat ntuple for KlId TMVA trainings for both ECL and KLM KlID. Output is root file. Training macro is located in reconstruction/modules/KlId/<classifier> .");

  addParam("outPath", m_outPath, "path where you want your root files to be placed.", m_outPath);
}



BKGClassifierDataWriterModule::~BKGClassifierDataWriterModule()
{
}


void BKGClassifierDataWriterModule::initialize()
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
  m_treeKLM   -> Branch("KLMtrackToECL",            & m_KLMtrackToECL);
  m_treeKLM   -> Branch("KLMECLEerror",             & m_KLMECLEerror);
  m_treeKLM   -> Branch("KLMECLenergy",             & m_KLMECLE);
  m_treeKLM   -> Branch("KLMECLE9oE25",             & m_KLMECLE9oE25);
  m_treeKLM   -> Branch("KLMECLtiming",             & m_KLMECLTiming);
  m_treeKLM   -> Branch("KLMECLTerror",             & m_KLMECLTerror);
  m_treeKLM   -> Branch("KLMECLdeltaL",             & m_KLMECLdeltaL);
  m_treeKLM   -> Branch("KLMECLmintrackDist",       & m_KLMECLminTrackDist);

  //new TODO check feature importance
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
  m_treeECL   -> Branch("ECLangleToMissE05",          & m_ECLangleTomissE05);
  m_treeECL   -> Branch("ECLangleToMissE06",          & m_ECLangleTomissE06);
  m_treeECL   -> Branch("ECLangleToMissE07",          & m_ECLangleTomissE07);
  m_treeECL   -> Branch("ECLangleToMissE08",          & m_ECLangleTomissE08);
  m_treeECL   -> Branch("ECLPDG",          & m_ECLPDG);
  m_treeECL   -> Branch("ECLKLMPDG",          & m_ECLKLMPDG);
  m_treeECL   -> Branch("ECLangleToClosestKLM",          & m_ECLangleToClosestKLM);
  m_treeECL   -> Branch("ECKKLMTime",          & m_ECLKLMTime);
  m_treeECL   -> Branch("ECLbkgProb",          & m_ECLbkgProb);

  m_treeECL   -> Branch("isBeamBKG",  & m_isBeamBKG);

}//init


void BKGClassifierDataWriterModule::beginRun()
{
}

void BKGClassifierDataWriterModule::endRun()
{
}

void BKGClassifierDataWriterModule::event()
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

    TrackClusterSeparation* trackSep = cluster.getRelatedTo<TrackClusterSeparation>();
    m_KLMTrackSepDist = trackSep->getDistance();
    m_KLMTrackSepAngle = trackSep->getTrackClusterAngle();

    m_KLMInitialTrackSepAngle = trackSep->getTrackClusterInitialSeparationAngle();
    m_KLMTrackRotationAngle = trackSep->getTrackRotationAngle();
    m_KLMTrackClusterSepAngle = trackSep->getTrackClusterSeparationAngle();


    // find nearest ecl cluster and calculate angular distance
    pair<ECLCluster*, double> closestECLAndDist = findClosestECLCluster(clusterPos);
    ECLCluster* closestECLCluster = get<0>(closestECLAndDist);
    m_KLMECLDist = get<1>(closestECLAndDist);

    // get vars from nearest ECL CLuster
    //TODO introduce cutoff to what is "nearest"
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
      // needed to add those ecl clusters to KLM particles that have a high
      // kl probability and no relation
      cluster.addRelationTo(closestECLCluster);
    } else {
      m_KLMECLdeltaL = -999;
      m_KLMECLminTrackDist = -999;
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
      m_KLMtrackToECL = trackECLClusterDist.Mag2();
    } else {
      m_KLMtrackToECL = -999;
    }

    m_treeKLM -> Fill();
  }// for klmcluster in klmclusters

// ---------------   ECL CLUSTERS

  TVector3 missE03 = calculateMissEVector(0.3);
  TVector3 missE04 = calculateMissEVector(0.4);
  TVector3 missE05 = calculateMissEVector(0.5);
  TVector3 missE06 = calculateMissEVector(0.6);
  TVector3 missE07 = calculateMissEVector(0.7);
  TVector3 missE08 = calculateMissEVector(0.8);

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

    m_ECLangleTomissE03 = missE03.Angle(clusterPos);
    m_ECLangleTomissE04 = missE04.Angle(clusterPos);
    m_ECLangleTomissE05 = missE05.Angle(clusterPos);
    m_ECLangleTomissE06 = missE06.Angle(clusterPos);
    m_ECLangleTomissE07 = missE07.Angle(clusterPos);
    m_ECLangleTomissE08 = missE08.Angle(clusterPos);

    // find mc truth
    // go thru all particles mothers up to Y4s and check if its a Klong
    MCParticle* part = cluster.getRelatedTo<MCParticle>();
    m_isBeamBKG = mcParticleIsBeamBKG(part);
    m_ECLTruth = mcParticleIsKlong(part);
    if (cluster.getRelatedTo<KlId>()) {
      m_ECLbkgProb = cluster.getRelatedTo<KlId>()->getBkgProb();
    } else {
      cout << " no klid " << endl;
      m_ECLbkgProb = 0;
    }
    //find closest track
    tuple<RecoTrack*, double, const TVector3*> closestTrackAndDistance = findClosestTrack(clusterPos);
    m_ECLtrackDist = get<1>(closestTrackAndDistance);

    tuple<const KLMCluster*, double, double> nextKLM = findClosestKLMCluster(clusterPos);


    if (cluster.getRelatedTo<MCParticle>()) {
      m_ECLPDG = cluster.getRelatedTo<MCParticle>()->getPDG();
      if (get<0>(nextKLM)  and get<0>(nextKLM)->getRelatedTo<MCParticle>()) {
        m_ECLKLMPDG = get<0>(nextKLM)->getRelatedTo<MCParticle>()->getPDG();
      } else {
        m_ECLKLMPDG = 0;
      }
    } else {
      m_ECLPDG = 0;
      m_ECLKLMPDG = 0;
    }

    if (get<0>(nextKLM)) {
      m_ECLKLMTime = get<0>(nextKLM)->getTime();
      m_ECLangleToClosestKLM = get<0>(nextKLM)->getClusterPosition().Angle(clusterPos);
    } else {
      m_ECLangleToClosestKLM = 0;
      m_ECLKLMTime = 0;
    }
    // finally fill tree
    m_treeECL -> Fill();
  }// for ecl cluster in clusters
} // event


void BKGClassifierDataWriterModule::terminate()
{
  // close root files
  m_f    -> cd();
  m_treeKLM -> Write();
  m_treeECL -> Write();
  m_f    -> Close();
}

