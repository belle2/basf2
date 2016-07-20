/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <reconstruction/modules/KlId/IDValidation/IDValidationModule.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <reconstruction/dataobjects/KlId.h>

#include <TTree.h>
#include <TFile.h>
#include <cstring>

#include "reconstruction/modules/KlId/KlIdKLMTMVAExpert/helperFunctions.h"

using namespace KlIdHelpers;
using namespace Belle2;
using namespace std;


// --------------------    MODULE    ---------------------------------------------------

REG_MODULE(IDValidation);

IDValidationModule::IDValidationModule(): Module()
{
  setDescription("Module to write data for roc and all sorts of performance analysis.");

  addParam("outPath", m_outPath, "path to put the root file", m_outPath);
}


IDValidationModule::~IDValidationModule()
{
}


// --------------------------------------Module----------------------------------------------
void IDValidationModule::initialize()
{
  // require existence of necessary datastore obj
  StoreArray<KLMCluster>::required();
  StoreArray<MCParticle>::required();
  StoreArray<ECLCluster>::required();

  StoreArray<ECLCluster> eclClusters;
  StoreArray<KLMCluster> klmClusters;
  StoreArray<MCParticle> mcParticles;
  StoreArray<KlId> klids;

  klmClusters.requireRelationTo(mcParticles);
  eclClusters.requireRelationTo(mcParticles);

  klmClusters.requireRelationTo(eclClusters);
  eclClusters.requireRelationTo(klids);
  klmClusters.requireRelationTo(klids);


  // initialize root tree to write stuff into
  m_f = new TFile(m_outPath.c_str(), "recreate");
  m_treeKLM = new TTree("KLMdata", "KLMdata");
  m_treeECL = new TTree("ECLdata", "ECLdata");


  m_treeKLM -> Branch("isKlong",                & m_isKlong);
  m_treeKLM -> Branch("isBeamBKG",              & m_isBeamBKG);
  m_treeKLM -> Branch("isECLMatchedCorrectly",  & m_ECLMatchedCorrectly);
  m_treeKLM -> Branch("KlId",                   & m_KlId);
  m_treeKLM -> Branch("BKGId",                  & m_BKGId);
  m_treeKLM -> Branch("PDG",                    & m_PDG);
  m_treeKLM -> Branch("Phi",                    & m_Phi);
  m_treeKLM -> Branch("Theta",                  & m_Theta);
  m_treeKLM -> Branch("Z",                      & m_Z);
  m_treeKLM -> Branch("Reconstructed",          & m_reconstructed);
  m_treeKLM -> Branch("Faked",                  & m_faked);
  m_treeKLM -> Branch("Momentum",               & m_momentum);
  m_treeKLM -> Branch("nKLMClusters",           & m_nKLMCluster);
  m_treeKLM -> Branch("nECLClusters",     & m_nECLCluster);

  m_treeECL -> Branch("isKlong",         & m_isKlong);
  m_treeECL -> Branch("isBeamBKG",       & m_isBeamBKG);
  m_treeECL -> Branch("KlId",            & m_KlId);
  m_treeECL -> Branch("BKGId",           & m_BKGId);
  m_treeECL -> Branch("PDG",             & m_PDG);
  m_treeECL -> Branch("Phi",             & m_Phi);
  m_treeECL -> Branch("Theta",           & m_Theta);
  m_treeECL -> Branch("Z",               & m_Z);
  m_treeECL -> Branch("Reconstructed",   & m_reconstructed);
  m_treeECL -> Branch("Faked",           & m_faked);
  m_treeECL -> Branch("Momentum",        & m_momentum);
  m_treeECL -> Branch("nECLClusters",     & m_nECLCluster);
  m_treeECL -> Branch("nKLMClusters",           & m_nKLMCluster);

}//init


void IDValidationModule::beginRun()
{
}

void IDValidationModule::endRun()
{
}

void IDValidationModule::event()
{
  // objects needed
  StoreArray<MCParticle> mcParticles;
  StoreArray<KLMCluster> klmClusters;
  StoreArray<ECLCluster> eclClusters;

// ------------------ KLM CLUSTERS


  m_nKLMCluster = klmClusters.getEntries();

  m_nECLCluster = eclClusters.getEntries();

  for (const KLMCluster& cluster : klmClusters) {

    // find mc truth
    // go thru all particles mothers up to Y4s and check if its a Klong
    m_ECLMatchedCorrectly = 0;
    MCParticle* particle = cluster.getRelatedTo<MCParticle>();
    if (particle) {m_momentum = particle->getMomentum().Mag2();}
    else {m_momentum = -999;}

    m_isKlong   = mcParticleIsKlong(particle);
    m_isBeamBKG = mcParticleIsBeamBKG(particle);

    m_Phi       = cluster.getMomentum().Phi();
    m_Theta     = cluster.getMomentum().Theta();
    m_Z         = cluster.getClusterPosition().Z();


    if (m_isBeamBKG) {
      m_PDG = 0;
    } else {

      m_PDG = std::abs(particle -> getPDG());

      // now check if ecl is associated correctly
      if (m_isKlong) {

        ECLCluster* AssociatedECL = cluster.getRelatedTo<ECLCluster>();
        MCParticle* ECLParticle = AssociatedECL -> getRelatedTo<MCParticle>();
        m_ECLMatchedCorrectly = mcParticleIsKlong(ECLParticle);
      }
    }

    // get vars from klong id object
    KlId* klid = cluster.getRelatedTo<KlId>();
    if (klid) {
      m_KlId = klid->getKlId();
      m_BKGId = klid->getBkgProb();
    } else {
      m_KlId = 0;
      m_BKGId = 0;
    }

    //TODO cut is arbitrary, aimed at 0.4 purity, which is 1 sigma to the left side of
    //the classifier output fitted by eye
    double classifierCut = 0.525;
    if (m_KlId > classifierCut) {m_reconstructed = 1; }
    else {m_reconstructed = 0;}

    if (m_reconstructed && (!m_isKlong)) {m_faked = 1;}
    else {m_faked = 0;}

    m_KlIdKLM = m_KlId;

    // fill klm tree
    m_treeKLM -> Fill();
  }// for klmcluster in klmclusters


// ---------------   ECL CLUSTERS

  // loop thru eclclusters in event and calculate vars
  for (const ECLCluster& cluster : eclClusters) {

    // find mc truth
    // go thru all particles mothers up to Y4s and check if its a Klong
    MCParticle* particle = cluster.getRelatedTo<MCParticle>();
    if (particle) {m_momentum = particle->getMomentum().Mag2();}
    else {m_momentum = -999;}

    m_isKlong = mcParticleIsKlong(particle);
    m_isBeamBKG = mcParticleIsBeamBKG(particle);


    m_Phi = cluster.getMomentum().Phi();
    m_Theta = cluster.getMomentum().Theta();
    m_Z = cluster.getclusterPosition().Z();



    if (m_isBeamBKG) {
      m_PDG = 0;
    } else {
      m_PDG = std::abs(particle -> getPDG());
    }


    // get vars from klong id object
    KlId* klid = cluster.getRelatedTo<KlId>();
    if (klid) {
      m_KlId = klid->getKlId();
      m_BKGId = klid->getBkgProb();
    } else {
      m_KlId = 0;
      m_BKGId = 0;
    }

    double classifierCut = 0.55;
    if (m_KlId > classifierCut) {m_reconstructed = 1; }
    else {m_reconstructed = 0;}

    if (m_reconstructed && (!m_isKlong)) {m_faked = 1;}
    else {m_faked = 0;}

    m_KlIdECL = m_KlId;


    // finally fill tree
    m_treeECL -> Fill();
  }// for ecl cluster in clusters

} // event


void IDValidationModule::terminate()
{
  // close root files
  m_f    -> cd(); // neccesarry (frame work has another open tree)
  m_treeKLM -> Write();
  m_treeECL -> Write();
  m_f    -> Close();
}




