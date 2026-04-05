/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/CATFinder/CATFinderModule.h>

#include <framework/logging/Logger.h>
#include <tracking/modules/CATFinder/CATFinderUtils.h>
#include <tracking/trackingUtilities/geometry/Vector2D.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

#include <TMatrixDSym.h>

using namespace Belle2;
using Belle2::MVA::ONNX::Tensor;

REG_MODULE(CATFinder);

CATFinderModule::CATFinderModule() : Module(), m_session("/work/gdepietro/cat-finder-cpp/cdcnet.onnx")
{
  setDescription("GNN based CDC track finding.");
  addParam("recoTracksStoreArrayName", m_CDCRecoTracksName, "StoreArray name of the output CDC reco tracks.",
           m_CDCRecoTracksName);
}

void CATFinderModule::initialize()
{
  m_CDCHits.isRequired();
  m_wireHitVector.isRequired();
  m_recoHitInformations.registerInDataStore();
  m_CDCRecoTracks.registerInDataStore(m_CDCRecoTracksName);
  m_CDCHits.registerRelationTo(m_CDCRecoTracks);
  m_recoHitInformations.registerRelationTo(m_CDCHits);
  m_CDCRecoTracks.registerRelationTo(m_recoHitInformations);
  m_CDCGeometryPar = &CDC::CDCGeometryPar::Instance();
}

void CATFinderModule::beginRun()
{
}

void CATFinderModule::event()
{
  const std::vector<TrackingUtilities::CDCWireHit>& wireHitVector = *m_wireHitVector;

  // Use only "unmasked" hits
  unsigned int nHits = 0;
  for (const auto& wireHit : wireHitVector) {
    if (!wireHit.getAutomatonCell().hasMaskedFlag())
      nHits++;
  }

  // Nothing to do if all the hits are already masked
  if (nHits == 0)
    return;

  // Input tensore: features per hit
  auto input_t    = Tensor<float>::make_shared({nHits, 7});
  // Output tensor: condensation score
  auto beta_t     = Tensor<float>::make_shared({nHits, 1});
  // Output tensor: predicted coordinates in the latent space
  auto coord_t    = Tensor<float>::make_shared({nHits, 3});
  // Output tensor: predicted momentum
  auto momentum_t = Tensor<float>::make_shared({nHits, 3});
  // Output tensor: predicted "vertex"
  auto vertex_t   = Tensor<float>::make_shared({nHits, 3});
  // Output tensor: predicted charge
  auto charge_t   = Tensor<float>::make_shared({nHits, 1});

  // Map from tensor row index back to the original wireHitVector index,
  // needed later when adding CDC hits to the RecoTrack
  std::vector<unsigned int> tensorIndexToHitIndex;
  tensorIndexToHitIndex.reserve(nHits);

  unsigned int iHit = 0;
  for (size_t iWireHit = 0; iWireHit < wireHitVector.size(); ++iWireHit) {

    // Again: skip the already masked hits
    if (wireHitVector[iWireHit].getAutomatonCell().hasMaskedFlag())
      continue;

    // Prepare the input features
    const CDCHit* cdcHit = wireHitVector[iWireHit].getHit();
    const unsigned short clayer = cdcHit->getICLayer();
    const unsigned short wire   = cdcHit->getIWire();
    const auto wirePos = m_CDCGeometryPar->c_Aligned;

    const double tdc_scaled  = (static_cast<double>(cdcHit->getTDCCount()) - TDC_OFFSET) / TDC_SCALE;
    const double adc_clipped = cdcHit->getADCCount() > ADC_CLIP
                               ? 1.
                               : static_cast<double>(cdcHit->getADCCount()) / ADC_CLIP;

    const B2Vector3D posForward  = m_CDCGeometryPar->wireForwardPosition(clayer, wire, wirePos);
    const B2Vector3D posBackward = m_CDCGeometryPar->wireBackwardPosition(clayer, wire, wirePos);

    // Prepare the tensor
    input_t->at({iHit, 0}) = 0.5 * (posForward.x() + posBackward.x()) / SPATIAL_COORDINATES_SCALE;
    input_t->at({iHit, 1}) = 0.5 * (posForward.y() + posBackward.y()) / SPATIAL_COORDINATES_SCALE;
    input_t->at({iHit, 2}) = tdc_scaled;
    input_t->at({iHit, 3}) = adc_clipped;
    input_t->at({iHit, 4}) = static_cast<double>(cdcHit->getISuperLayer()) / SLAYER_SCALE;
    input_t->at({iHit, 5}) = static_cast<double>(clayer)                   / CLAYER_SCALE;
    input_t->at({iHit, 6}) = static_cast<double>(cdcHit->getILayer())      / LAYER_SCALE;

    tensorIndexToHitIndex.push_back(iWireHit);
    ++iHit;
  }

  B2DEBUG(29, "CDCWireHits in the event: " << nHits);
  if (nHits != iHit)
    B2ERROR("Different number of hits: something went wrong...");

  // Run the GNN inference
  m_session.run(
  {{"input", input_t}},
  {{"beta", beta_t}, {"ccoords", coord_t}, {"p", momentum_t}, {"vertex", vertex_t}, {"charge", charge_t}}
  );

  // Build an index array sorted by descending beta so we process the most probable condensation points first
  std::vector<unsigned int> betaIndices(nHits);
  std::iota(betaIndices.begin(), betaIndices.end(), 0);
  std::sort(betaIndices.begin(), betaIndices.end(),
  [&](unsigned int i1, unsigned int i2) { return beta_t->at({i1, 0}) > beta_t->at({i2, 0}); });

  // A hit is a candidate condensation point only if its beta exceeds the threshold
  std::vector<uint8_t> selectedBetas(nHits);
  for (size_t i = 0; i < nHits; ++i)
    selectedBetas[i] = static_cast<uint8_t>(beta_t->at({i, 0}) > T_BETA);

  // A new condensation point is accepted only if it lies farther than T_DISTANCE
  // from every already-accepted point in latent coordinate space
  std::vector<size_t> conPointIndices;
  constexpr double thresholdSquared = T_DISTANCE * T_DISTANCE;

  // Returns true if hit i is outside the exclusion radius of every accepted condensation point
  auto isOutOfRadius = [&](unsigned int i) {
    for (unsigned int iConPoint : conPointIndices) {
      double d = 0.0;
      d += (coord_t->at({i, 0}) - coord_t->at({iConPoint, 0})) * (coord_t->at({i, 0}) - coord_t->at({iConPoint, 0}));
      d += (coord_t->at({i, 1}) - coord_t->at({iConPoint, 1})) * (coord_t->at({i, 1}) - coord_t->at({iConPoint, 1}));
      d += (coord_t->at({i, 2}) - coord_t->at({iConPoint, 2})) * (coord_t->at({i, 2}) - coord_t->at({iConPoint, 2}));
      if (d <= thresholdSquared)
        return false;
    }
    return true;
  };

  for (unsigned int i = 0; i < betaIndices.size(); ++i) {
    unsigned int iBeta = betaIndices[i];
    if (!selectedBetas[iBeta])
      continue;  // Below the beta threshold: not a condensation point candidate
    if (conPointIndices.empty() or isOutOfRadius(iBeta))
      conPointIndices.push_back(iBeta);  // Accept as a new condensation point
    else
      selectedBetas[iBeta] = 0;  // Too close to an existing seed: let's discard it
  }
  B2DEBUG(29, "Condensation points in the event: " << conPointIndices.size());

  // Convert the condensation points into RecoTracks: one condensation point -> one RecoTrack
  for (size_t iConPoint : conPointIndices) {

    std::vector<CATFinderUtils::KDTHit> kdtHits;
    kdtHits.reserve(nHits);

    // Collect all hits whose clustering coordinates fall within HIT_DISTANCE of this seed
    for (size_t i = 0; i < nHits; ++i) {
      const double dx = coord_t->at({iConPoint, 0}) - coord_t->at({i, 0});
      const double dy = coord_t->at({iConPoint, 1}) - coord_t->at({i, 1});
      const double dz = coord_t->at({iConPoint, 2}) - coord_t->at({i, 2});
      if (std::hypot(dx, dy, dz) < HIT_DISTANCE) {
        // Store the wire X and Y coordinatres together with the tensor row index for later lookup
        kdtHits.push_back({input_t->at({i, 0}), input_t->at({i, 1}), static_cast<int>(i)});
      }
    }

    // Reject tracks with too few hits
    if (kdtHits.size() < CDC_HIT_INDICES_CUT)
      continue;

    // Retrieve predicted momentum, vertex and charge from the condensation point's output
    const ROOT::Math::XYZVector momentum(
      momentum_t->at({iConPoint, 0}), momentum_t->at({iConPoint, 1}), momentum_t->at({iConPoint, 2}));
    const ROOT::Math::XYZVector position(
      vertex_t->at({iConPoint, 0}) * SPATIAL_COORDINATES_SCALE,
      vertex_t->at({iConPoint, 1}) * SPATIAL_COORDINATES_SCALE,
      vertex_t->at({iConPoint, 2}) * SPATIAL_COORDINATES_SCALE);
    const int charge = (charge_t->at({iConPoint, 0}) >= 0.5) ? 1 : -1;

    B2DEBUG(29, LogVar("Condensation point", iConPoint) << LogVar("Attached hits", kdtHits.size())
            << LogVar("Momentum", std::sqrt(momentum.Mag2())) << LogVar("Vertex", std::sqrt(position.Mag2()))
            << LogVar("Charge", charge));

    if (std::isnan(position.X()) or std::isnan(momentum.X())) {
      B2WARNING("Skipping track with NaN values.");
      continue;
    }

    // Order hits along the helix from the innermost CDC wall outward,
    // starting from where the predicted trajectory intersects the inner wall
    CATFinderUtils::HitOrderer hitOrderer;
    auto [startingX, startingY] = projectToCDCWall(position, momentum, 16);
    std::vector<int> sortedIndices =
      hitOrderer.orderHits(startingX, startingY, std::move(kdtHits));

    // Create a new RecoTrack and seed it with the network predictions
    RecoTrack* cdcRecotrack = m_CDCRecoTracks.appendNew();
    cdcRecotrack->setPositionAndMomentum(position, momentum);
    cdcRecotrack->setChargeSeed(charge);

    // Use a loose covariance matrix as the seed uncertainty
    auto seedCovariance = TMatrixDSym(6);
    for (int j = 0; j < 6; ++j)
      for (int k = 0; k < 6; ++k)
        seedCovariance[j][k] = 1e-1;
    cdcRecotrack->setSeedCovariance(seedCovariance);

    // Add CDC hits in the sorted order
    int iRecoTrackHit = 0;
    for (int tensorIndex : sortedIndices) {
      cdcRecotrack->addCDCHit(wireHitVector[tensorIndexToHitIndex[tensorIndex]].getHit(), iRecoTrackHit);
      ++iRecoTrackHit;
    }
  }
}

std::pair<double, double> CATFinderModule::projectToCDCWall(const ROOT::Math::XYZVector& pos,
                                                            const ROOT::Math::XYZVector& mom,
                                                            double targetR = 16.0)
{
  // Check if we are already outside or at the boundary
  double rSq = pos.X() * pos.X() + pos.Y() * pos.Y();
  if (rSq >= targetR * targetR)
    return {pos.X(), pos.Y()};
  // Coefficients for a*t^2 + b*t + c = 0
  // Solving for |(pos + t*mom).xy| = targetR
  double a = mom.X() * mom.X() + mom.Y() * mom.Y();
  double b = 2.0 * (pos.X() * mom.X() + pos.Y() * mom.Y());
  double c = rSq - (targetR * targetR);
  double discriminant = b * b - 4.0 * a * c;
  if (discriminant < 0 or a == 0)
    return {pos.X(), pos.Y()};
  double sqrtD = std::sqrt(discriminant);
  double invA = 1.0 / a;
  double t1 = 0.5 * (-b + sqrtD) * invA;
  double t2 = 0.5 * (-b - sqrtD) * invA;
  // Get the first positive intersection point
  double t = -1.0;
  if (t1 > 0 && t2 > 0) t = std::min(t1, t2);
  else if (t1 > 0)      t = t1;
  else if (t2 > 0)      t = t2;
  if (t > 0)
    return {pos.X() + t * mom.X(), pos.Y() + t * mom.Y()};
  return {pos.X(), pos.Y()};
}
