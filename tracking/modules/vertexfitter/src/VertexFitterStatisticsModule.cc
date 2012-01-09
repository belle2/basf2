#include <tracking/modules/vertexfitter/VertexFitterStatisticsModule.h>

#include <tracking/dataobjects/VertexFitterVertexContainer.h>

#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>
#include <GFTrack.h>
#include <GFTrackCand.h>
#include <TFile.h>

#include <fstream>

#include "TVector3.h"


using namespace Belle2;

REG_MODULE(VertexFitterStatistics)

static bool copyFile(const char* const from, const char* const to)
{
  std::fstream fromFile(from, std::ios_base::binary | std::ios_base::in);
  std::fstream toFile(to, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

  if (!fromFile.good() || !toFile.good())
    return false;

  toFile << fromFile.rdbuf();
  return true;
}

VertexFitterStatisticsModule::VertexFitterStatisticsModule() : Module(), vertexDisplacementHistogram(NULL)
{
  setDescription("Statistics of VertexFitter/RAVE results.");

  addParam("RAVEVerticesColName", m_RAVEVerticesColName, "Name of collection holding the rave::Vertex instances.", std::string(""));
  addParam("MCParticlesColName", m_MCParticlesColName, "Name of collection holding the MCParticle instances.", std::string(""));
  addParam("GFTrackCandsColName", m_GFTrackCandsColName, "Name of collection holding the GFTrackCand instances.", std::string(""));
  addParam("GFTracksColName", m_GFTracksColName, "Name of collection holding the GFTrack instances.", std::string(""));
  addParam("resultFilePath", m_resultFilePath, "");
  addParam("steeringFile", m_steeringFile, "");
}

VertexFitterStatisticsModule::~VertexFitterStatisticsModule()
{
}

void VertexFitterStatisticsModule::initialize()
{
}

void VertexFitterStatisticsModule::beginRun()
{
  trackMomentumHistogram = new TH1D("trackMomentum", "Track Momentum: Magnitude of track momentum vector", 20000, 0, 8);
  trackMomentumXHistogram = new TH1D("trackMomentumX", "Track Momentum X: X-component of track momentum vector", 10000, -8, 8);
  trackMomentumYHistogram = new TH1D("trackMomentumY", "Track Momentum Y: Y-component of track momentum vector", 10000, -8, 8);
  trackMomentumZHistogram = new TH1D("trackMomentumZ", "Track Momentum Z: Z-component of track momentum vector", 10000, -8, 8);
  vertexDisplacementHistogram = new TH1D("vertexDisplacement", "Vertex Displacement: Magnitude of vector from true to calculated vertex", 20000, 0, 8);
  vertexDisplacementXHistogram = new TH1D("vertexDisplacementX", "Vertex Displacment X: X-component of vector from true to calculated vertex", 20000, -8, 8);
  vertexDisplacementYHistogram = new TH1D("vertexDisplacementY", "Vertex Displacment Y: Y-component of vector from true to calculated vertex", 20000, -8, 8);
  vertexDisplacementZHistogram = new TH1D("vertexDisplacementZ", "Vertex Displacment Z: Z-component of vector from true to calculated vertex", 20000, -8, 8);
  standardisedResidualXHistogram = new TH1D("standardisedResidualX", "Standardised Residual in X-coordinate of the vertex.", 20000, -100, 100);
  standardisedResidualYHistogram = new TH1D("standardisedResidualY", "Standardised Residual in Y-coordinate of the vertex.", 20000, -100, 100);
  standardisedResidualZHistogram = new TH1D("standardisedResidualZ", "Standardised Residual in Z-coordinate of the vertex.", 20000, -0.1, 0.1);
  trueVertexXHistogram = new TH1D("trueVertexX", "True Vertex X coordinate", 10000, -0.25, 0.25);
  trueVertexYHistogram = new TH1D("trueVertexY", "True Vertex Y coordinate", 10000, -0.25, 0.25);
  trueVertexZHistogram = new TH1D("trueVertexZ", "True Vertex Z coordinate", 10000, -0.25, 0.25);
  chiSquaredHistogram = new TH1D("chiSquared", "chi^2", 10000, -0.1, 1000);
  probHistogram = new TH1D("prob", "Prob(chi2, ndf)", 10000, -0.1, 1.1);
  trackProbHistogram = new TH1D("trackProb", "Track's Prob(chi2, ndf)", 500000, -0.1, 1.1);

  trackMomentumHistogram->SetBit(TH1::kCanRebin);
  trackMomentumXHistogram->SetBit(TH1::kCanRebin);
  trackMomentumYHistogram->SetBit(TH1::kCanRebin);
  trackMomentumZHistogram->SetBit(TH1::kCanRebin);
  vertexDisplacementHistogram->SetBit(TH1::kCanRebin);
  vertexDisplacementXHistogram->SetBit(TH1::kCanRebin);
  vertexDisplacementYHistogram->SetBit(TH1::kCanRebin);
  vertexDisplacementZHistogram->SetBit(TH1::kCanRebin);
  trueVertexXHistogram->SetBit(TH1::kCanRebin);
  trueVertexYHistogram->SetBit(TH1::kCanRebin);
  trueVertexZHistogram->SetBit(TH1::kCanRebin);
  chiSquaredHistogram->SetBit(TH1::kCanRebin);
}

void VertexFitterStatisticsModule::event()
{
  StoreArray<VertexFitterVertexContainer> storedVertices(m_RAVEVerticesColName);
  StoreArray<MCParticle> particles(m_MCParticlesColName);
  StoreArray<GFTrackCand> trackCandidates(m_GFTrackCandsColName);
  StoreArray<GFTrack> tracks(m_GFTracksColName);

  for (int trackID = 0; trackID < tracks.getEntries(); ++trackID) {
    const TVector3 momentum = tracks[trackID]->getMom();

    trackMomentumHistogram->Fill(momentum.Mag());
    trackMomentumXHistogram->Fill(momentum.X());
    trackMomentumYHistogram->Fill(momentum.Y());
    trackMomentumZHistogram->Fill(momentum.Z());

    B2INFO("rave: track id: " << trackID << ", track chi2: " << tracks[trackID]->getChiSqu() << ", track ndf: " << tracks[trackID]->getNDF() << ", p: " << TMath::Prob(tracks[trackID]->getChiSqu(), tracks[trackID]->getNDF()));

    trackProbHistogram->Fill(TMath::Prob(tracks[trackID]->getChiSqu(), tracks[trackID]->getNDF()));
  }

  for (int vertexID = 0; vertexID < storedVertices.getEntries(); ++vertexID) {
    if (!storedVertices[vertexID]->containsValidVertex())
      continue;

    const std::vector<rave::Track> vertexTracks = storedVertices[vertexID]->getVertex().tracks();
    std::vector<const GFTrackCand*> vertexTrackCandidates;
    B2INFO("**** std::vector<const GFTrackCand*> vertexTrackCandidates; ****")
    vertexTrackCandidates.reserve(vertexTracks.size());

    for (int GFTrackIndex = 0; GFTrackIndex < tracks.getEntries(); ++GFTrackIndex) { //fill vertexTrackCandidates with Candidates from GFTrack
      if (TMath::Prob(tracks[GFTrackIndex]->getChiSqu(), tracks[GFTrackIndex]->getNDF()) < 0.05) // same in VertexFittermodule
      { }
      else {
        vertexTrackCandidates.push_back(&(tracks[GFTrackIndex]->getCand()));
        B2INFO("**** vertexTrackCandidates.push_back(&(tracks[GFTrackIndex]->getCand())); ****")
      }
    }
    if (vertexTrackCandidates.empty()) {
      B2WARNING("ravestat: Vertex has no track candidates associated.");
      return;
    }

    const MCParticle* const creatingParticle = particles[vertexTrackCandidates[0]->getMcTrackId()];

    if (!creatingParticle->hasValidVertex()) { //check for default value
      B2WARNING("ravestat: Creating particle does not have valid vertex.");
      return;
    }

    const TVector3 trueVertex = creatingParticle->getProductionVertex();

    for (unsigned int i = 1; i < vertexTrackCandidates.size(); ++i) {
      const MCParticle* const otherParticle = particles[vertexTrackCandidates[i]->getMcTrackId()];
      if (!otherParticle->hasValidVertex()) {
        B2WARNING("rave: Creating particle does not have valid vertex.");
        return;
      }
      if (otherParticle->getProductionVertex() != trueVertex) {
        B2WARNING("rave: Creating particle does not have valid vertex.");
        return;
      }
    }

    const TVector3 calculatedVertex(storedVertices[vertexID]->getVertex().position().x(), storedVertices[vertexID]->getVertex().position().y(), storedVertices[vertexID]->getVertex().position().z());
    const TVector3 displacement = calculatedVertex - trueVertex;

    B2INFO("ravestat: true Vertex is at x=" << trueVertex.X() << ", y=" << trueVertex.Y() << ", z=" << trueVertex.Z());
    B2INFO("ravestat: calc Vertex is at x=" << calculatedVertex.X() << ", y=" << calculatedVertex.Y() << ", z=" << calculatedVertex.Z());
    B2INFO("ravestat: displacement is   x=" << displacement.X() << ", y=" << displacement.Y() << ", z=" << displacement.Z());
    B2INFO("ravestat: displacement magnitude is " << displacement.Mag());

    vertexDisplacementHistogram->Fill(displacement.Mag());
    vertexDisplacementXHistogram->Fill(displacement.X());
    vertexDisplacementYHistogram->Fill(displacement.Y());
    vertexDisplacementZHistogram->Fill(displacement.Z());

    standardisedResidualXHistogram->Fill(displacement.X() / sqrt(storedVertices[vertexID]->getVertex().error().dxx()));
    standardisedResidualYHistogram->Fill(displacement.Y() / sqrt(storedVertices[vertexID]->getVertex().error().dyy()));
    standardisedResidualZHistogram->Fill(displacement.Z() / sqrt(storedVertices[vertexID]->getVertex().error().dzz()));
    B2INFO("ravestat: standardised residual x: " << displacement.X() / sqrt(storedVertices[vertexID]->getVertex().error().dxx()) << ", y: " << displacement.Y() / sqrt(storedVertices[vertexID]->getVertex().error().dyy()) << ", z: " << displacement.Z() / sqrt(storedVertices[vertexID]->getVertex().error().dzz()));
    B2INFO("ravestat: Var x: " << storedVertices[vertexID]->getVertex().error().dxx());
    B2INFO("standard deviation: " << sqrt(storedVertices[vertexID]->getVertex().error().dxx()));

    trueVertexXHistogram->Fill(trueVertex.X());
    trueVertexYHistogram->Fill(trueVertex.Y());
    trueVertexZHistogram->Fill(trueVertex.Z());

    B2INFO("ravestat: chi2=" << storedVertices[vertexID]->getVertex().chiSquared());
    B2INFO("ravestat: ndf=" << storedVertices[vertexID]->getVertex().ndf());
    B2INFO("ravestat: entries in vertex=" << storedVertices[vertexID]->getVertex().tracks().size());
    if (storedVertices[vertexID]->getVertex().hasRefittedTracks())
    {B2INFO("ravestat: vertex has refitted tracks");}
    else if (!storedVertices[vertexID]->getVertex().hasRefittedTracks())
    {B2INFO("ravestat: vertex does not have refitted tracks");}

    chiSquaredHistogram->Fill(storedVertices[vertexID]->getVertex().chiSquared());
    probHistogram->Fill(TMath::Prob(storedVertices[vertexID]->getVertex().chiSquared(), storedVertices[vertexID]->getVertex().ndf()));
  }
}

void VertexFitterStatisticsModule::endRun()
{
  const std::string rootFilePath = m_resultFilePath + ".root";
  TFile rootFile(rootFilePath.c_str(), "RECREATE");

  trackMomentumHistogram->SetDirectory(&rootFile);
  trackMomentumXHistogram->SetDirectory(&rootFile);
  trackMomentumYHistogram->SetDirectory(&rootFile);
  trackMomentumZHistogram->SetDirectory(&rootFile);
  vertexDisplacementHistogram->SetDirectory(&rootFile);
  vertexDisplacementXHistogram->SetDirectory(&rootFile);
  vertexDisplacementYHistogram->SetDirectory(&rootFile);
  vertexDisplacementZHistogram->SetDirectory(&rootFile);
  standardisedResidualXHistogram->SetDirectory(&rootFile);
  standardisedResidualYHistogram->SetDirectory(&rootFile);
  standardisedResidualZHistogram->SetDirectory(&rootFile);
  trueVertexXHistogram->SetDirectory(&rootFile);
  trueVertexYHistogram->SetDirectory(&rootFile);
  trueVertexZHistogram->SetDirectory(&rootFile);
  chiSquaredHistogram->SetDirectory(&rootFile);
  probHistogram->SetDirectory(&rootFile);
  trackProbHistogram->SetDirectory(&rootFile);

  trackMomentumHistogram->Write();
  trackMomentumXHistogram->Write();
  trackMomentumYHistogram->Write();
  trackMomentumZHistogram->Write();
  vertexDisplacementHistogram->Write();
  vertexDisplacementXHistogram->Write();
  vertexDisplacementYHistogram->Write();
  vertexDisplacementZHistogram->Write();
  standardisedResidualXHistogram->Write();
  standardisedResidualYHistogram->Write();
  standardisedResidualZHistogram->Write();
  trueVertexXHistogram->Write();
  trueVertexYHistogram->Write();
  trueVertexZHistogram->Write();
  chiSquaredHistogram->Write();
  probHistogram->Write();
  trackProbHistogram->Write();

  const std::string steeringFileCopyPath = m_resultFilePath + "_steering.py";
  if (!copyFile(m_steeringFile.c_str(), steeringFileCopyPath.c_str()))
    B2ERROR("ravestat: failed to copy steering file.");

  delete trackMomentumHistogram;
  trackMomentumHistogram = NULL;
  delete trackMomentumXHistogram;
  trackMomentumXHistogram = NULL;
  delete trackMomentumYHistogram;
  trackMomentumYHistogram = NULL;
  delete trackMomentumZHistogram;
  trackMomentumZHistogram = NULL;
  delete vertexDisplacementHistogram;
  vertexDisplacementHistogram = NULL;
  delete vertexDisplacementXHistogram;
  vertexDisplacementXHistogram = NULL;
  delete vertexDisplacementYHistogram;
  vertexDisplacementYHistogram = NULL;
  delete vertexDisplacementZHistogram;
  vertexDisplacementZHistogram = NULL;
  delete standardisedResidualXHistogram;
  standardisedResidualXHistogram = NULL;
  delete standardisedResidualYHistogram;
  standardisedResidualYHistogram = NULL;
  delete standardisedResidualZHistogram;
  standardisedResidualZHistogram = NULL;
  delete trueVertexXHistogram;
  trueVertexXHistogram = NULL;
  delete trueVertexYHistogram;
  trueVertexYHistogram = NULL;
  delete trueVertexZHistogram;
  trueVertexZHistogram = NULL;
  delete chiSquaredHistogram;
  chiSquaredHistogram = NULL;
  delete probHistogram;
  probHistogram = NULL;
}

void VertexFitterStatisticsModule::terminate()
{
}