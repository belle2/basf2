/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <tracking/modules/CATFinder/CATFinderModule.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/modules/CATFinder/HitOrderer.h>

#include <mva/interface/Weightfile.h>
#include <mva/interface/Interface.h>
#include <mva/interface/Expert.h>

#include <TMatrixDSym.h>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>

using namespace Belle2;

REG_MODULE(CATFinder);

CATFinderModule::CATFinderModule() : Module()
{
  setDescription("GNN based CDC track finding.");
}

void CATFinderModule::initialize()
{
  m_CDCHits.isRequired();
  m_wireHitVector.isRequired();
  m_recoHitInformations.registerInDataStore();
  m_CDCRecoTracks.registerInDataStore();
  m_CDCHits.registerRelationTo(m_CDCRecoTracks);
  m_recoHitInformations.registerRelationTo(m_CDCHits);
  m_CDCRecoTracks.registerRelationTo(m_recoHitInformations);
  m_CDCGeometryPar = &CDC::CDCGeometryPar::Instance();
}

void CATFinderModule::beginRun()
{
  // Initialize CATFinder weightfile
  if (m_weightfileRepresentation.isValid()) {
    std::stringstream ss(m_weightfileRepresentation -> m_data);
    auto weightFile = MVA::Weightfile::loadFromStream(ss);
    initializeMVA(weightFile);
  }
}

void CATFinderModule::event()
{
  preprocess();
  runGNN();
  postprocess();
}

void CATFinderModule::preprocess()
{
  const std::vector<CDCWireHit>& wireHitVector = *m_wireHitVector;

  m_dataset->m_input.clear();
  m_dataset->m_input.reserve(wireHitVector.size() * N_INPUT_FEATURES);

  for (const auto& wireHit : wireHitVector) {

    if (wireHit.getAutomatonCell().hasMaskedFlag())
      continue;

    const CDCHit cdcHit = *wireHit.getHit();

    const unsigned short clayer = cdcHit.getICLayer();
    const unsigned short wire = cdcHit.getIWire();
    const auto wirePos = m_CDCGeometryPar->c_Aligned;

    const double tdc_scaled = (static_cast<double>(cdcHit.getTDCCount()) - TDC_OFFSET) / TDC_SCALE;
    const double adc_clipped = cdcHit.getADCCount() > ADC_CLIP ? 1. : static_cast<double>(cdcHit.getADCCount()) / ADC_CLIP;
    //unsigned short tot = cdcHit.getTOT();

    const B2Vector3D posForward = m_CDCGeometryPar->wireForwardPosition(clayer, wire, wirePos);
    const B2Vector3D posBackward = m_CDCGeometryPar->wireBackwardPosition(clayer, wire, wirePos);

    const double x = (posForward.x() + posBackward.x()) / 200.;
    const double y = (posForward.y() + posBackward.y()) / 200.;

    //auto wireHitX = wireHit.getRefPos2D().x();
    //auto wireHitY = wireHit.getRefPos2D().y();

    const double superlayer_scaled = static_cast<double>(cdcHit.getISuperLayer()) / SLAYER_SCALE;
    const double clayer_scaled = static_cast<double>(clayer) / CLAYER_SCALE;
    const double layer_scaled = static_cast<double>(cdcHit.getILayer()) / LAYER_SCALE;
    //double chargeDeposit = wireHit.getRefChargeDeposit();
    //double driftTime = wireHit.getDriftTime();
    //double driftLength = wireHit.getRefDriftLength();

    m_dataset->m_input.push_back(x);
    m_dataset->m_input.push_back(y);
    m_dataset->m_input.push_back(tdc_scaled);
    m_dataset->m_input.push_back(adc_clipped);
    m_dataset->m_input.push_back(superlayer_scaled);
    m_dataset->m_input.push_back(clayer_scaled);
    m_dataset->m_input.push_back(layer_scaled);
  }

  // Final step: let's prepare the vectors for the GNN output and the post-processing.
  prepareVectors();
}

void CATFinderModule::runGNN()
{
  // Running the GNN
  const unsigned int outputSize = (m_dataset->m_input.size() / N_INPUT_FEATURES) * N_OUTPUT_FEATURES;
  auto output = m_expert->applyArbitrarySize(*m_dataset, outputSize);

  // Extracting the GNN outputs
  for (size_t i = 0; i + N_OUTPUT_FEATURES <= output.size(); i += N_OUTPUT_FEATURES) {
    // For each CDCHit, the returned output is the following one:
    // - 1 double for the beta value
    // - X doubles for the coordinates of the condensation point in the latent space (e.g.: if the latent space has 3 dimensions, X=3)
    // - 3 doubles for the coordinates of the predicted momentum
    // - 3 doubles for the coordinates of the predicted starting point
    // - 1 double for the predicted charge
    // We have: 1 + X + 3 + 3 + 1 == N_OUTPUT_FEATURES
    m_predBetas.push_back(output[i]);
    m_coords.push_back(std::vector<double>(output.begin() + i + 1, output.begin() + i + 1 + LATENT_SPACE_N_DIM));
    m_predPs.push_back(std::vector<double>(output.begin() + i + 1 + LATENT_SPACE_N_DIM, output.begin() + i + 4 + LATENT_SPACE_N_DIM));
    m_predVs.push_back(std::vector<double>(output.begin() + i + 4 + LATENT_SPACE_N_DIM, output.begin() + i + 7 + LATENT_SPACE_N_DIM));
    m_predQs.push_back(output[i + N_OUTPUT_FEATURES - 1]);
  }
}

void CATFinderModule::postprocess()
{
  // Creating the beta mask
  std::vector<int> betaIndices(m_predBetas.size());
  std::vector<int> selectedBetas(m_predBetas.size());

  std::iota(betaIndices.begin(), betaIndices.end(), 0);
  std::sort(betaIndices.begin(), betaIndices.end(),
  [&](int i1, int i2) {
    return m_predBetas[i1] > m_predBetas[i2];
  });

  for (size_t i = 0; i < m_predBetas.size(); ++i) {
    selectedBetas[i] = static_cast<int>(m_predBetas[i] > T_BETA);
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


    //Calculate CDCHits and GNN nodes assigned to the condensation point

    std::vector<int> CDCHitIndices;
    CDCHitIndices.reserve(r.size());
    std::vector<std::vector<double>> gnnNodes;
    gnnNodes.reserve(r.size());

    for (size_t i = 0; i < r.size(); ++i) {
      if (r[i] < HIT_DISTANCE) {
        CDCHitIndices.push_back(i);
        gnnNodes.push_back({m_dataset->m_input[7 * i] * 100, m_dataset->m_input[7 * i + 1] * 100});
      }
    }

    // Cut on the amount of CDC hits assigned to the condensation point

    if (CDCHitIndices.size() < CDC_HIT_INDICES_CUT) {
      B2INFO("Skipping contensation point due to CDCHit cut.");
      continue;
    }

    // Get the momentum and position of the condensation point

    const ROOT::Math::XYZVector momentum(m_conPointPs[conPoint][0],
                                         m_conPointPs[conPoint][1],
                                         m_conPointPs[conPoint][2]
                                        );

    const ROOT::Math::XYZVector position(m_conPointVs[conPoint][0] * 100.,
                                         m_conPointVs[conPoint][1] * 100.,
                                         m_conPointVs[conPoint][2] * 100.
                                        );

    if (std::isnan(position.X()) || std::isnan(momentum.X())) {
      B2WARNING("Skipping track with NaN values.");
      continue;
    }

    // Get the charge of the condensation point

    auto tCharge = m_conPointQs[conPoint];
    const short charge = (tCharge >= 0.5) ? 1 : -1;

    // Order the hits with KDT

    HitOrderer hitOrderer;
    std::vector<int> sortedIndices = hitOrderer.orderHits({position.X(), position.Y()}, gnnNodes, CDCHitIndices);

    // Create a new RecoTrack and fill it with position, momentum and charge information

    RecoTrack* cdcRecotrack = m_CDCRecoTracks.appendNew();
    cdcRecotrack -> setPositionAndMomentum(position, momentum);
    cdcRecotrack -> setChargeSeed(charge);

    // Create a covatiance matrix and add it to the RecoTrack

    auto seed_cov = TMatrixDSym(6);
    for (int i = 0; i < 6; ++i) {
      for (int k = 0; k < 6; ++k) {
        seed_cov[i][k] = 1e-1;
      }
    }

    cdcRecotrack -> setSeedCovariance(seed_cov);

    // Add the sorted CDCHits to the RecoTrack

    int k = 0;
    for (int cdcHitIndex : sortedIndices) {
      cdcRecotrack -> addCDCHit(m_CDCHits[cdcHitIndex], k);
      ++k;
    }
  }
}

void CATFinderModule::initializeMVA(MVA::Weightfile& weightfile)
{
  auto supportedInterfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions generalOptions;
  weightfile.getOptions(generalOptions);

  m_expert = supportedInterfaces[generalOptions.m_method] -> getExpert();
  m_expert -> load(weightfile);

  std::vector<float> dummy;
  m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(generalOptions, std::move(dummy), 0));
}

void CATFinderModule::collectOverThreshold(const std::vector<int>& betaIndices, const std::vector<std::vector<double>>& coords,
                                           std::vector<int>& selectedBetas)
{
  if (!betaIndices.empty() && betaIndices[0] < coords.size()) {
    m_conPoints.push_back(coords[betaIndices[0]]);
  }

  for (size_t i = 1; i < betaIndices.size(); ++i) {
    int id = betaIndices[i];
    if (selectedBetas[id]) {
      const std::vector<double>& conCandidate = coords[id];
      if (isConPointOutOfRadius(conCandidate, m_conPoints)) {
        m_conPoints.push_back(conCandidate);
      } else {
        selectedBetas[id] = false;
      }
    }
  }
}

bool CATFinderModule::isConPointOutOfRadius(const std::vector<double>& pointCandidate,
                                            const std::vector<std::vector<double>>& points)
{
  for (const std::vector<double>& point : points) {
    double squareSum = 0;
    for (size_t i  = 0; i < point.size(); ++i) {
      double diff = pointCandidate[i] - point[i];
      squareSum += diff * diff;
    }
    double distance = std::sqrt(squareSum);
    if (distance <= T_DISTANCE) {
      return false;
    }
  }
  return true;
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
