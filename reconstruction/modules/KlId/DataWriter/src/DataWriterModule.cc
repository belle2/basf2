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
#include <mdst/dataobjects/KlId.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <tracking/dataobjects/TrackClusterSeparation.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <analysis/ClusterUtility/ClusterUtils.h>

#include <TTree.h>
#include <TFile.h>
#include <cstring>
#include <utility>

#include "reconstruction/modules/KlId/KLMExpert/KlId.h"

using namespace Belle2;
using namespace std;
using namespace KlId;


// --------------------------------------Module----------------------------------------------


REG_MODULE(DataWriter);

DataWriterModule::DataWriterModule(): Module()
{
  setDescription("Used to write flat ntuple for KlId classifier trainings for both ECL and KLM KlID. Output is a root file.");

  addParam("outPath", m_outPath, "Output path - where you want your root files to be placed.", m_outPath);
  addParam("useKLM", m_useKLM, "Write KLM data.", m_useKLM);
  addParam("useECL", m_useECL, "Write ECL data.", m_useECL);
}



DataWriterModule::~DataWriterModule()
{
}


void DataWriterModule::initialize()
{
  // require existence of necessary datastore obj

  m_eclClusters.isRequired();
  m_klmClusters.isRequired();
  m_mcParticles.isRequired();

  m_klmClusters.requireRelationTo(m_mcParticles);
  m_klmClusters.registerRelationTo(m_eclClusters);

  m_f = new TFile(m_outPath.c_str(), "recreate");

  m_treeECLhadron = new TTree("ECLdataHadron", "ECLdataHadron");
  m_treeECLgamma = new TTree("ECLdataGamma", "ECLdataGamma");

  // KLM
  if (m_useKLM) {
    m_treeKLM = new TTree("KLMdata", "KLMdata");
    m_treeKLM -> Branch("KLMMCMom",                 & m_KLMMCMom);
    m_treeKLM -> Branch("KLMMCPhi",                 & m_KLMMCPhi);
    m_treeKLM -> Branch("KLMMCTheta",               & m_KLMMCTheta);
    m_treeKLM -> Branch("KLMMom",                   & m_KLMMom);
    m_treeKLM -> Branch("KLMPhi",                   & m_KLMPhi);
    m_treeKLM -> Branch("KLMTheta",                 & m_KLMTheta);
    m_treeKLM -> Branch("KLMMCLifetime",            & m_KLMMCLifetime);
    m_treeKLM -> Branch("KLMMCPDG",                 & m_KLMMCPDG);
    m_treeKLM -> Branch("KLMMCPrimaryPDG",          & m_KLMMCPrimaryPDG);
    m_treeKLM -> Branch("KLMMCStatus",              & m_KLMMCStatus);
    m_treeKLM -> Branch("KLMnCluster",                & m_KLMnCluster);
    m_treeKLM -> Branch("KLMnLayer",                  & m_KLMnLayer);
    m_treeKLM -> Branch("KLMnInnermostlayer",         & m_KLMnInnermostLayer);
    m_treeKLM -> Branch("KLMglobalZ",                 & m_KLMglobalZ);
    m_treeKLM -> Branch("KLMtime",                    & m_KLMtime);
    m_treeKLM -> Branch("KLMinvM",                    & m_KLMinvM);
    m_treeKLM -> Branch("KLMTruth",                   & m_KLMTruth);
    m_treeKLM -> Branch("KLMdistToNextCl",            & m_KLMnextCluster);
    m_treeKLM -> Branch("KLMenergy",                  & m_KLMenergy);
    m_treeKLM -> Branch("KLMaverageInterClusterDist", & m_KLMavInterClusterDist);
    m_treeKLM -> Branch("KLMhitDepth",                & m_KLMhitDepth);

    m_treeKLM -> Branch("KLMECLHypo",              & m_KLMECLHypo);
    m_treeKLM -> Branch("KLMECLZMVA",              & m_KLMECLZMVA);
    m_treeKLM -> Branch("KLMECLZ40",               & m_KLMECLZ40);
    m_treeKLM -> Branch("KLMECLZ51",               & m_KLMECLZ51);
    m_treeKLM -> Branch("KLMECLUncertaintyPhi",    & m_KLMECLUncertaintyPhi);
    m_treeKLM -> Branch("KLMECLUncertaintyTheta",  & m_KLMECLUncertaintyTheta);
    m_treeKLM -> Branch("KLMdistToNextECL",        & m_KLMECLDist);
    m_treeKLM -> Branch("KLMtrackToECL",           & m_KLMtrackToECL);
    m_treeKLM -> Branch("KLMECLEerror",            & m_KLMECLEerror);
    m_treeKLM -> Branch("KLMECLenergy",            & m_KLMECLE);
    m_treeKLM -> Branch("KLMECLE9oE25",            & m_KLMECLE9oE25);
    m_treeKLM -> Branch("KLMECLtiming",            & m_KLMECLTiming);
    m_treeKLM -> Branch("KLMECLTerror",            & m_KLMECLTerror);
    m_treeKLM -> Branch("KLMECLdeltaL",            & m_KLMECLdeltaL);
    m_treeKLM -> Branch("KLMECLmintrackDist",      & m_KLMECLminTrackDist);
    m_treeKLM -> Branch("KLMTrackSepDist",         & m_KLMTrackSepDist);
    m_treeKLM -> Branch("KLMTrackSepAngle",        & m_KLMTrackSepAngle);
    m_treeKLM -> Branch("KLMInitialtrackSepAngle", & m_KLMInitialTrackSepAngle);
    m_treeKLM -> Branch("KLMTrackRotationAngle",   & m_KLMTrackRotationAngle);
    m_treeKLM -> Branch("KLMTrackClusterSepAngle", & m_KLMTrackClusterSepAngle);
    m_treeKLM -> Branch("isBeamBKG",               & m_isBeamBKG);
    m_treeKLM -> Branch("KLMKlId",                 & m_KLMKLid);
    m_treeKLM -> Branch("KLMAngleToMC",            & m_KLMAngleToMC);
    m_treeKLM -> Branch("KLMMCWeight",             & m_KLMMCWeight);
    m_treeKLM -> Branch("KLMtrackFlag",            & m_KLMtrackFlag);
    m_treeKLM -> Branch("KLMeclFlag",              & m_KLMeclFlag);
    m_treeKLM -> Branch("isSignal",                & m_isSignal);
  }//useKLM

  //ECL
  if (m_useECL) {
    m_treeECLhadron = new TTree("ECLdataHadron", "ECLdataHadron");
    m_treeECLgamma = new TTree("ECLdataGamma", "ECLdataGamma");

    m_treeECLhadron -> Branch("ECLMCMom",             & m_ECLMCMom);
    m_treeECLhadron -> Branch("ECLMCPhi",             & m_ECLMCPhi);
    m_treeECLhadron -> Branch("ECLMCLifetime",        & m_ECLMCLifetime);
    m_treeECLhadron -> Branch("ECLMCPDG",             & m_ECLMCPDG);
    m_treeECLhadron -> Branch("ECLMCTheta",           & m_ECLMCTheta);
    m_treeECLhadron -> Branch("ECLMCLifetime",        & m_ECLMCLifetime);
    m_treeECLhadron -> Branch("ECLMCPDG",             & m_ECLMCPDG);
    m_treeECLhadron -> Branch("ECLMCStatus",          & m_ECLMCStatus);
    m_treeECLhadron -> Branch("ECLMCPrimaryPDG",      & m_ECLMCPrimaryPDG);
    m_treeECLhadron -> Branch("ECLUncertaintyEnergy", & m_ECLUncertaintyEnergy);
    m_treeECLhadron -> Branch("ECLUncertaintyTheta",  & m_ECLUncertaintyTheta);
    m_treeECLhadron -> Branch("ECLUncertaintyPhi",    & m_ECLUncertaintyPhi);
    m_treeECLhadron -> Branch("ECLMom",               & m_ECLMom);
    m_treeECLhadron -> Branch("ECLPhi",               & m_ECLPhi);
    m_treeECLhadron -> Branch("ECLTheta",             & m_ECLTheta);
    m_treeECLhadron -> Branch("ECLZ",                 & m_ECLZ);
    m_treeECLhadron -> Branch("ECLenergy",            & m_ECLE);
    m_treeECLhadron -> Branch("ECLE9oE25",            & m_ECLE9oE25);
    m_treeECLhadron -> Branch("ECLtiming",            & m_ECLTiming);
    m_treeECLhadron -> Branch("ECLR",                 & m_ECLR);
    m_treeECLhadron -> Branch("ECLTruth",             & m_ECLTruth);
    m_treeECLhadron -> Branch("ECLZ51",               & m_ECLZ51);
    m_treeECLhadron -> Branch("ECLZ40",               & m_ECLZ40);
    m_treeECLhadron -> Branch("ECLE1oE9",             & m_ECLE1oE9);
    m_treeECLhadron -> Branch("ECL2ndMom",            & m_ECL2ndMom);
    m_treeECLhadron -> Branch("ECLnumChrystals",      & m_ECLnumChrystals);
    m_treeECLhadron -> Branch("ECLLAT",               & m_ECLLAT);
    m_treeECLhadron -> Branch("ECLZMVA",              & m_ECLZMVA);
    m_treeECLhadron -> Branch("ECLKlId",              & m_ECLKLid);
    m_treeECLhadron -> Branch("ECLdeltaL",            & m_ECLdeltaL);
    m_treeECLhadron -> Branch("ECLmintrackDist",      & m_ECLminTrkDistance);
    m_treeECLhadron -> Branch("isBeamBKG",            & m_isBeamBKG);
    m_treeECLhadron -> Branch("ECLMCWeight",          & m_ECLMCWeight);
    m_treeECLhadron -> Branch("isSignal",             & m_isSignal);


    m_treeECLgamma -> Branch("ECLMCMom",             & m_ECLMCMom);
    m_treeECLgamma -> Branch("ECLMCPhi",             & m_ECLMCPhi);
    m_treeECLgamma -> Branch("ECLMCTheta",           & m_ECLMCTheta);
    m_treeECLgamma -> Branch("ECLMCLifetime",        & m_ECLMCLifetime);
    m_treeECLgamma -> Branch("ECLMCPDG",             & m_ECLMCPDG);
    m_treeECLgamma -> Branch("ECLMCStatus",          & m_ECLMCStatus);
    m_treeECLgamma -> Branch("ECLMCPrimaryPDG",      & m_ECLMCPrimaryPDG);
    m_treeECLgamma -> Branch("ECLUncertaintyEnergy", & m_ECLUncertaintyEnergy);
    m_treeECLgamma -> Branch("ECLUncertaintyTheta",  & m_ECLUncertaintyTheta);
    m_treeECLgamma -> Branch("ECLUncertaintyPhi",    & m_ECLUncertaintyPhi);
    m_treeECLgamma -> Branch("ECLMom",               & m_ECLMom);
    m_treeECLgamma -> Branch("ECLPhi",               & m_ECLPhi);
    m_treeECLgamma -> Branch("ECLTheta",             & m_ECLTheta);
    m_treeECLgamma -> Branch("ECLZ",                 & m_ECLZ);
    m_treeECLgamma -> Branch("ECLenergy",            & m_ECLE);
    m_treeECLgamma -> Branch("ECLE9oE25",            & m_ECLE9oE25);
    m_treeECLgamma -> Branch("ECLtiming",            & m_ECLTiming);
    m_treeECLgamma -> Branch("ECLR",                 & m_ECLR);
    m_treeECLgamma -> Branch("ECLTruth",             & m_ECLTruth);
    m_treeECLgamma -> Branch("ECLZ51",               & m_ECLZ51);
    m_treeECLgamma -> Branch("ECLZ40",               & m_ECLZ40);
    m_treeECLgamma -> Branch("ECLE1oE9",             & m_ECLE1oE9);
    m_treeECLgamma -> Branch("ECL2ndMom",            & m_ECL2ndMom);
    m_treeECLgamma -> Branch("ECLnumChrystals",      & m_ECLnumChrystals);
    m_treeECLgamma -> Branch("ECLLAT",               & m_ECLLAT);
    m_treeECLgamma -> Branch("ECLZMVA",              & m_ECLZMVA);
    m_treeECLgamma -> Branch("ECLKlId",              & m_ECLKLid);
    m_treeECLgamma -> Branch("ECLdeltaL",            & m_ECLdeltaL);
    m_treeECLgamma -> Branch("ECLmintrackDist",      & m_ECLminTrkDistance);
    m_treeECLgamma -> Branch("isBeamBKG",            & m_isBeamBKG);
    m_treeECLgamma -> Branch("ECLMCWeight",          & m_ECLMCWeight);
    m_treeECLgamma -> Branch("isSignal",                 & m_isSignal);
  }//useECL
}//init


void DataWriterModule::beginRun()
{
}

void DataWriterModule::endRun()
{
}

void DataWriterModule::event()
{

  for (const KLMCluster& cluster : m_klmClusters) {

    if (!m_useKLM) {continue;}

    const TVector3& clusterPos = cluster.getClusterPosition();

    m_KLMPhi                         = clusterPos.Phi();
    m_KLMTheta                       = clusterPos.Theta();

    m_KLMglobalZ                     = clusterPos.Z();
    m_KLMnCluster                    = m_klmClusters.getEntries();
    m_KLMnLayer                      = cluster.getLayers();
    m_KLMnInnermostLayer             = cluster.getInnermostLayer();
    m_KLMtime                        = cluster.getTime();
    m_KLMinvM                        = cluster.getMomentum().M2();
    m_KLMenergy                      = cluster.getEnergy();
    m_KLMhitDepth                    = cluster.getClusterPosition().Mag();
    m_KLMtrackFlag                   = cluster.getAssociatedTrackFlag();
    m_KLMeclFlag                     = cluster.getAssociatedEclClusterFlag();

    m_KLMTrackSepDist         = -999;
    m_KLMTrackSepAngle        = -999;
    m_KLMInitialTrackSepAngle = -999;
    m_KLMTrackRotationAngle   = -999;
    m_KLMTrackClusterSepAngle = -999;
    auto trackSeperations = cluster.getRelationsTo<TrackClusterSeparation>();
    TrackClusterSeparation* trackSep;
    float best_dist = 100000000;
    float dist;
    for (auto trackSeperation :  trackSeperations) {
      dist = trackSeperation.getDistance();
      if (dist < best_dist) {
        best_dist = dist;
        trackSep = &trackSeperation;
        m_KLMTrackSepDist                = trackSep->getDistance();
        m_KLMTrackSepAngle               = trackSep->getTrackClusterAngle();
        m_KLMInitialTrackSepAngle        = trackSep->getTrackClusterInitialSeparationAngle();
        m_KLMTrackRotationAngle          = trackSep->getTrackRotationAngle();
        m_KLMTrackClusterSepAngle        = trackSep->getTrackClusterSeparationAngle();
      }
    }


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
      m_KLMECLE              = closestECLCluster->getEnergy();
      m_KLMECLE9oE25         = closestECLCluster->getE9oE21();
      m_KLMECLEerror         = closestECLCluster->getUncertaintyEnergy();
      m_KLMECLTerror         = closestECLCluster->getDeltaTime99();
      m_KLMECLdeltaL         = closestECLCluster->getDeltaL();;
      m_KLMECLminTrackDist   = closestECLCluster->getMinTrkDistance();
      m_KLMECLTiming         = closestECLCluster->getTime();
      m_KLMECLZMVA           = closestECLCluster->getZernikeMVA();
      m_KLMECLZ40            = closestECLCluster->getAbsZernike40();
      m_KLMECLZ51            = closestECLCluster->getAbsZernike51();
      m_KLMECLUncertaintyPhi = closestECLCluster->getUncertaintyPhi();
      m_KLMECLUncertaintyTheta = closestECLCluster->getUncertaintyTheta();
      m_KLMECLHypo = closestECLCluster->getHypothesisId();
    } else {
      m_KLMECLdeltaL         = -999;
      m_KLMECLminTrackDist   = -999;
      m_KLMECLE              = -999;
      m_KLMECLE9oE25         = -999;
      m_KLMECLTiming         = -999;
      m_KLMECLTerror         = -999;
      m_KLMECLEerror         = -999;

      m_KLMECLHypo           = -999;
      m_KLMECLZMVA           = -999;
      m_KLMECLZ40            = -999;
      m_KLMECLZ51            = -999;
      m_KLMECLUncertaintyPhi = -999;
      m_KLMECLUncertaintyTheta = -999;
    }

    tuple<const KLMCluster*, double, double> closestKLMAndDist = findClosestKLMCluster(clusterPos);
    m_KLMnextCluster        = get<1>(closestKLMAndDist);
    m_KLMavInterClusterDist = get<2>(closestKLMAndDist);

    const auto mcParticleWeightPair = cluster.getRelatedToWithWeight<MCParticle>();
    MCParticle* part        = mcParticleWeightPair.first;

    m_KLMTruth              = mcParticleIsKlong(part);
    m_isBeamBKG             = mcParticleIsBeamBKG(part);

    if (part) {
      m_KLMMCWeight = mcParticleWeightPair.second;
      m_KLMAngleToMC    = clusterPos.Angle(part->getMomentum());
      m_KLMMCStatus     = part->getStatus();
      m_KLMMCLifetime   = part->getLifetime();
      m_KLMMCPDG        = std::abs(part->getPDG());
      m_KLMMCPrimaryPDG = getPrimaryPDG(part);
      m_KLMMCMom        = part->getMomentum().Mag();
      m_KLMMCPhi        = part->getMomentum().Phi();
      m_KLMMCTheta      = part->getMomentum().Theta();
    } else {
      m_KLMMCWeight = -999;
      m_KLMAngleToMC    = -999;
      m_KLMMCStatus     = -999;
      m_KLMMCLifetime   = -999;
      m_KLMMCPDG        = -999;
      m_KLMMCPrimaryPDG = -999;
      m_KLMMCMom        = -999;
      m_KLMMCPhi        = -999;
      m_KLMMCTheta      = -999;
    }

    KlId* klid = cluster.getRelatedTo<KlId>();
    if (klid) {
      m_KLMKLid = klid->getKlId();
    } else {
      m_KLMKLid = -999;
    }
    m_isSignal = isKLMClusterSignal(cluster, 0);


    m_treeKLM -> Fill();
  }// for klmcluster in klmclusters

// ---------------   ECL CLUSTERS
  for (const ECLCluster& cluster : m_eclClusters) {

    if (!m_useECL) {continue;}

    m_ECLminTrkDistance = cluster.getMinTrkDistance();
    m_ECLdeltaL         = cluster.getDeltaL();
    m_ECLE              = cluster.getEnergy();
    m_ECLE9oE25         = cluster.getE9oE21();
    m_ECLTiming         = cluster.getTime();
    m_ECLR              = cluster.getR();
    m_ECLEerror         = cluster.getUncertaintyEnergy();
    m_ECLZ51            = cluster.getAbsZernike51();
    m_ECLZ40            = cluster.getAbsZernike40();
    m_ECLE1oE9          = cluster.getE1oE9();
    m_ECL2ndMom         = cluster.getSecondMoment();
    m_ECLnumChrystals   = cluster.getNumberOfCrystals();
    m_ECLLAT            = cluster.getLAT();
    m_ECLZMVA           = cluster.getZernikeMVA();

    if (isnan(m_ECLminTrkDistance)) { m_ECLminTrkDistance = -999;}
    if (isnan(m_ECLdeltaL))         { m_ECLdeltaL         = -999;}
    if (isnan(m_ECLE))              { m_ECLE              = -999;}
    if (isnan(m_ECLE9oE25))         { m_ECLE9oE25         = -999;}
    if (isnan(m_ECLTiming))         { m_ECLTiming         = -999;}
    if (isnan(m_ECLR))              { m_ECLR              = -999;}
    if (isnan(m_ECLEerror))         { m_ECLEerror         = -999;}
    if (isnan(m_ECLZ40))            { m_ECLZ40            = -999;}
    if (isnan(m_ECLZ51))            { m_ECLZ51            = -999;}
    if (isnan(m_ECLE1oE9))          { m_ECLE1oE9          = -999;}
    if (isnan(m_ECL2ndMom))         { m_ECL2ndMom         = -999;}
    if (isnan(m_ECLnumChrystals))   { m_ECLnumChrystals   = -999;}
    if (isnan(m_ECLLAT))            { m_ECLLAT            = -999;}
    if (isnan(m_ECLZMVA))           { m_ECLZMVA           = -999;}

    KlId* klid = cluster.getRelatedTo<KlId>();
    if (klid) {
      m_ECLKLid = klid->getKlId();
    } else {
      m_ECLKLid = -999;
    }

    const TVector3& clusterPos = cluster.getClusterPosition();

    m_ECLPhi               = clusterPos.Phi();
    m_ECLTheta             = clusterPos.Theta();
    m_ECLZ                 = clusterPos.Z();

    ClusterUtils C;
    m_ECLMom               = C.Get4MomentumFromCluster(&cluster).Vect().Mag2();
    m_ECLDeltaTime         = cluster.getDeltaTime99();

    m_ECLUncertaintyEnergy = cluster.getUncertaintyEnergy();
    m_ECLUncertaintyTheta  = cluster.getUncertaintyTheta();
    m_ECLUncertaintyPhi    = cluster.getUncertaintyPhi();

//    MCParticle* part       = cluster.getRelatedTo<MCParticle>();
    const auto mcParticleWeightPair = cluster.getRelatedToWithWeight<MCParticle>();
    MCParticle* part        = mcParticleWeightPair.first;

    m_isBeamBKG            = mcParticleIsBeamBKG(part);
    m_ECLTruth             = mcParticleIsKlong(part);
    if (part) {
      m_ECLMCWeight = mcParticleWeightPair.second;
      m_ECLMCStatus        = part->getStatus();
      m_ECLMCLifetime      = part->getLifetime();
      m_ECLMCPDG           = std::abs(part->getPDG());

      m_ECLMCPrimaryPDG    = getPrimaryPDG(part);

      m_ECLMCMom           = part->getMomentum().Mag2();
      m_ECLMCPhi           = part->getMomentum().Phi();
      m_ECLMCTheta         = part->getMomentum().Theta();
    } else {
      m_ECLMCWeight = -999;
      m_ECLMCStatus        = -999;
      m_ECLMCLifetime      = -999;
      m_ECLMCPrimaryPDG    = -999;
      m_ECLMCPDG           = -999;
      m_ECLMCMom           = -999;
      m_ECLMCPhi           = -999;
      m_ECLMCTheta         = -999;
    }

    m_isSignal = isECLClusterSignal(cluster);

    if (cluster.getHypothesisId() == 6) { m_treeECLhadron -> Fill();}
    if (cluster.getHypothesisId() == 5) { m_treeECLgamma  -> Fill();}
  }// for ecl cluster in clusters
} // event


void DataWriterModule::terminate()
{
  // close root files
  m_f                 -> cd();
  if (m_useKLM) { m_treeKLM -> Write();}
  if (m_useECL) {
    m_treeECLhadron     -> Write();
    m_treeECLgamma      -> Write();
  }
  m_f                 -> Close();
}

