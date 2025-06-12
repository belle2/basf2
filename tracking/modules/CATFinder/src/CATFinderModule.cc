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

  CDCGeometryPar = &CDC::CDCGeometryPar::Instance();

  evtNo = 0;
}

void CATFinderModule::beginRun()
{
  // Initialize CATFinder weightfile

  if (m_weightfile_representation.isValid()) {
    std::stringstream ss(m_weightfile_representation -> m_data);
    auto weightFile = MVA::Weightfile::loadFromStream(ss);
    initializeMVA(weightFile);
  }

}

void CATFinderModule::event()
{
  ++evtNo;
  preprocess();
  runGNN();
  postprocess();
}

void CATFinderModule::terminate()
{
  return;
}

void CATFinderModule::preprocess()
{
  const std::vector<CDCWireHit>& wireHitVector = *m_wireHitVector;
  std::vector<int> cdcHitIndexVector;
  int wireHitAmount = wireHitVector.size();
  prepareVectors();

  m_dataset->m_input.clear();
  m_dataset->m_input.reserve(wireHitAmount * N_INPUT_FEATURES);

  for (int hitID = 0; hitID < wireHitAmount; ++hitID) {
    const auto& wireHit = wireHitVector[hitID];

    if (wireHit.getAutomatonCell().hasMaskedFlag()) {
      continue;
    }

    const CDCHit cdcHit = *wireHit.getHit();

    unsigned short m_clayer = cdcHit.getICLayer();
    unsigned short m_wire = cdcHit.getIWire();
    auto m_wirePos = CDCGeometryPar -> c_Aligned;

    double m_tdc = (static_cast<double>(cdcHit.getTDCCount()) - TDC_OFFSET) / TDC_SCALE;
    double m_adc = cdcHit.getADCCount() > ADC_CLIP ? 1. : static_cast<double>(cdcHit.getADCCount()) / ADC_CLIP;
    //unsigned short tot = cdcHit.getTOT();

    B2Vector3D posForward = CDCGeometryPar -> wireForwardPosition(m_clayer, m_wire, m_wirePos);
    B2Vector3D posBackward = CDCGeometryPar -> wireBackwardPosition(m_clayer, m_wire, m_wirePos);

    double x = (posForward.x() + posBackward.x()) / 200.;
    double y = (posForward.y() + posBackward.y()) / 200.;

    //auto wireHitX = wireHit.getRefPos2D().x();
    //auto wireHitY = wireHit.getRefPos2D().y();

    double m_slayer = static_cast<double>(cdcHit.getISuperLayer()) / SLAYER_SCALE;
    double m_clayer_scaled = static_cast<double>(cdcHit.getICLayer()) / CLAYER_SCALE;
    double m_layer = static_cast<double>(cdcHit.getILayer()) / LAYER_SCALE;
    //double chargeDeposit = wireHit.getRefChargeDeposit();
    //double driftTime = wireHit.getDriftTime();
    //double driftLength = wireHit.getRefDriftLength();

    m_dataset->m_input.push_back(x);
    m_dataset->m_input.push_back(y);
    m_dataset->m_input.push_back(m_tdc);
    m_dataset->m_input.push_back(m_adc);
    m_dataset->m_input.push_back(m_slayer);
    m_dataset->m_input.push_back(m_clayer_scaled);
    m_dataset->m_input.push_back(m_layer);

  }
}

void CATFinderModule::runGNN()
{
  // Running and timing the GNN

  auto output = m_expert->applyArbitrarySize(*m_dataset, m_dataset->m_input.size(),
                                             (m_dataset->m_input.size() / N_INPUT_FEATURES) * 11);

  // Extracting GNN outputs

  for (size_t i = 0; i + 8 + LATENT_SPACE_N_DIM <= output.size(); i += N_OUTPUT_FEATURES) {
    predBetas.push_back(output[i]);
    coords.push_back(std::vector<double>(output.begin() + i + 1, output.begin() + i + 1 + LATENT_SPACE_N_DIM));
    predPs.push_back(std::vector<double>(output.begin() + i + 1 + LATENT_SPACE_N_DIM, output.begin() + i + 4 + LATENT_SPACE_N_DIM));
    predVs.push_back(std::vector<double>(output.begin() + i + 4 + LATENT_SPACE_N_DIM, output.begin() + i + 7 + LATENT_SPACE_N_DIM));
    predQs.push_back(output[i + N_OUTPUT_FEATURES - 1]);
  }
}

void CATFinderModule::postprocess()
{

  // Creating the beta mask

  std::vector<int> betaIndices(predBetas.size());
  std::vector<int> selectedBetas(predBetas.size());

  std::iota(betaIndices.begin(), betaIndices.end(), 0);
  std::sort(betaIndices.begin(), betaIndices.end(),
  [&](int i1, int i2) {
    return predBetas[i1] > predBetas[i2];
  });

  for (size_t i = 0; i < predBetas.size(); ++i) {
    selectedBetas[i] = static_cast<int>(predBetas[i] > T_BETA);
  }

  collectOverThreshold(betaIndices, coords, selectedBetas);


  // Apply beta mask to GNN outputs

  size_t betasCount = std::count(selectedBetas.begin(), selectedBetas.end(), 1);
  conPoints.resize(betasCount);
  conPointPs.resize(betasCount);
  conPointVs.resize(betasCount);
  conPointQs.resize(betasCount);

  size_t j = 0;
  for (size_t i = 0; i < selectedBetas.size(); ++i) {
    if (selectedBetas[i]) {
      conPoints[j] = coords[i];
      conPointPs[j] = predPs[i];
      conPointVs[j] = predVs[i];
      conPointQs[j] = predQs[i];
      ++j;
    }
  }

  // Loop over the condensation points

  for (size_t conPoint = 0; conPoint < conPoints.size(); ++conPoint) {

    // Calculate distances of the condensation point to all other points

    const size_t coordSize = coords.size();
    std::vector<double> r(coordSize);

    for (size_t i = 0; i < coordSize; ++i) {
      r[i] = std::hypot(conPoints[conPoint][0] - coords[i][0],
                        conPoints[conPoint][1] - coords[i][1],
                        conPoints[conPoint][2] - coords[i][2]);
    }


    //Calculate CDCHits and GNN nodes assigned to the condensation point

    std::vector<int> CDCHitIndices;
    std::vector<std::vector<double>> gnnNodes;

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

    std::vector<double> momentum = {conPointPs[conPoint][0],
                                    conPointPs[conPoint][1],
                                    conPointPs[conPoint][2]
                                   };

    std::vector<double> position = {conPointVs[conPoint][0] * 100.,
                                    conPointVs[conPoint][1] * 100.,
                                    conPointVs[conPoint][2] * 100.
                                   };

    if (std::isnan(position[0]) || std::isnan(momentum[0])) {
      B2WARNING("Skipping track with NaN values.");
      continue;
    }


    // Get the charge of the condensation point

    auto tCharge = conPointQs[conPoint];
    short charge;

    tCharge >= 0.5 ? charge = 1 : charge = -1;

    // Order the hits with KDT

    HitOrderer hitOrderer;
    std::vector<int> sortedIndices = hitOrderer.orderHits({0., 0.}, gnnNodes, CDCHitIndices);

    // Create a new RecoTrack and fill it with position, momentum and charge information

    RecoTrack* cdcRecotrack = m_CDCRecoTracks.appendNew();
    cdcRecotrack -> setPositionAndMomentum(
      ROOT::Math::XYZVector(position[0], position[1], position[2]),
      ROOT::Math::XYZVector(momentum[0], momentum[1], momentum[2])
    );

    cdcRecotrack -> setChargeSeed(charge);


    // Create a covatiance matrix and add it to the RecoTrack

    auto seed_cov = TMatrixDSym(6);
    for (int i = 0; i < 6; ++i) {
      for (int j = 0; j < 6; ++j) {
        seed_cov[i][j] = 1e-1;
      }
    }

    cdcRecotrack -> setSeedCovariance(seed_cov);


    // Add the sorted CDCHits to the RecoTrack

    int j = 0;
    for (int cdcHitIndex : sortedIndices) {
      cdcRecotrack -> addCDCHit(m_CDCHits[cdcHitIndex], j);
      ++j;
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

void CATFinderModule::collectOverThreshold(std::vector<int> betaIndices, std::vector<std::vector<double>> coords,
                                           std::vector<int>& selectedBetas)
{
  if (!betaIndices.empty() && betaIndices[0] < coords.size()) {
    conPoints.push_back(coords[betaIndices[0]]);
  }

  for (size_t i = 1; i < betaIndices.size(); ++i) {
    int id = betaIndices[i];
    if (selectedBetas[id]) {
      const std::vector<double>& conCandidate = coords[id];
      if (isConPointOutOfRadius(conCandidate, conPoints)) {
        conPoints.push_back(conCandidate);
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
  predBetas.clear();
  predQs.clear();
  conPointQs.clear();
  predPs.clear();
  predVs.clear();
  coords.clear();
  conPointPs.clear();
  conPointVs.clear();
  betaIndices.clear();
  selectedBetas.clear();
  conPoints.clear();
}