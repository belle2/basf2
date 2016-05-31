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

using namespace Belle2;
using namespace std;



/** go thru mother particles if theres a klong return true, else return false  */
bool isParticleKlong(MCParticle* particle)
{
  // shouldnt be the case but ...
  if (!particle) {return false;}

  while (!(particle -> getMother() == nullptr)) {
    if (particle -> getPDG() == 130) {
      return true;
    }//if
    particle = particle -> getMother();
  }// while
  return false;
}//end function




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

  m_treeECL -> Branch("isKlong",                & m_isKlong);
  m_treeECL -> Branch("isBeamBKG",              & m_isBeamBKG);
  m_treeECL -> Branch("KlId",                   & m_KlId);
  m_treeECL -> Branch("BKGId",                  & m_BKGId);
  m_treeECL -> Branch("PDG",                    & m_PDG);


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

  for (const KLMCluster& cluster : klmClusters) {

    // find mc truth
    // go thru all particles mothers up to Y4s and check if its a Klong
    m_isKlong = 0;
    m_isBeamBKG = 0;
    m_ECLMatchedCorrectly = 0;
    MCParticle* particle = cluster.getRelatedTo<MCParticle>();
    if (particle == nullptr) {
      m_isBeamBKG = 1; // this is the case for beambkg
      m_PDG = 0;
    } else {

      m_PDG = std::abs(particle -> getPDG());

      if (isParticleKlong(particle)) {
        m_isKlong = 1;

        // now check if ecl is associated correctly
        ECLCluster* AssociatedECL = cluster.getRelatedTo<ECLCluster>();
        MCParticle* ECLParticle = AssociatedECL -> getRelatedTo<MCParticle>();
        if (isParticleKlong(ECLParticle)) {
          m_ECLMatchedCorrectly = 1;
        }
      }
    }

    // get vars from klong id object
    KlId* klid = cluster.getRelatedTo<KlId>();
    m_KlId = klid->getKlId();
    m_BKGId = klid->getBkgProb();


    // fill klm tree
    m_treeKLM -> Fill();
  }// for klmcluster in klmclusters


// ---------------   ECL CLUSTERS

  // loop thru eclclusters in event and calculate vars
  for (const ECLCluster& cluster : eclClusters) {

    // find mc truth
    // go thru all particles mothers up to Y4s and check if its a Klong
    m_isKlong = 0;
    m_isBeamBKG = 0;
    MCParticle* particle = cluster.getRelatedTo<MCParticle>();
    if (particle == nullptr) {
      m_isBeamBKG = 1; // this is the case for beambkg
      m_PDG = 0;
    } else {

      m_PDG = std::abs(particle -> getPDG());

      if (isParticleKlong(particle)) {
        m_isKlong = 1;
      }//if is klong
    }// else: particle is not null


    // get vars from klong id object
    KlId* klid = cluster.getRelatedTo<KlId>();
    if (klid) {
      m_KlId = klid->getKlId();
      m_BKGId = klid->getBkgProb();
    } else {
      m_KlId = 0;
      m_BKGId = 0;
    }

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




