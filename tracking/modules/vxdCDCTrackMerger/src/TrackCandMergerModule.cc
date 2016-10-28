#include <tracking/modules/vxdCDCTrackMerger/TrackCandMergerModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include "genfit/TrackCand.h"
#include "genfit/RKTrackRep.h"
//#include "genfit/AbsTrackRep.h"
#include <mdst/dataobjects/MCParticle.h>

//avoid having to wrap everything in the namespace explicitly
//only permissible in .cc files!
using namespace Belle2;

REG_MODULE(TrackCandMerger)

TrackCandMergerModule::TrackCandMergerModule() : Module()
{
  setDescription("VXDCDCTrackMerger module helper. This module merges track candidates reconstructed separately in the CDC and the VXD for fitting purposes");
  addParam("SiTrackCandidatesColName",  m_SiTrackCandidatesColName,  "Silicon GFTrack Candidate Collection");
  addParam("CDCTrackCandidatesColName", m_CDCTrackCandidatesColName, "CDC Track Candidate Collection");
  addParam("TrackCandidatesCollection", m_TrackCandidatesCollection, "Output Track Candidate Collection");
  //  addParam("MCParticlesColName", m_mcParticlesColName, "Name of collection holding the MCParticles (need to create relations between found tracks and MCParticles)", string(""));
}


TrackCandMergerModule::~TrackCandMergerModule()
{
}


void TrackCandMergerModule::initialize()
{
  //StoreArray<MCParticle> mcParticles("");
  //mcParticles.isRequired();

  StoreArray<genfit::TrackCand>::required(m_SiTrackCandidatesColName);
  StoreArray<genfit::TrackCand>::required(m_CDCTrackCandidatesColName);
  StoreArray<genfit::TrackCand>::registerPersistent(m_TrackCandidatesCollection);
}

void TrackCandMergerModule::beginRun()
{
}

void TrackCandMergerModule::event()
{

  StoreArray<MCParticle> mcParticles("");

  //get silicon candidate tracks
  StoreArray<genfit::TrackCand> si_mcTracks(m_SiTrackCandidatesColName);
  unsigned int nSiTracks = si_mcTracks.getEntries();

  B2INFO("TrackCandMerger: input Number of Silicon Cands: " << nSiTracks);
  if (nSiTracks == 0) B2WARNING("TrackCandMerger: SiTracksCandCol is empty!");

  //get CDC tracks
  StoreArray<genfit::TrackCand> cdc_mcTracks(m_CDCTrackCandidatesColName);
  unsigned int nCDCTracks = cdc_mcTracks.getEntries();

  B2INFO("TrackCandMerger: input Number of CDC Cands: " << nCDCTracks);
  if (nCDCTracks == 0) B2WARNING("TrackCandMerger: CDCTracksCandCol is empty!");

  //Merge
  StoreArray<genfit::TrackCand> mcTracksCand(m_TrackCandidatesCollection);

  for (unsigned int i = 0; i < nCDCTracks; i++)
    mcTracksCand.appendNew(*(cdc_mcTracks[i]));

  for (unsigned int i = 0; i < nSiTracks; i++)
    mcTracksCand.appendNew(*(si_mcTracks[i]));

}

void TrackCandMergerModule::endRun()
{
}

void TrackCandMergerModule::terminate()
{
}
