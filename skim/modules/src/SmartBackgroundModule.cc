/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// TODOs:
// multiple skims??
// write documentation, tutorial for training new model/fine tuning?

#include <skim/modules/SmartBackgroundModule.h>
#include <skim/dbobjects/SmartBackgroundConfig.h>

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
                 "Given one or multiple skims, the model predicts the probability of each event passing each of the skims. "
                 "Events are then sampled for each skim according to this probability. "
                 "An event weight is stored for each skim in the event extra info as 'weight_<SkimName>', "
                 "either as the inverse probability if the event is sampled for that skim, or 0 otherwise. "
                 "If an event is sampled for none of the provided skims, the module returns 0, otherwise 1. "
                 "Use case is the reduction of simulation time for directly skimmed MC productions.");
  addParam("skimCodes", m_skimCodes, "Skim LFN codes");
  addParam("overrideEventType", m_overrideEventType,
           "Override automatically determined event type", false);
  addParam("eventType", m_eventType, "Event type (charged, mixed, uubar, ccbar, ddbar, ssbar, taupair)",
           std::string("unset"));
  addParam("payload", m_payload, "Name of payload storing neural network weights in ONNX format",
           std::string("SmartBKGWeights.onnx"));
  addParam("debugMode", m_debugMode,
           "Debug mode execution (in debug mode, always returns 1 and NN predictions are saved to the event extra info "
           "as 'SmartBKG_Prediction')", false);
  addParam("overrideActivation", m_activationOverride, "Override parameters (a, b) of the activation function (clipped exponential)",
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
  m_skimnamesMapping = config->getSkimnamesMapping();

  // Check parameters
  for (int skimCode : m_skimCodes) {
    if (m_skimcodesMapping.find(skimCode) == m_skimcodesMapping.end()) {
      B2FATAL("SmartBkg: Provided skim code " << skimCode << " is unknown. Check documentation for allowed skim codes.");
    }
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
  if (m_overrideEventType) {
    if (m_eventType == "unset") {
      B2FATAL("SmartBkg: overrideEventType is set to true but eventType is not set.");
    } else if (c_eventtypeMapping.find(m_eventType) == c_eventtypeMapping.end()) {
      B2FATAL("SmartBkg: Provided event type " << m_eventType <<
              " is unknown. Allowed event types: charged, mixed, uubar, ddbar, ssbar, ccbar, taupair.");
    } else {
      B2DEBUG(20, "SmartBkg: Event type override is enabled, using event type " << m_eventType);
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

  // Set correct event type
  StoreObjPtr<EventExtraInfo> eventExtraInfo;
  std::string detectedEventType = eventExtraInfo->getEventType();
  if (detectedEventType == "") {
    if (!m_overrideEventType) {
      B2FATAL("SmartBkg: Event type could not be automatically determined, please set override parameter.");
    }
  } else {
    if (!m_overrideEventType) {
      m_eventType = detectedEventType;
      B2DEBUG(20, "SmartBkg: Automatically inferred event type as '" << m_eventType << "'");
      if (c_eventtypeMapping.find(m_eventType) == c_eventtypeMapping.end()) {
        B2FATAL("SmartBkg: Inferred event type '" << m_eventType <<
                "' is unknown. Allowed event types: charged, mixed, uubar, ddbar, ssbar, ccbar, taupair.");
      }
    } else {
      if (m_eventType != detectedEventType) {
        B2WARNING("SmartBkg: Event type override disagrees with event type inferred from event extra info! \n"
                  "                    Override: '" << m_eventType <<
                  "'\n                    Extra info: '" << detectedEventType <<
                  "'\n                    Are you sure this is what you want?");
      }
    }
  }

  // Load store array of MC particles and the event extra info
  StoreArray<MCParticle> mcparticles;
  const int numParticles = mcparticles.getEntries();

  // Clear vectors that hold preprocessed input data
  m_xValues.resize(0);
  m_pdgValues.resize(0);
  m_motherValues.resize(0);

  // New 0-based index of particles after preprocessing
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

  // Extract predictions for skims and perform importance sampling
  // For each skim a seperate weight is saved to the event extra info (0 if event is not sampled for that skim)
  // If event is sampled for no skim, return value is false
  bool returnValue = false;
  for (int skimCode : m_skimCodes) {
    float prediction;
    const uint16_t skimIndex = m_skimcodesMapping[skimCode];
    const std::string skimName = m_skimnamesMapping[skimCode];
    if (m_activationOverride) {
      prediction = this->activation(outputTensor->at({skimIndex}), m_activationOverrideParams[0], m_activationOverrideParams[1]);
    } else {
      std::vector<float> params = m_paramsMapping[skimCode];
      prediction = this->activation(outputTensor->at({skimIndex}), params[0], params[1]);
    }
    if (!m_debugMode) {
      const double randomNum = gRandom->Uniform(0, 1);
      if (randomNum < prediction) {
        returnValue = true;
        eventExtraInfo->addExtraInfo("weight_" + skimName, 1 / prediction);
        B2DEBUG(20, "SmartBkg: Prediction for skim " << skimName << " is " << prediction << ", weight is set to " << 1 / prediction);
      } else {
        eventExtraInfo->addExtraInfo("weight_" + skimName, 0);
        B2DEBUG(20, "SmartBkg: Prediction for skim " << skimName << " is " << prediction << ", weight is set to 0");
      }
    } else {
      returnValue = true;
      eventExtraInfo->addExtraInfo("SmartBKG_Prediction_" + skimName, prediction);
    }
  }
  if (returnValue) {
    B2DEBUG(20, "SmartBkg: Event is kept");
  } else {
    B2DEBUG(20, "SmartBkg: Event is discarded");
  }
  this->setReturnValue(returnValue);

}