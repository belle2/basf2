/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// TODOs: write documentation, set payload name in python?

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
                 "The module returns 0 with this probability or otherwise 1. "
                 "Events are then weighted with their inverse probability "
                 "(weights are saved in the event meta data, by multiplying them to the generator weight). "
                 "Use case is the reduction of simulation time for directly skimmed MC productions.");
  addParam("skimCode", m_skimCode, "Skim LFN code");
  addParam("eventType", m_eventType, "Event type (charged, mixed, uubar, ccbar, ddbar, ssbar, taupair)");
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
  auto accessor = DBAccessorBase(DBStoreEntry::c_RawFile, std::string("SmartBKGWeights.onnx"), true);
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
  const unsigned int numParticles = mcparticles.getEntries();

  // Create input tensors for the five inputs of the SmartBKG model
  auto xTensor = MVA::ONNX::Tensor<float>::make_shared({1, 100, 8});
  auto pdgTensor = MVA::ONNX::Tensor<int32_t>::make_shared({1, 100});
  auto motherTensor = MVA::ONNX::Tensor<int32_t>::make_shared({1, 100});
  auto maskTensor = MVA::ONNX::Tensor<uint8_t>::make_shared({1, 100});
  auto cTensor = MVA::ONNX::Tensor<int32_t>::make_shared({1});

  // Create output tensor
  auto outputTensor = MVA::ONNX::Tensor<float>::make_shared({1, 51});

  // Warning about 100 particle truncation
  if (numParticles > 100) {
    B2DEBUG(20, "SmartBkg: More than 100 MC particles in event, cutting off excess particles");
  }

  // Set event type input
  cTensor->at(0) = c_eventtypeMapping.at(m_eventType);

  // Loop over particles and set particle wise inputs
  for (unsigned int i = 0; i < 100; ++i) {
    if (i < numParticles) {

      // If particle exists set mask to 0
      maskTensor->at({0, i}) = 0;

      const MCParticle& p = *mcparticles[i];

      // Set momentum and vertex 4-vector inputs
      const ROOT::Math::XYZVector momentum = p.getMomentum();
      const ROOT::Math::XYZVector vertex = p.getVertex();
      xTensor->at({0, i, 0}) = p.getProductionTime();
      xTensor->at({0, i, 1}) = vertex.X();
      xTensor->at({0, i, 2}) = vertex.Y();
      xTensor->at({0, i, 3}) = vertex.Z();
      xTensor->at({0, i, 4}) = p.getEnergy();
      xTensor->at({0, i, 5}) = momentum.X();
      xTensor->at({0, i, 6}) = momentum.Y();
      xTensor->at({0, i, 7}) = momentum.Z();

      // Set mapped PDG code input
      const int pdg = p.getPDG();
      int pdgMapped = m_pdgMapping[pdg];
      if (!pdgMapped) {
        B2WARNING("SmartBkg: Encountered particle with unknown pdg: " << pdg <<
                  ", assigning out-of-distribution value 0 as mapped pdg input.");
        pdgMapped = 0;
      }
      pdgTensor->at({0, i}) = pdgMapped;

      // Set mother index input
      MCParticle* mother = p.getMother();
      int motherindex = -1;
      if (mother) {
        motherindex = mother->getArrayIndex();
      }
      motherTensor->at({0, i}) = motherindex;

    } else {
      // If particle does not exist, set mask to 1 and all inputs to 0
      maskTensor->at({0, i}) = 1;
      for (unsigned j = 0; j < 8; ++j) {
        xTensor->at({0, i, j}) = 0;
      }
      pdgTensor->at({0, i}) = 0;
      motherTensor->at({0, i}) = 0;
    }
  }

  // Perform inference
  m_session->run({{"x", xTensor}, {"pdg", pdgTensor}, {"mother", motherTensor}, {"mask", maskTensor}, {"c", cTensor}}, {{"output", outputTensor}});

  // Extract prediction for selected skim
  const uint16_t skimIndex = m_skimcodesMapping[m_skimCode];
  float prediction;
  if (m_activationOverride) {
    prediction = this->activation(outputTensor->at({0, skimIndex}), m_activationOverrideParams[0], m_activationOverrideParams[1]);
  } else {
    std::vector<float> params = m_paramsMapping[m_skimCode];
    prediction = this->activation(outputTensor->at({0, skimIndex}), params[0], params[1]);
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