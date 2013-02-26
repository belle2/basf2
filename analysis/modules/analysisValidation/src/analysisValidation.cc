/*******************************************************************************
 * Module: analysisValidation                                                  *
 *                                                                             *
 * Contributors: Phillip Urquijo                                               *
 *                                                                             *
 * Description: This modules checks quantities used in the analysis package    *
 *              - results are monitored through analysis/validation            *
 *******************************************************************************/

#include <analysis/modules/analysisValidation/analysisValidation.h>
#include <framework/core/Environment.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include <generators/utilities/cm2LabBoost.h>

#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/TrackFitResult.h>
#include <ecl/dataobjects/ECLGamma.h>

#include <analysis/utility/physics.h>
#include <analysis/KFit/MakeMotherKFit.h>

using namespace std;
using namespace Belle2;
using namespace analysis;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(analysisValidation)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

analysisValidationModule::analysisValidationModule() : Module()
{
  //Set module properties
  setDescription("analysisValidation Module");

  //Parameters definition
  addParam("outputFileName",           m_dataOutFileName,
           "Output root file name of this module. Suffixes to distinguish them will be added automatically", string("analysisValidation"));
  addParam("MCParticlesColName",       m_mcParticlesColName, "MCParticles collection name" ,     string(""));
  addParam("GFTrackCandidatesColName", m_gfTrackCandColName, "GF Track cand. collection name",   string(""));
  addParam("GFTracksColName",          m_gfTracksColName,    "GFTracks collection name",         string(""));
  addParam("TracksColName",            m_tracksColName,      "Tracks collection name",           string(""));
  addParam("ECLGammasColName",         m_ECLgammasColName,   "ECLGammas collection name",        string(""));
  addParam("ParticlesColName",         m_ParticlesColName,   "Particles collection name",        string("Particles"));
  addParam("TestType",                 m_TestType,           "Test type: dst, udst",             string("dst"));

}

void analysisValidationModule::initialize()
{
  B2INFO("[analysisValidation Module]: Starting initialization of analysisValidation Module.");

  nevt = 0;

  // Initialize store arrays
  StoreArray<MCParticle>     mcParticles(m_mcParticlesColName);
  StoreArray<GFTrackCand>    gfTrackCand(m_gfTrackCandColName);
  StoreArray<GFTrack>        gfTracks(m_gfTracksColName);
  StoreArray<Track>          tracks(m_tracksColName);
  StoreArray<ECLGamma>       gammas(m_ECLgammasColName);
  StoreArray<ECLShower>      showers;

  RaveSetup::initialize("GFRave");
  StoreArray<Particle>::registerPersistent(m_ParticlesColName);
  RelationArray::registerPersistent<Particle, Track>(m_ParticlesColName, m_tracksColName);
  RelationArray::registerPersistent<Particle, ECLGamma>();

  // Initializing the output root file
  string dataFileName = m_dataOutFileName + ".root";
  m_rootFile = new TFile(dataFileName.c_str(), "RECREATE");

  h_nMCParticles = new TH1I("h_nMCParticles", ";nMCParticles;events", 290, -0.5, 289.5);
  h_nParticles   = new TH1I("h_nParticles"  , ";nParticles;events"  , 290, -0.5, 289.5);
  h_nmcphotons   = new TH1I("h_nmcphotons"  , ";nmcphotons;events"  , 290, -0.5, 289.5);
  h_nTracks      = new TH1I("h_nTracks"     , ";nTracks;events"     , 90, -0.5, 89.5);
  h_nGFTracks    = new TH1I("h_nGFTracks"   , ";nGFTracks;events"   , 90, -0.5, 89.5);
  h_nECLGammas   = new TH1I("h_nECLGammas"  , ";nECLGammas;events"  , 90, -0.5, 89.5);
  h_nECLShowers  = new TH1I("h_nECLShowers" , ";nECLShowers;events" , 90, -0.5, 89.5);

  B2INFO("[analysisValidation Module]: Finished initialising the EvtGen Study Module.");

}

void analysisValidationModule::terminate()
{

  h_nMCParticles->Write();
  h_nParticles->Write();
  h_nmcphotons->Write();
  h_nTracks->Write();
  h_nGFTracks->Write();
  h_nECLGammas->Write();
  h_nECLShowers->Write();

  m_rootFile->Close();
}

void analysisValidationModule::event()
{
  B2INFO("[analysisValidation Module]: Begin Event.");

  nevt++;

  /** Check that DST/MDST access through DataStore is working correctly**/
  if (m_TestType == "dst")checkDSTContents();

  /** Check that UDST access through DataStore is working correctly**/
  if (m_TestType == "udst")checkUDSTContents();

  B2INFO("[analysisValidation] - end of event!");
}


void analysisValidationModule::checkDSTContents()
{
  /** ECL gamma container **/
  StoreArray<ECLGamma>    eclgammas(m_ECLgammasColName);
  h_nECLGammas->Fill((int) eclgammas.getEntries());
  B2INFO(boost::format("[analysisValidation Module]: Number of photons: %d") % eclgammas.getEntries());

  /** ECL shower container **/
  StoreArray<ECLShower>   showers;
  h_nECLShowers->Fill((int) showers.getEntries());

  /** Tracks container **/
  StoreArray<Track>  tracks(m_tracksColName);
  h_nTracks->Fill(tracks.getEntries());

  /** GFTracks container **/
  StoreArray<GFTrack>  gfTracks(m_gfTracksColName);
  h_nGFTracks->Fill(gfTracks.getEntries());

  /** MC particles container **/
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  h_nMCParticles->Fill((int) mcParticles.getEntries());

}

void analysisValidationModule::checkUDSTContents()
{
  /** Particles container **/
  StoreArray<Particle> particles;
  h_nParticles->Fill((int) particles.getEntries());

  /** Test relations **/
  /** monitoring histogram to be prepared **/
  StoreArray<ECLGamma>    eclgammas(m_ECLgammasColName);
  StoreArray<Track>       tracks(m_tracksColName);
  RelationArray particleToTrack(particles, tracks);
  RelationArray particleToGamma(particles, eclgammas);

}


