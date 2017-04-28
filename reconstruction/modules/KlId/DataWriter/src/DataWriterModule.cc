/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * **************************************************************************/



#include <reconstruction/modules/KlId/DataWriter/DataWriterModule.h>

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

#include "reconstruction/modules/KlId/KLMExpert/helperFunctions.h"

using namespace Belle2;
using namespace std;
using namespace KlIdHelpers;


// --------------------------------------Module----------------------------------------------


REG_MODULE(DataWriter);

DataWriterModule::DataWriterModule(): Module() // constructor kan nkeine argumente nehmen
{
  setDescription("Used to write flat ntuple for KlId TMVA trainings for both ECL and KLM KlID. Output is root file. Training macro is located in reconstruction/modules/KlId/<classifier> .");

  addParam("outPath", m_outPath, "path where you want your root files to be placed.", m_outPath);
}



DataWriterModule::~DataWriterModule()
{
}


void DataWriterModule::initialize()
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

  m_treeECL   -> Branch("ECLdeltaL",            & m_ECLdeltaL);
  m_treeECL   -> Branch("ECLmintrackDist",      & m_ECLminTrkDistance);
  m_treeECL   -> Branch("isBeamBKG",  & m_isBeamBKG);

}//init


void DataWriterModule::beginRun()
{
}

void DataWriterModule::endRun()
{
}

void DataWriterModule::event()
{
  StoreArray<MCParticle> mcParticles;
  StoreArray<KLMCluster> klmClusters;
  StoreArray<RecoTrack> genfitTracks;
  StoreArray<ECLCluster> eclClusters;

// ------------------ KLM CLUSTERS

  for (const KLMCluster& cluster : klmClusters) {

    const TVector3& clusterPos = cluster.getClusterPosition();

    m_KLMglobalZ = clusterPos.Z();
    m_KLMnCluster = klmClusters.getEntries();
    m_KLMnLayer = cluster.getLayers();
    m_KLMnInnermostLayer = cluster.getInnermostLayer();
    m_KLMtime = cluster.getTime();
    m_KLMinvM = cluster.getMomentum().M2();
    m_KLMenergy = cluster.getEnergy();
    m_KLMhitDepth = cluster.getClusterPosition().Mag2();

    TrackClusterSeparation* trackSep = cluster.getRelatedTo<TrackClusterSeparation>();
    m_KLMTrackSepDist = trackSep->getDistance();
    m_KLMTrackSepAngle = trackSep->getTrackClusterAngle();

    m_KLMInitialTrackSepAngle = trackSep->getTrackClusterInitialSeparationAngle();
    m_KLMTrackRotationAngle = trackSep->getTrackRotationAngle();
    m_KLMTrackClusterSepAngle = trackSep->getTrackClusterSeparationAngle();

    if (isnan(m_KLMglobalZ))              { m_KLMglobalZ              = -999;}
    if (isnan(m_KLMnCluster))             { m_KLMnCluster             = -999;}
    if (isnan(m_KLMnLayer))               { m_KLMnLayer               = -999;}
    if (isnan(m_KLMnInnermostLayer))      { m_KLMnInnermostLayer      = -999;}
    if (isnan(m_KLMtime))                 { m_KLMtime                 = -999;}
    if (isnan(m_KLMinvM))                 { m_KLMinvM                 = -999;}
    if (isnan(m_KLMenergy))               { m_KLMenergy               = -999;}
    if (isnan(m_KLMhitDepth))             { m_KLMhitDepth             = -999;}
    if (isnan(m_KLMTrackSepDist))         { m_KLMTrackSepDist         = -999;}
    if (isnan(m_KLMTrackSepAngle))        { m_KLMTrackSepAngle        = -999;}
    if (isnan(m_KLMInitialTrackSepAngle)) { m_KLMInitialTrackSepAngle = -999;}
    if (isnan(m_KLMTrackRotationAngle))   { m_KLMTrackRotationAngle   = -999;}
    if (isnan(m_KLMTrackClusterSepAngle)) { m_KLMTrackClusterSepAngle = -999;}


    pair<ECLCluster*, double> closestECLAndDist = findClosestECLCluster(clusterPos);
    ECLCluster* closestECLCluster = get<0>(closestECLAndDist);
    m_KLMECLDist = get<1>(closestECLAndDist);

    if (!(closestECLCluster == nullptr)) {
      m_KLMECLE      = closestECLCluster->getEnergy();
      m_KLMECLE9oE25 = closestECLCluster->getE9oE21();
      m_KLMECLEerror = closestECLCluster->getUncertaintyEnergy();
      m_KLMECLTerror = closestECLCluster->getDeltaTime99();

      m_KLMECLdeltaL = closestECLCluster->getDeltaL();;
      m_KLMECLminTrackDist = closestECLCluster->getMinTrkDistance();
      m_KLMECLTiming = closestECLCluster->getTime();
    } else {
      m_KLMECLdeltaL = -999;
      m_KLMECLminTrackDist = -999;
      m_KLMECLE      = -999;
      m_KLMECLE9oE25 = -999;
      m_KLMECLTiming = -999;
      m_KLMECLTerror = -999;
      m_KLMECLEerror = -999;
    }

    tuple<const KLMCluster*, double, double> closestKLMAndDist = findClosestKLMCluster(clusterPos);
    m_KLMnextCluster = get<1>(closestKLMAndDist);
    m_KLMavInterClusterDist = get<2>(closestKLMAndDist);

    MCParticle* part = cluster.getRelatedTo<MCParticle>();
    m_KLMTruth = mcParticleIsKlong(part);
    m_isBeamBKG = mcParticleIsBeamBKG(part);

    // use genfit to find nearest track by extrapolation
    tuple<RecoTrack*, double, std::unique_ptr<const TVector3>> closestTrackAndDistance
                                                            = findClosestTrack(clusterPos, 0.26);
    m_KLMtrackDist = get<1>(closestTrackAndDistance);
    const TVector3* poca = get<2>(closestTrackAndDistance).get();

    if (poca and closestECLCluster) {
      const TVector3& trackECLClusterDist = closestECLCluster->getPosition() - *poca;
      m_KLMtrackToECL = trackECLClusterDist.Mag2();
    } else {
      m_KLMtrackToECL = -999;
    }

    m_treeKLM -> Fill();
  }// for klmcluster in klmclusters

// ---------------   ECL CLUSTERS
  for (const ECLCluster& cluster : eclClusters) {


    m_ECLminTrkDistance = cluster.getMinTrkDistance();
    m_ECLdeltaL = cluster.getDeltaL();

    m_ECLE = cluster.getEnergy();
    m_ECLE9oE25 = cluster.getE9oE21();
    m_ECLTiming = cluster.getTime();
    m_ECLR = cluster.getR();
    m_ECLEerror = cluster.getUncertaintyEnergy();


    if (isnan(m_ECLminTrkDistance)) {m_ECLminTrkDistance = -999;}
    if (isnan(m_ECLdeltaL)) {m_ECLdeltaL                 = -999;}
    if (isnan(m_ECLE)) {m_ECLE                           = -999;}
    if (isnan(m_ECLE9oE25)) {m_ECLE9oE25                 = -999;}
    if (isnan(m_ECLTiming)) {m_ECLTiming                 = -999;}
    if (isnan(m_ECLR)) {m_ECLR                           = -999;}
    if (isnan(m_ECLEerror)) {m_ECLEerror                 = -999;}


    const TVector3& clusterPos = cluster.getClusterPosition();

    MCParticle* part = cluster.getRelatedTo<MCParticle>();
    m_isBeamBKG = mcParticleIsBeamBKG(part);
    m_ECLTruth = mcParticleIsKlong(part);

    tuple<RecoTrack*, double, std::unique_ptr<const TVector3>> closestTrackAndDistance = findClosestTrack(clusterPos, .26);
    m_ECLtrackDist = get<1>(closestTrackAndDistance);

    m_treeECL -> Fill();
  }// for ecl cluster in clusters
} // event


void DataWriterModule::terminate()
{
  // close root files
  m_f    -> cd();
  m_treeKLM -> Write();
  m_treeECL -> Write();
  m_f    -> Close();
}

