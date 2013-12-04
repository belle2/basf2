#include <tracking/modules/vertexer/VertexerModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>
//#include <generators/dataobjects/MCParticle.h>
#include <tracking/gfbfield/GFGeant4Field.h>


#include <genfit/Track.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/Exception.h>
#include <genfit/TrackCand.h>
#include <genfit/ConstField.h>
#include <genfit/FieldManager.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/MaterialEffects.h>
#include <genfit/GFRaveVertexFactory.h>
#include <genfit/GFRaveVertex.h>

#include <TVector3.h>
#include <TMatrixD.h>
#include <TGeoManager.h>



using namespace Belle2;

using std::string;
using std::vector;

REG_MODULE(Vertexer)


VertexerModule::VertexerModule() : Module()
{
  setDescription("Wrapper module for GFRave. Will get genfit::Tracks from the dataStore and put fitted GFRaveVertices onto the dataStore.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("GFRaveVerbosityLevel", m_verbosity, "Integer will control the verbosity of the GFRaveFactory class", 0);
  addParam("vertexingMethod", m_method, "Select vertexing method", string("kalman-smoothing:1"));
  addParam("useBeamSpot", m_useBeamSpot, "use beam spot info or not (not is default)", false);
  addParam("useGenfitPropagation", m_useGenfitPropagation, "use either the Genfit propagation (default) or the Rave propagation (vacuum only)", true);
  addParam("beamSpotPosition", m_beamSpotPos, "the position of the beam spot", vector<double>(0));
  addParam("beamSpotCovariance", m_beamSpotCov, "the covariance matrix of the beam spot position", vector<double>(0));
  addParam("genfit::TracksColName", m_gfTracksColName, "Name of collection of genfit::Tracks used for input", string(""));
  addParam("extrapolateToInterActionRegion", m_extrapolateToIR, "if true extrapolate the genfit::Tracks to the interaction point before giving them to the vertex fit", false);
}


void VertexerModule::initialize()
{
  StoreArray<genfit::Track>::required();
  if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
    //pass the magnetic field to genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());
    genfit::MaterialEffects::getInstance()->setMscModel("Highland");
  }
  //register output datastore
  StoreArray<genfit::GFRaveVertex>::registerPersistent();// vertices;

  m_gfRaveVertexFactoryPtr = new genfit::GFRaveVertexFactory(m_verbosity, not m_useGenfitPropagation);
  m_gfRaveVertexFactoryPtr->setMethod(m_method);
  if (m_useBeamSpot == true) {
    if (m_beamSpotPos.size() == 3 and m_beamSpotCov.size() == 9) {
      TVector3 beamSpotPos(m_beamSpotPos[0], m_beamSpotPos[1], m_beamSpotPos[2]);
      TMatrixDSym beamSpotCov(3, &m_beamSpotCov[0]); //when C++2011 is used .data() should used instead
      m_gfRaveVertexFactoryPtr->setBeamspot(beamSpotPos, beamSpotCov);
//      beamSpotPos.Print();
//      beamSpotCov.Print();
    } else {
      m_useBeamSpot = false;
      B2ERROR("beamSpotPostion did not have exactly 3 elements or beamSpotCovariance did not have exactly 9 elements therefore beam spot info cannot be used");
    }

  }

}

void VertexerModule::beginRun()
{
  m_ndfTooSmallCounter = 0;
  m_fittedVertices = 0;
  m_extrapFailed = 0;
}

void VertexerModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();

  B2DEBUG(100, "********   VertexerModule  processing event number: " << eventCounter << " ************");
  StoreArray<genfit::Track> gfTracks(m_gfTracksColName);
  const int nGfTracks = gfTracks.getEntries();
  //StoreArray<MCParticle> mcParticles;
  //StoreArray<genfit::TrackCand> trackCandidates;

  int ndf = 2 * nGfTracks;
  if (m_useBeamSpot == true) {
    ndf += 3;
  }
  if (ndf < 4) {
    B2DEBUG(100, "event " << eventCounter <<  " has not enough information to fit at least one vertex. Event will be skipped");
    ++m_ndfTooSmallCounter;
    return;
  }

  StoreArray<genfit::GFRaveVertex> vertices; //the output datastore
  vertices.create();

  B2DEBUG(100, " will feed  " << nGfTracks << " tracks to GFRave");


  //get all tracks of one event
  vector<genfit::Track*> tracksForRave(nGfTracks);
  vector<genfit::MeasuredStateOnPlane*> statesForRave(nGfTracks);
  for (int i = 0; i not_eq nGfTracks; ++i) {
    if (m_extrapolateToIR == true) { //
      genfit::MeasuredStateOnPlane* state = NULL;
      try {
        TVector3 pos(0., 0., 0.); //origin assume the interaction point is (0,0,0)

        genfit::MeasuredStateOnPlane* state = new genfit::MeasuredStateOnPlane(gfTracks[i]->getFittedState());
        state->extrapolateToPoint(pos);

        tracksForRave[i] =  gfTracks[i];
        statesForRave[i] = state;
      } catch (...) {
        ++m_extrapFailed;
        delete state;
      }

    } else
      tracksForRave[i] =  gfTracks[i];
  }

  vector < genfit::GFRaveVertex* > verticesFromRave;

  if (m_extrapolateToIR == true)
    m_gfRaveVertexFactoryPtr->findVertices(&verticesFromRave, tracksForRave, statesForRave, m_useBeamSpot);
  else
    m_gfRaveVertexFactoryPtr->findVertices(&verticesFromRave, tracksForRave, m_useBeamSpot);

  const int nVerticesFromRave = verticesFromRave.size();
  B2DEBUG(100, nVerticesFromRave << " vertices were found/fitted in event " << eventCounter);
  m_fittedVertices += nVerticesFromRave;
//write the fitted vertices to the storeArray and clean up the stuff created with new
  for (int i = 0; i not_eq nVerticesFromRave; ++i) {
    vertices.appendNew(*(verticesFromRave[i]));
    B2DEBUG(100, "Vertex " << i << " has " << verticesFromRave[i]->getNTracks() << " tracks, total χ² value " << verticesFromRave[i]->getChi2() << " and position " <<  verticesFromRave[i]->getPos()[0] << " " <<  verticesFromRave[i]->getPos()[1] << " " <<  verticesFromRave[i]->getPos()[2])
    delete verticesFromRave[i];
  }

}


void VertexerModule::endRun()
{
  if (m_ndfTooSmallCounter not_eq 0) {
    B2WARNING(m_ndfTooSmallCounter << " events had too little information to reconstruct at least one vertex");
  }
  if (m_extrapFailed not_eq 0) {
    B2WARNING(m_extrapFailed << "tracks could not be extrapolated to the interaction region");
  }
  B2INFO(m_fittedVertices << " vertices were fitted by Rave in this Run");
}


void VertexerModule::terminate()
{
  delete m_gfRaveVertexFactoryPtr;
}
