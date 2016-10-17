#include <tracking/modules/vxdCDCTrackMerger/GFTrackSplitterModule.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include "genfit/TrackCand.h"
#include "genfit/RKTrackRep.h"

#include "cdc/dataobjects/CDCHit.h"

//#include "genfit/AbsTrackRep.h"
#include <mdst/dataobjects/MCParticle.h>

//avoid having to wrap everything in the namespace explicitly
//only permissible in .cc files!
using namespace Belle2;

REG_MODULE(GFTrackSplitter)

GFTrackSplitterModule::GFTrackSplitterModule() : Module()
{
  setDescription("VXDCDCTrackMerger helper module. This module splits an array of GFtracks in two arrays corresponding to the CDC and VXD track candidates respectively.");
  addParam("SiGFTracksColName",  m_SiGFTracksColName,  "Silicon GFTrack Ouput Collection");
  addParam("CDCGFTracksColName", m_CDCGFTracksColName, "CDC Track Output Collection");
  addParam("GFTracksColName", m_GFTracksColName, "Input GF Track Collection");
  addParam("storeTrackCandName", m_storeTrackCandName, "Input Track Cand Collection");
  //  addParam("CDCHitsColName", m_CDCHitsColName, "CDC Hits");
  //addParam("SVDHitsColName", m_SVDHitsColName, "SVD Hits");
  //addParam("PXDHitsColName", m_PXDHitsColName, "PXD Hits");
}


GFTrackSplitterModule::~GFTrackSplitterModule()
{
}


void GFTrackSplitterModule::initialize()
{
  StoreArray<MCParticle> mcParticles("");
  mcParticles.isRequired();

  StoreArray<genfit::Track>::registerPersistent(m_SiGFTracksColName);
  //siTracks.isRequired();

  StoreArray<genfit::Track>::registerPersistent(m_CDCGFTracksColName);
  //cdcTracks.isRequired();

  //RelationArray siTrackToMC(siTracks, mcParticles);
  //siTrackToMC.isRequired();

  //RelationArray cdcTrackToMC(cdcTracks, mcParticles);
  //cdcTrackToMC.isRequired();

  StoreArray<genfit::TrackCand>::required(m_storeTrackCandName);
  StoreArray<genfit::Track>::required(m_GFTracksColName);
  //tracks.isRequired();

  //RelationArray tracksToMC( tracks, mcParticles );
  //tracksToMC.registerAsPersistent();

}

void GFTrackSplitterModule::beginRun()
{
}

void GFTrackSplitterModule::event()
{

  StoreArray<MCParticle> mcParticles("");

  const StoreArray<genfit::TrackCand> storeTrackCand(m_storeTrackCandName);

  //get tracks
  StoreArray<genfit::Track> GFTracksColName(m_GFTracksColName);
  unsigned int nTracks = GFTracksColName.getEntries();

  B2INFO("GFTrackSplitter: total number of tracks (CDC+VXD): " << nTracks);
  if (nTracks == 0) B2WARNING("GFTracksColName is empty!");

  StoreArray<genfit::Track> si_mcTracks(m_SiGFTracksColName);
  StoreArray<genfit::Track> cdc_mcTracks(m_CDCGFTracksColName);

  //SPLIT
  //int htrack=-1;
  for (unsigned int i = 0; i < nTracks; i++) {
    genfit::Track* aTrkPtr = GFTracksColName[i];
    if (DataStore::getRelatedToObj<genfit::TrackCand>(aTrkPtr, m_storeTrackCandName) != NULL) {
      const genfit::TrackCand* aTrkCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(aTrkPtr, m_storeTrackCandName);
      if (aTrkCandPtr == NULL) {
        B2WARNING("Track candidate pointer is NULL (GFSplitterModule)");
        continue;
      }
      std::vector<int> cca = aTrkCandPtr->getDetIDs();
      if ((cca[0]) > 2) {
        //for(int cci=0; cci<5; cci++)
        //std::cout << "CDC: " << cca[cci] << std::endl;
        cdc_mcTracks.appendNew(*(GFTracksColName[i]));
        //htrack++;
      } else {
        si_mcTracks.appendNew(*(GFTracksColName[i]));
        //std::vector<int> svb=aTrkCandPtr->getDetIDs();
        //for(int svj=0; svj<5; svj++)
        //  std::cout << "Si: " << svb[svj] << std::endl;
      }
    }
  }
}

void GFTrackSplitterModule::endRun()
{
}

void GFTrackSplitterModule::terminate()
{
}
