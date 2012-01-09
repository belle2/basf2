#include <tracking/modules/vertexfitter/VertexFitterModule.h>

#include <tracking/dataobjects/VertexFitterVertexContainer.h>

#include <framework/datastore/StoreArray.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <rave/Vertex.h>
#include <rave/VertexFactory.h>
#include <GFTrack.h>
#include <RKTrackRep.h>
#include <TMath.h>
#include <TFile.h>
#include <GFException.h>

using namespace Belle2;

REG_MODULE(VertexFitter)


class RaveDebug {
public:
  static void print(const rave::Vector6D& vec, const std::string& desc = "") {
    B2INFO("rave: " << desc << ": x=" << vec.x() << ", y=" << vec.y() << ", z=" << vec.z() << ", px=" << vec.px() << ", py=" << vec.py() << ", pz=" << vec.pz());
  }

  static void print(const rave::Covariance6D& cov, const std::string& desc = "") {
    B2INFO("rave: " << desc << ": dxx=" << cov.dxx() << ", dxy=" << cov.dxy() << ", dxz=" << cov.dxz() << ", dyy=" << cov.dyy() << ", dyz=" << cov.dyz() << ", dzz=" << cov.dzz() <<
           ", dxpx=" << cov.dxpx() << ", dxpy=" << cov.dxpy() << ", dxpz=" << cov.dxpz() << ", dypx=" << cov.dypx() << ", dypy=" << cov.dypy() << ", dypz=" << cov.dypz() <<
           ", dzpx=" << cov.dzpx() << ", dzpy=" << cov.dzpy() << ", dzpz=" << cov.dzpz() << ", dpxpx=" << cov.dpxpx() << ", dpxpy=" << cov.dpxpy() <<
           ", dpxpz=" << cov.dpxpz() << ", dpypy=" << cov.dpypy() << ", dpypz=" << cov.dpypz() << ", dpzpz=" << cov.dpzpz());
  }
};

VertexFitterModule::VertexFitterModule() : Module()
{
  setDescription("Integration of VertexFitter Rave (Reconstruction in an Abstract Vertices Environment).");

  addParam("GFTracksColName", m_GFTracksColName, "Name of collection holding the GFTrack instances.", std::string(""));
  addParam("RAVEVerticesColName", m_RAVEVerticesColName, "Name of collection holding the rave::Vertex instances.", std::string(""));
}

void VertexFitterModule::initialize()
{
}

void VertexFitterModule::beginRun()
{
  B2INFO("rave: beginRun");
}

void VertexFitterModule::event()
{
  B2INFO("rave: Event");
  StoreArray<GFTrack> GFTracks(m_GFTracksColName);

  if (GFTracks.getEntries() == 0) {
    B2WARNING("rave: no tracks found.");
    return;
  }

  B2INFO("rave: processing " << GFTracks.getEntries() << " track(s).");

  B2WARNING("rave: is unit conversion from genfit to rave necessary?");

  std::vector<rave::Track> raveTracks;
  raveTracks.reserve(GFTracks.getEntries());

  const TVector3 origin(0, 0, 0);

  for (int i = 0; i < GFTracks.getEntries(); ++i) {
    if (dynamic_cast<RKTrackRep*>(GFTracks[i]->getCardinalRep()) == NULL) {
      B2ERROR("rave: Track representation is not of the expected type.");
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
    RaveDebug::print(positionAndMomentum, "track parameters");
    RaveDebug::print(covariance6D, "covariance matrix");
    raveTracks.push_back(rave::Track(positionAndMomentum, covariance6D, track->getCharge(), track->getChiSqu(), track->getNDF(), GFTracks[i]));
  }

  class Belle2MagneticField : public rave::MagneticField {
  public:
    virtual ~Belle2MagneticField() {
    }

    virtual rave::MagneticField* copy() const {
      return new Belle2MagneticField;
    }

    virtual rave::Vector3D inTesla(const rave::Point3D& point) const {
      const TVector3 ret = BFieldMap::Instance().getBField(TVector3(point.x(), point.y(), point.z()));
      return rave::Vector3D(ret.X(), ret.Y(), ret.Z());
    }
  };

  if (raveTracks.size() < 2) {
    B2WARNING("rave: less than two usable tracks found.");
    return;
  }

  const Belle2MagneticField magneticField;
  const rave::VertexFactory vertexFactory(magneticField);
  const std::vector<rave::Vertex> vertices = vertexFactory.create(raveTracks);

  const rave::Vector3D b = Belle2MagneticField().inTesla(rave::Point3D(1, 2, 3));
  B2INFO("rave: B is x=" << b.x() << ", y=" << b.y() << ", z=" << b.z());
  B2INFO("rave: found " << vertices.size() << " vertices.");

  StoreArray<VertexFitterVertexContainer> storedVertices(m_RAVEVerticesColName);
  for (unsigned int i = 0; i < vertices.size(); i++) {
    new(storedVertices->AddrAt(i)) VertexFitterVertexContainer(vertices[i]);
  }

}
void VertexFitterModule::endRun()
{
}
void VertexFitterModule::terminate()
{
}