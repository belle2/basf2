#include <tracking/modules/vertexer/VertexerModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>
//#include <generators/dataobjects/MCParticle.h>
#include <tracking/gfbfield/GFGeant4Field.h>


#include <GFTrack.h>
#include <GFException.h>
#include <GFTrackCand.h>
#include <GFConstField.h>
#include <GFFieldManager.h>


#include <GFRaveVertex.h>

using namespace Belle2;

using std::string;
using std::vector;

REG_MODULE(Vertexer)


VertexerModule::VertexerModule() : Module()
{
  setDescription("Wrapper module for GFRave. Will get GFTracks from the dataStore and put fitted GFRaveVertices onto the dataStore.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("GFRaveVerbosityLevel", m_verbosity, "Integer will control the verbosity of the GFRaveFactory class", 0);
  addParam("vertexingMethod", m_method, "Select vertexing method", string("kalman-smoothing:1"));
  addParam("useBeamSpot", m_useBeamSpot, "use beam spot info or not (not is default)", false);
  addParam("useGenfitPropagation", m_useGenfitPropagation, "use either the Genfit propagation (default) or the Rave propagation (vacuum only)", true);
  addParam("beamSpotPostion", m_beamSpotPos, "the postion of the beam spot", TVector3());
  addParam("beamSpotCovariance", m_beamSpotCov, "the covariance matrix of the beam spot postion", TMatrixD());

}


void VertexerModule::initialize()
{
  //setup genfit geometry and magneic field in case you what to used data saved on disc because then the genifitter modul was not run
  // convert the geant4 geometry to a TGeo geometry
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  geoManager.createTGeoRepresentation();
  //pass the magnetic field to genfit
  GFFieldManager::getInstance()->init(new GFGeant4Field());

  //register output datastore
  StoreArray<GFRaveVertex> vertices;

  m_gfRaveVertexFactoryPtr = new GFRaveVertexFactory(m_verbosity, not m_useGenfitPropagation);
  m_gfRaveVertexFactoryPtr->setMethod(m_method);
  if (m_useBeamSpot == true) {
    m_gfRaveVertexFactoryPtr->setBeamspot(m_beamSpotPos, m_beamSpotCov);
  }

}

void VertexerModule::beginRun()
{

}

void VertexerModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();

  B2DEBUG(100, "********   VertexerModule  processing event number: " << eventCounter << " ************");
  StoreArray<GFTrack> gfTracks;
  const int nGfTracks = gfTracks.getEntries();
  //StoreArray<MCParticle> mcParticles;
  StoreArray<GFTrackCand> trackCandidates;

  if (nGfTracks == 0) {
    B2DEBUG(100, "event " << eventCounter <<  " has 0 GFTrack objects. Nothing will be done for this event");
    return;
  }

  StoreArray<GFRaveVertex> vertices; //the output datastore

  B2DEBUG(100, " will feed  " << nGfTracks << " tracks to GFRave");




  //get all tracks of one event
  vector<GFTrack*> tracksForRave(nGfTracks);
  for (int i = 0; i not_eq nGfTracks; ++i) {
    tracksForRave[i] =  gfTracks[i];

  }

  vector < GFRaveVertex* > verticesFromRave;

  m_gfRaveVertexFactoryPtr->findVertices(&verticesFromRave, tracksForRave, true);

  const int nVerticesFromRave = verticesFromRave.size();
//write the fitted vertices to the storeArray and clean up the stuff created with new
  for (int i = 0; i not_eq nVerticesFromRave; ++i) {
    vertices.appendNew(*(verticesFromRave[i]));
    delete verticesFromRave[i];
  }

}
void VertexerModule::endRun()
{


}
void VertexerModule::terminate()
{
  delete m_gfRaveVertexFactoryPtr;
}
