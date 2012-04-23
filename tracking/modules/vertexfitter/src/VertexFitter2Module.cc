#include <tracking/modules/vertexfitter/VertexFitter2Module.h>
#include <framework/datastore/StoreArray.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <rave/Vertex.h>
#include <rave/VertexFactory.h>
#include <GFTrack.h>
#include <RKTrackRep.h>
#include <TMath.h>
#include <TFile.h>
#include <GFException.h>
#include <generators/dataobjects/MCParticle.h>
#include <GFTrackCand.h>
#include <fstream>
#include "TVector3.h"

using namespace Belle2;

REG_MODULE(VertexFitter2)

static bool copyFile(const char* const from, const char* const to) // used to cope steering file
{
  std::fstream fromFile(from, std::ios_base::binary | std::ios_base::in);
  std::fstream toFile(to, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

  if (!fromFile.good() || !toFile.good())
    return false;
  toFile << fromFile.rdbuf();
  return true;
}

class RaveDebug {
public:
  static void print(const rave::Vector6D& vec, const std::string& desc = "") {
    B2INFO(" ** VF:  : " << desc << ": x=" << vec.x() << ", y=" << vec.y() << ", z=" << vec.z() << ", px=" << vec.px() << ", py=" << vec.py() << ", pz=" << vec.pz());
  }

  static void print(const rave::Covariance6D& cov, const std::string& desc = "") { // prints all coveriance matrix values
    B2INFO(" ** VF:  : " << desc << ": dxx=" << cov.dxx() << ", dxy=" << cov.dxy() << ", dxz=" << cov.dxz() << ", dyy=" << cov.dyy() << ", dyz=" << cov.dyz() << ", dzz=" << cov.dzz() <<
           ", dxpx=" << cov.dxpx() << ", dxpy=" << cov.dxpy() << ", dxpz=" << cov.dxpz() << ", dypx=" << cov.dypx() << ", dypy=" << cov.dypy() << ", dypz=" << cov.dypz() <<
           ", dzpx=" << cov.dzpx() << ", dzpy=" << cov.dzpy() << ", dzpz=" << cov.dzpz() << ", dpxpx=" << cov.dpxpx() << ", dpxpy=" << cov.dpxpy() <<
           ", dpxpz=" << cov.dpxpz() << ", dpypy=" << cov.dpypy() << ", dpypz=" << cov.dpypz() << ", dpzpz=" << cov.dpzpz());
  }
};


VertexFitter2Module::VertexFitter2Module() : Module()
{
  setDescription("Integration of VertexFitter Rave (Reconstruction in an Abstract Vertices Environment).");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("GFTracksColName", m_GFTracksColName, "Name of collection holding the GFTrack instances.", std::string(""));
  addParam("RAVEVerticesColName", m_RAVEVerticesColName, "Name of collection holding the rave::Vertex instances.", std::string(""));
  addParam("MCParticlesColName", m_MCParticlesColName, "Name of collection holding the MCParticle instances.", std::string(""));
  addParam("GFTrackCandsColName", m_GFTrackCandsColName, "Name of collection holding the GFTrackCand instances.", std::string(""));
  addParam("resultFilePath", m_resultFilePath, "");
  addParam("steeringFile", m_steeringFile, "");
}


void VertexFitter2Module::initialize()
{
}

void VertexFitter2Module::beginRun()
{
  B2INFO(" ** VF:  : beginRun");
  // initialize histograms
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
  trackProbHistogram = new TH1D("trackProb", "Track's Prob(chi2, ndf)", 100000, -0.1, 1.1);

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

void VertexFitter2Module::event()
{
  B2INFO("********   VertexFitter2Module VF  ********");
  StoreArray<GFTrack> GFTracks(m_GFTracksColName);
  StoreArray<MCParticle> particles(m_MCParticlesColName);
  StoreArray<GFTrackCand> trackCandidates(m_GFTrackCandsColName);

  if (GFTracks.getEntries() == 0) {
    B2WARNING(" ** VF:  : no tracks found.");
    return;
  }
  B2INFO(" ** VF:  : processing " << GFTracks.getEntries() << " track(s).");

  for (int trackID = 0; trackID < GFTracks.getEntries(); ++trackID) {
    const TVector3 momentum = GFTracks[trackID]->getMom();
    trackMomentumHistogram->Fill(momentum.Mag());
    trackMomentumXHistogram->Fill(momentum.X());
    trackMomentumYHistogram->Fill(momentum.Y());
    trackMomentumZHistogram->Fill(momentum.Z());
    B2INFO(" ** VF:  : track id: " << trackID <<
           ", track chi2: " << GFTracks[trackID]->getChiSqu() <<
           ", track ndf: " << GFTracks[trackID]->getNDF() <<
           ", p: " << TMath::Prob(GFTracks[trackID]->getChiSqu(), GFTracks[trackID]->getNDF()));
    trackProbHistogram->Fill(TMath::Prob(GFTracks[trackID]->getChiSqu(), GFTracks[trackID]->getNDF()));
  }

  std::vector<rave::Track> raveTracks;
  raveTracks.reserve(GFTracks.getEntries());
  const TVector3 origin(0, 0, 0);

  for (int i = 0; i < GFTracks.getEntries(); ++i) {
    if (dynamic_cast<RKTrackRep*>(GFTracks[i]->getCardinalRep()) == NULL) {
      B2ERROR(" ** VF:  : Track representation is not of the expected type.");
      return;
    }
    RKTrackRep* const track = static_cast<RKTrackRep*>(GFTracks[i]->getCardinalRep());
    if (TMath::Prob(track->getChiSqu(), track->getNDF()) < 0.05) // same in VertexFitterStat Modul
      continue;

    TVector3 poca; //Point Of Closest Approach of the track to the origin.
    TVector3 directionInPoca;
    track->extrapolateToPoint(origin, poca, directionInPoca);
    const GFDetPlane plane(poca, directionInPoca);
    TVector3 position;
    TVector3 momentum;
    TMatrixT<double> covariance;
    track->getPosMomCov(plane, position, momentum, covariance);
    const rave::Vector6D positionAndMomentum(position.X(), position.Y(), position.Z(), momentum.X(), momentum.Y(), momentum.Z());
    const rave::Covariance6D covariance6D(covariance[0][0], covariance[0][1], covariance[0][2], covariance[1][1], covariance[1][2], covariance[2][2],
                                          covariance[0][3], covariance[0][4], covariance[0][5], covariance[1][3], covariance[1][4], covariance[1][5],
                                          covariance[2][3], covariance[2][4], covariance[2][5], covariance[3][3], covariance[3][4], covariance[3][5],
                                          covariance[4][4], covariance[4][5], covariance[5][5]);

    //RaveDebug::print(positionAndMomentum, "track parameters");
    //RaveDebug::print(covariance6D, "covariance matrix"); //prints allvalues ofcovariance matrix
    raveTracks.push_back(rave::Track(positionAndMomentum, covariance6D, track->getCharge(), track->getChiSqu(), track->getNDF(), GFTracks[i]));
  }

  class Belle2MagneticField : public rave::MagneticField {
  public:
    virtual rave::MagneticField* copy() const {
      return new Belle2MagneticField;
    }
    virtual rave::Vector3D inTesla(const rave::Point3D& point) const {
      const TVector3 ret = BFieldMap::Instance().getBField(TVector3(point.x(), point.y(), point.z()));
      return rave::Vector3D(ret.X(), ret.Y(), ret.Z());
    }
    virtual ~Belle2MagneticField() {
    }
  };

  if (raveTracks.size() < 2) {
    B2WARNING(" ** VF:  : less than two usable tracks found. (p <= 0.05)");
    return;
  }
  Belle2MagneticField magneticField;

  const rave::VertexFactory vertexFactory(magneticField);
  const std::vector<rave::Vertex> vertices = vertexFactory.create(raveTracks); // using Kalmanfitter as default
  const rave::Vector3D b = Belle2MagneticField().inTesla(rave::Point3D(1, 2, 3));

  B2INFO(" ** VF:  : B is x=" << b.x() << ", y=" << b.y() << ", z=" << b.z());
  B2INFO(" ** VF:  : found " << vertices.size() << " vertices.");


  for (unsigned int vertexID = 0; vertexID < vertices.size(); ++vertexID) {
    if (!vertices.size() > 0)
      continue;

    std::vector<const GFTrackCand*> vertexTrackCandidates;
    vertexTrackCandidates.reserve(raveTracks.size());

    for (int GFTrackIndex = 1; GFTrackIndex < GFTracks.getEntries(); ++GFTrackIndex) {
      //fill vertexTrackCandidates with Candidates from GFTrack
      if (TMath::Prob(GFTracks[GFTrackIndex]->getChiSqu(), GFTracks[GFTrackIndex]->getNDF()) < 0.05)// 5% cut
      { }
      else {
        vertexTrackCandidates.push_back(&(GFTracks[GFTrackIndex]->getCand()));
      }
    }

    if (vertexTrackCandidates.empty()) {
      B2WARNING(" ** VF:  : Vertex has no track candidates associated.");
      return;
    }

    const MCParticle* const creatingParticle = particles[vertexTrackCandidates[0]->getMcTrackId()];

    if (!creatingParticle->hasValidVertex()) {
      B2WARNING(" ** VF:  : Creating particle does not have valid vertex.");
      return;
    }

    const TVector3 trueVertex = creatingParticle->getProductionVertex();

    for (unsigned int i = 1; i < vertexTrackCandidates.size(); ++i) {

      const MCParticle* const otherParticle = particles[vertexTrackCandidates[i]->getMcTrackId()];
      if (!otherParticle->hasValidVertex()) {
        B2WARNING(" ** VF:  : Creating particle does not have valid vertex.");
        return;
      }

      if (otherParticle->getProductionVertex() != trueVertex) {
        B2WARNING(" ** VF:  : Creating particle does not have valid vertex.");
        return;
      }
    }
    const TVector3 calculatedVertex(vertices[vertexID].position().x(), vertices[vertexID].position().y(), vertices[vertexID].position().z());
    const TVector3 displacement = calculatedVertex - trueVertex;

    B2INFO(" ** VF:  : true Vertex is at x=" << trueVertex.X() << ", y=" << trueVertex.Y() << ", z=" << trueVertex.Z());
    B2INFO(" ** VF:  : calc Vertex is at x=" << calculatedVertex.X() << ", y=" << calculatedVertex.Y() << ", z=" << calculatedVertex.Z());
    B2INFO(" ** VF:  : displacement is   x=" << displacement.X() << ", y=" << displacement.Y() << ", z=" << displacement.Z());
    B2INFO(" ** VF:  : displacement magnitude is " << displacement.Mag());
    B2INFO(" ** VF:  : standardised residual x: " << displacement.X() / sqrt(vertices[vertexID].error().dxx()) <<
           ", y: " << displacement.Y() / sqrt(vertices[vertexID].error().dyy()) <<
           ", z: " << displacement.Z() / sqrt(vertices[vertexID].error().dzz()));
    B2INFO(" ** VF:  : Var x: " << vertices[vertexID].error().dxx());
    B2INFO(" ** VF:  : standard deviation: " << sqrt(vertices[vertexID].error().dxx()));
    B2INFO(" ** VF:  : chi2=" << vertices[vertexID].chiSquared());

    vertexDisplacementHistogram->Fill(displacement.Mag());
    vertexDisplacementXHistogram->Fill(displacement.X());
    vertexDisplacementYHistogram->Fill(displacement.Y());
    vertexDisplacementZHistogram->Fill(displacement.Z());
    standardisedResidualXHistogram->Fill(displacement.X() / sqrt(vertices[vertexID].error().dxx()));
    standardisedResidualYHistogram->Fill(displacement.Y() / sqrt(vertices[vertexID].error().dyy()));
    standardisedResidualZHistogram->Fill(displacement.Z() / sqrt(vertices[vertexID].error().dzz()));
    trueVertexXHistogram->Fill(trueVertex.X());
    trueVertexYHistogram->Fill(trueVertex.Y());
    trueVertexZHistogram->Fill(trueVertex.Z());
    chiSquaredHistogram->Fill(vertices[vertexID].chiSquared());
    probHistogram->Fill(TMath::Prob(vertices[vertexID].chiSquared(), vertices[vertexID].ndf()));
  }
}
void VertexFitter2Module::endRun()
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
    B2ERROR(" ** VF:  : failed to copy steering file.");

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
void VertexFitter2Module::terminate()
{
}
