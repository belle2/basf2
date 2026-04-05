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

  // Ugly solution, I know
  unsigned int nHits = 0;
  for (const auto& wireHit : wireHitVector) {
    if (wireHit.getAutomatonCell().hasMaskedFlag())
      continue;
    nHits++;
  }

  // Nothing to do: let's return
  if (nHits == 0)
    return;

  auto input = Tensor<float>::make_shared({nHits, 7});
  auto beta = Tensor<float>::make_shared({nHits, 1});
  auto coord = Tensor<float>::make_shared({nHits, 3});
  auto p = Tensor<float>::make_shared({nHits, 3});
  auto vertex = Tensor<float>::make_shared({nHits, 3});
  auto charge = Tensor<float>::make_shared({nHits, 1});

  std::vector<unsigned int> tensorIndexToHitIndex;
  tensorIndexToHitIndex.reserve(nHits);

  unsigned int iHit = 0;

  for (const auto& wireHit : wireHitVector) {
    if (wireHit.getAutomatonCell().hasMaskedFlag())
      continue;

    const CDCHit* cdcHit = wireHit.getHit();

    const unsigned short clayer = cdcHit->getICLayer();
    const unsigned short wire = cdcHit->getIWire();
    const auto wirePos = m_CDCGeometryPar->c_Aligned;

    const double tdc_scaled = (static_cast<double>(cdcHit->getTDCCount()) - TDC_OFFSET) / TDC_SCALE;
    const double adc_clipped = cdcHit->getADCCount() > ADC_CLIP ? 1. : static_cast<double>(cdcHit->getADCCount()) / ADC_CLIP;

    const B2Vector3D posForward = m_CDCGeometryPar->wireForwardPosition(clayer, wire, wirePos);
    const B2Vector3D posBackward = m_CDCGeometryPar->wireBackwardPosition(clayer, wire, wirePos);

    const double x = 0.5 * (posForward.x() + posBackward.x()) / SPATIAL_COORDINATES_SCALE;
    const double y = 0.5 * (posForward.y() + posBackward.y()) / SPATIAL_COORDINATES_SCALE;

    const double superlayer_scaled = static_cast<double>(cdcHit->getISuperLayer()) / SLAYER_SCALE;
    const double clayer_scaled = static_cast<double>(clayer) / CLAYER_SCALE;
    const double layer_scaled = static_cast<double>(cdcHit->getILayer()) / LAYER_SCALE;

    input->at({iHit, 0}) = x;
    input->at({iHit, 1}) = y;
    input->at({iHit, 2}) = tdc_scaled;
    input->at({iHit, 3}) = adc_clipped;
    input->at({iHit, 4}) = superlayer_scaled;
    input->at({iHit, 5}) = clayer_scaled;
    input->at({iHit, 6}) = layer_scaled;

    tensorIndexToHitIndex.push_back(iHit);

    ++iHit;
  }

  // Final step: let's prepare the vectors for the GNN output and the post-processing.
  prepareVectors();

  m_session.run(
  {{"input", input}}, {
    {"beta", beta},
    {"ccoords", coord},
    {"p", p},
    {"vertex", vertex},
    {"charge", charge}
  }
  );

  // Extracting the GNN outputs
  for (size_t i = 0; i < iHit; ++i) {
    m_predBetas.push_back(beta->at({i, 0}));
    m_coords.push_back({coord->at({i, 0}), coord->at({i, 1}), coord->at({i, 2})});
    m_predPs.push_back({p->at({i, 0}), p->at({i, 1}), p->at({i, 2})});
    m_predVs.push_back({vertex->at({i, 0}), vertex->at({i, 1}), vertex->at({i, 2})});
    m_predQs.push_back(charge->at({i, 0}));
  }

  // Creating the beta mask
  std::vector<unsigned int> betaIndices(m_predBetas.size());
  std::vector<uint8_t> selectedBetas(m_predBetas.size());

  std::iota(betaIndices.begin(), betaIndices.end(), 0);
  std::sort(betaIndices.begin(), betaIndices.end(),
  [&](int i1, int i2) {
    return m_predBetas[i1] > m_predBetas[i2];
  });

  for (size_t i = 0; i < m_predBetas.size(); ++i) {
    selectedBetas[i] = static_cast<uint8_t>(m_predBetas[i] > T_BETA);
  }

  collectOverThreshold(betaIndices, m_coords, selectedBetas);


  // Apply beta mask to GNN outputs
  const size_t betasCount = std::count(selectedBetas.begin(), selectedBetas.end(), 1);
  m_conPoints.resize(betasCount);
  m_conPointPs.resize(betasCount);
  m_conPointVs.resize(betasCount);
  m_conPointQs.resize(betasCount);

  size_t j = 0;
  for (size_t i = 0; i < selectedBetas.size(); ++i) {
    if (selectedBetas[i]) {
      m_conPoints[j] = m_coords[i];
      m_conPointPs[j] = m_predPs[i];
      m_conPointVs[j] = m_predVs[i];
      m_conPointQs[j] = m_predQs[i];
      ++j;
    }
  }

  // Loop over the condensation points
  for (size_t conPoint = 0; conPoint < m_conPoints.size(); ++conPoint) {

    // Calculate distances of the condensation point to all other points
    const size_t coordSize = m_coords.size();
    std::vector<double> r(coordSize);

    for (size_t i = 0; i < coordSize; ++i) {
      r[i] = std::hypot(m_conPoints[conPoint][0] - m_coords[i][0],
                        m_conPoints[conPoint][1] - m_coords[i][1],
                        m_conPoints[conPoint][2] - m_coords[i][2]);
    }

    // Calculate CDCHits and GNN nodes assigned to the condensation point
    std::vector<int> indices;
    indices.reserve(r.size());
    std::vector<std::vector<double>> gnnNodes;
    gnnNodes.reserve(r.size());

    for (size_t i = 0; i < r.size(); ++i) {
      if (r[i] < HIT_DISTANCE) {
        indices.push_back(i);
        gnnNodes.push_back({input->at({i, 0}), input->at({i, 1})}); // x, y
      }
    }

    // Cut on the amount of CDC hits assigned to the condensation point
    if (indices.size() < CDC_HIT_INDICES_CUT) {
      continue;
    }

    // Get the momentum and position of the condensation point
    const ROOT::Math::XYZVector momentum(m_conPointPs[conPoint][0],
                                         m_conPointPs[conPoint][1],
                                         m_conPointPs[conPoint][2]
                                        );

    const ROOT::Math::XYZVector position(m_conPointVs[conPoint][0] * SPATIAL_COORDINATES_SCALE,
                                         m_conPointVs[conPoint][1] * SPATIAL_COORDINATES_SCALE,
                                         m_conPointVs[conPoint][2] * SPATIAL_COORDINATES_SCALE
                                        );

    if (std::isnan(position.X()) || std::isnan(momentum.X())) {
      B2WARNING("Skipping track with NaN values.");
      continue;
    }

    // Get the charge of the condensation point
    auto tCharge = m_conPointQs[conPoint];
    const short ccharge = (tCharge >= 0.5) ? 1 : -1;

    // Order the hits with KDT
    CATFinderUtils::HitOrderer hitOrderer;
    auto [startingX, startingY] = projectToCDCWall(position, momentum, 16);
    std::vector<int> sortedIndices = hitOrderer.orderHits(std::vector<double>(startingX, startingY), gnnNodes, indices);

    // Create a new RecoTrack and fill it with position, momentum and charge information
    RecoTrack* cdcRecotrack = m_CDCRecoTracks.appendNew();
    cdcRecotrack->setPositionAndMomentum(position, momentum);
    cdcRecotrack->setChargeSeed(ccharge);

    // Create a covatiance matrix and add it to the RecoTrack
    auto seed_cov = TMatrixDSym(6);
    for (int i = 0; i < 6; ++i) {
      for (int k = 0; k < 6; ++k) {
        seed_cov[i][k] = 1e-1;
      }
    }
    cdcRecotrack->setSeedCovariance(seed_cov);

    // Add the sorted CDCHits to the RecoTrack
    int k = 0;
    for (int tensorIndex : sortedIndices) {
      unsigned int hitIndex = tensorIndexToHitIndex[tensorIndex];
      cdcRecotrack->addCDCHit(wireHitVector[hitIndex].getHit(), k);
      ++k;
    }
  }
}

void CATFinderModule::collectOverThreshold(const std::vector<unsigned int>& betaIndices,
                                           const std::vector<std::vector<double>>& coords,
                                           std::vector<uint8_t>& selectedBetas)
{
  if (!betaIndices.empty() && betaIndices[0] < coords.size()) {
    m_conPoints.push_back(coords[betaIndices[0]]);
  }

  for (size_t i = 1; i < betaIndices.size(); ++i) {
    int id = betaIndices[i];
    if (selectedBetas[id] > 0) {
      const std::vector<double>& conCandidate = coords[id];
      if (isConPointOutOfRadius(conCandidate, m_conPoints)) {
        m_conPoints.push_back(conCandidate);
      } else {
        selectedBetas[id] = 0;
      }
    }
  }
}

bool CATFinderModule::isConPointOutOfRadius(const std::vector<double>& pointCandidate,
                                            const std::vector<std::vector<double>>& points)
{
  constexpr double thresholdSquared = T_DISTANCE * T_DISTANCE;
  for (const auto& point : points) {
    double distanceSquared = 0.0;
    for (size_t i = 0; i < point.size(); ++i) {
      const double diff = pointCandidate[i] - point[i];
      distanceSquared += diff * diff;
    }
    if (distanceSquared <= thresholdSquared) {
      return false;
    }
  }
  return true;
}

std::pair<double, double> projectToCDCWall(const ROOT::Math::XYZVector& pos,
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

void CATFinderModule::prepareVectors()
{
  // Clearing vectors from previous event
  m_predBetas.clear();
  m_predQs.clear();
  m_conPointQs.clear();
  m_predPs.clear();
  m_predVs.clear();
  m_coords.clear();
  m_conPointPs.clear();
  m_conPointVs.clear();
  m_betaIndices.clear();
  m_selectedBetas.clear();
  m_conPoints.clear();
}
