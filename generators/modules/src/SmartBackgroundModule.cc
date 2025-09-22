/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// TODOs:
// multiple skims??
// write documentation, set payload name in python!

#include <generators/modules/SmartBackgroundModule.h>
#include <generators/dbobjects/SmartBackgroundConfig.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/EventExtraInfo.h>
#include <framework/database/DBAccessorBase.h>
#include <framework/database/DBStoreEntry.h>
#include <framework/database/DBObjPtr.h>
#include <mva/methods/ONNX.h>

#include <string>
#include <vector>
#include <memory>

#include <TRandom.h>
#include <TMath.h>

using namespace Belle2;

REG_MODULE(SmartBackground);

class Module;

SmartBackgroundModule::SmartBackgroundModule() : Module()
{

  setDescription("Event preselector based on the SmartBkg neural network. "
                 "Should be used only for directly skimmed MC productions. "
                 "Must be added to the path after the MC generator module but before simulation. "
                 "Given a specific skim the neural network predicts the probability of the event passing the skim. "
                 "The module returns 1 with this probability or otherwise 0. "
                 "Events are then weighted with their inverse probability "
                 "(weights are saved in the event meta data, by multiplying them to the generator weight). "
                 "Use case is the reduction of simulation time for directly skimmed MC productions.");
  addParam("skimCode", m_skimCode, "Skim LFN code");
  addParam("eventType", m_eventType, "Event type (charged, mixed, uubar, ccbar, ddbar, ssbar, taupair)");
  addParam("payload", m_payload, "Name of payload storing neural network weights in ONNX format",
           std::string("SmartBKGWeights.onnx"));
  addParam("debugMode", m_debugMode,
           "Debug mode execution (in debug mode, always returns 1 and NN predictions are saved to the event extra info "
           "as 'SmartBKG_Prediction')", false);
  addParam("activationOverride", m_activationOverride, "Override parameters (a, b) of the activation function (clipped exponential)",
           false);
  addParam("activationOverrideParams", m_activationOverrideParams,
           "Parameters (a, b) of the activation function (clipped exponential)", std::vector<float>({0.5, 0.0}));

}

void SmartBackgroundModule::initialize()
{

  //Load config from payload
  DBObjPtr<SmartBackgroundConfig> config("SmartBackgroundConfig", true);
  m_pdgMapping = config->getPdgMapping();
  m_skimcodesMapping = config->getSkimcodesMapping();
  m_paramsMapping = config->getParameterMapping();

  // Check parameters
  if (m_skimcodesMapping.find(m_skimCode) == m_skimcodesMapping.end()) {
    B2FATAL("SmartBkg: Provided skim code " << m_skimCode << " is unknown. Check documentation for allowed skim codes.");
  }
  if (c_eventtypeMapping.find(m_eventType) == c_eventtypeMapping.end()) {
    B2FATAL("SmartBkg: Provided event type " << m_eventType <<
            " is unknown. Allowed event types: charged, mixed, uubar, ddbar, ssbar, ccbar, taupair.");
  }
  if (m_activationOverride) {
    if (m_activationOverrideParams.size() != 2) {
      B2FATAL("SmartBkg: activationOverrideParams must be a vector of size 2, got " << m_activationOverrideParams.size());
    }
    B2DEBUG(20, "SmartBkg: Activation override is enabled, using custom parameters (a, b) = (" << m_activationOverrideParams[0] <<
            ", " << m_activationOverrideParams[1] << ")");
    if (m_activationOverrideParams[0] == 0.5 && m_activationOverrideParams[1] == 0.0) {
      B2WARNING("SmartBkg: Activation override parameters (a, b) are used but set to default values (0.5, 0.0), is this what you want?");
    }
  }

  // Load model from payload
  auto accessor = DBAccessorBase(DBStoreEntry::c_RawFile, m_payload, true);
  const std::string filename = accessor.getFilename();

  // initialize ONNX session
  m_session = std::make_unique<MVA::ONNX::Session>(filename.c_str());

}

float SmartBackgroundModule::activation(float logit, float a, float b)
{
  // Clipped exponential function
  float e = TMath::Exp(a * (logit - b));
  if (e > 1.0) {
    e = 1.0;
  } else if (e < 0.01) {
    e = 0.01;
  }
  return e;
}

void SmartBackgroundModule::event()
{

  // Load store array of MC particles and the event extra info
  StoreArray<MCParticle> mcparticles;
  const int numParticles = mcparticles.getEntries();

  // Clear vectors that hold preprocessed input data
  m_xValues.resize(0);
  m_pdgValues.resize(0);
  m_motherValues.resize(0);

  // New 0-based index of particles after preprocessing removes some particles
  unsigned int newIndex = 0;
  // Some maps needed for preprocessing
  std::unordered_map<int, int> qg_mother_mapping;
  std::unordered_map<int, unsigned int> index_mapping;
  float particle0Properties[4];

  // Loop over particles in event
  for (int particleIndex = 0; particleIndex < numParticles; ++particleIndex) {

    // Get particle and info necessary for preprocessing
    const MCParticle& p = *mcparticles[particleIndex];
    const ROOT::Math::XYZVector vertex = p.getVertex();
    const int pdg = p.getPDG();
    MCParticle* mother = p.getMother();
    int motherIndex = 0;
    if (mother) {
      motherIndex = mother->getArrayIndex();
    }

    // Preprocessing
    if (p.isInitial() || (vertex.X() >= 10) || (vertex.Y() >= 10) || (vertex.Z() >= 10)) {
      // Skip initials and decays far outside
      continue;
    } else if (pdg == 21 || (std::abs(pdg) <= 5)) {
      // Skip quarks and gluons, save index to reconstruct non quark-gluon mothers
      qg_mother_mapping[particleIndex] = motherIndex;
      continue;
    }

    // If particle is kept, save new index
    index_mapping[particleIndex] = newIndex;

    // If first particle, save vertex
    if (newIndex == 0u) {
      particle0Properties[0] = p.getProductionTime();
      particle0Properties[1] = vertex.X();
      particle0Properties[2] = vertex.Y();
      particle0Properties[3] = vertex.Z();
    }

    // Set momentum and vertex 4-vector inputs relative to first particle
    const ROOT::Math::XYZVector momentum = p.getMomentum();
    m_xValues.push_back(p.getProductionTime() - particle0Properties[0]);
    m_xValues.push_back(vertex.X() - particle0Properties[1]);
    m_xValues.push_back(vertex.Y() - particle0Properties[2]);
    m_xValues.push_back(vertex.Z() - particle0Properties[3]);
    m_xValues.push_back(p.getEnergy());
    m_xValues.push_back(momentum.X());
    m_xValues.push_back(momentum.Y());
    m_xValues.push_back(momentum.Z());

    // Set mapped PDG code input
    int pdgMapped = m_pdgMapping[pdg];
    if (!pdgMapped) {
      B2WARNING("SmartBkg: Encountered particle with unknown pdg: " << pdg <<
                ", assigning out-of-distribution value 0 as mapped pdg input.");
      pdgMapped = 0;
    }
    m_pdgValues.push_back(pdgMapped);

    // Set mother index input
    while (motherIndex > 0 && (qg_mother_mapping.find(motherIndex) != qg_mother_mapping.end())) {
      motherIndex = qg_mother_mapping[motherIndex];
    }
    if (index_mapping.find(motherIndex) != index_mapping.end()) {
      m_motherValues.push_back(index_mapping[motherIndex]);
    } else {
      m_motherValues.push_back(-1);
    }

    ++newIndex;
  }

  // Create input ONNX tensors from the input vectors
  int numRemainingParticles = m_pdgValues.size();
  auto xTensor = MVA::ONNX::Tensor<float>::make_shared(m_xValues, {numRemainingParticles, 8});
  auto pdgTensor = MVA::ONNX::Tensor<int32_t>::make_shared(m_pdgValues, {numRemainingParticles});
  auto motherTensor = MVA::ONNX::Tensor<int32_t>::make_shared(m_motherValues, {numRemainingParticles});

  // Set event type input
  auto cTensor = MVA::ONNX::Tensor<int32_t>::make_shared({});
  cTensor->at(0) = c_eventtypeMapping.at(m_eventType);

  // Create output tensor
  auto outputTensor = MVA::ONNX::Tensor<float>::make_shared({static_cast<int64_t>(this->m_skimcodesMapping.size())});

  // Perform inference
  m_session->run({{"x", xTensor}, {"pdg", pdgTensor}, {"mother", motherTensor}, {"c", cTensor}}, {{"output", outputTensor}});

  // Extract prediction for selected skim
  const uint16_t skimIndex = m_skimcodesMapping[m_skimCode];
  float prediction;
  if (m_activationOverride) {
    prediction = this->activation(outputTensor->at({skimIndex}), m_activationOverrideParams[0], m_activationOverrideParams[1]);
  } else {
    std::vector<float> params = m_paramsMapping[m_skimCode];
    prediction = this->activation(outputTensor->at({skimIndex}), params[0], params[1]);
  }

  // Save weight to event meta data / prediction to event extra info
  if (!m_debugMode) {
    StoreObjPtr<EventMetaData> eventMetaData;
    eventMetaData->setGeneratedWeight(eventMetaData->getGeneratedWeight() * (1.0 / prediction));
  } else {
    StoreObjPtr<EventExtraInfo> eventExtraInfo;
    eventExtraInfo->addExtraInfo("SmartBKG_Prediction", prediction);
  }

  // Importance sampling
  if (!m_debugMode) {
    const double randomNum = gRandom->Uniform(0, 1);
    const bool returnValue = randomNum < prediction;
    this->setReturnValue(returnValue);
    if (returnValue) {
      B2DEBUG(20, "SmartBkg prediction is " << prediction << "; event is kept and weight is set to " << 1 / prediction);
    } else {
      B2DEBUG(20, "SmartBkg prediction is " << prediction << "; event is discarded");
    }
  } else {
    this->setReturnValue(true);
  }

}