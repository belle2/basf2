/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// TODOs: set payload name in python? StoreObjPtr fuer MCParticle List?, eventExtraInfo as member?

#include <generators/modules/SmartBackgroundModule.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/dataobjects/EventExtraInfo.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBAccessorBase.h>
#include <framework/database/DBStoreEntry.h>
#include <mva/methods/ONNX.h>

#include <TRandom.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SmartBackground);

SmartBackgroundModule::SmartBackgroundModule() : Module()
{

  setDescription("mva method to speed up skimmed simulation");

  addParam("skimCode", m_skimCode, "Skim LFN code");
  addParam("eventType", m_eventType, "Event type (charged, mixed, uubar, ccbar, ddbar, ssbar, taupair)", std::string("charged"));
  addParam("debugMode", m_debugMode,
           "Debug mode execution (in debug mode, no events are discarded and NN predictions are saved to the event extra info)", false);

}

void SmartBackgroundModule::initialize()
{

  // Check parameters
  if (c_skimcodesMapping.find(m_skimCode) == c_skimcodesMapping.end()) {
    B2FATAL("Provided skim code " << m_skimCode << " is unknown. Check documentation for allowed skim codes.");
  }
  if (c_eventtypeMapping.find(m_eventType) == c_eventtypeMapping.end()) {
    B2FATAL("Provided event type " << m_eventType <<
            " is unknown. Allowed event types: charged, mixed, uubar, ddbar, ssbar, ccbar, taupair.");
  }

  // Load model from payload
  auto accessor = DBAccessorBase(DBStoreEntry::c_RawFile, std::string("SmartBKGWeights.onnx"), true);
  std::string filename = accessor.getFilename();

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
  unsigned numParticles = mcparticles.getEntries();
  StoreObjPtr<EventExtraInfo> eventExtraInfo;

  // Create input tensors for the five inputs of the SmartBKG model
  auto xTensor = MVA::ONNX::Tensor<float>::make_shared(100 * 8, {1, 100, 8});
  auto pdgTensor = MVA::ONNX::Tensor<int32_t>::make_shared(100, {1, 100});
  auto motherTensor = MVA::ONNX::Tensor<int32_t>::make_shared(100, {1, 100});
  auto maskTensor = MVA::ONNX::Tensor<uint8_t>::make_shared(100, {1, 100});
  auto cTensor = MVA::ONNX::Tensor<int32_t>::make_shared(1, {1});

  // Create output tensor
  auto outputTensor = MVA::ONNX::Tensor<float>::make_shared(51, {1, 51});

  // Warning about 100 particle truncation
  if (numParticles > 100) {
    B2DEBUG(20, "More than 100 MC particles in event, cutting off excess particles");
  }

  // Set event type input
  cTensor->at(0) = c_eventtypeMapping.at(m_eventType);

  // Loop over particles and set particle wise inputs
  for (unsigned i = 0; i < 100; ++i) {
    if (i < numParticles) {

      // If particle exists set mask to 0
      maskTensor->at({0, i}) = 0;

      const MCParticle& p = *mcparticles[i];

      // Set momentum and vertex 4-vector inputs
      float energy = p.getEnergy();
      ROOT::Math::XYZVector momentum = p.getMomentum();
      float prodT = p.getProductionTime();
      ROOT::Math::XYZVector vertex = p.getVertex();
      float x[8] = {prodT, static_cast<float>(vertex.X()), static_cast<float>(vertex.Y()), static_cast<float>(vertex.Z()),
                    energy, static_cast<float>(momentum.X()), static_cast<float>(momentum.Y()), static_cast<float>(momentum.Z())
                   };
      for (unsigned j = 0; j < 8; ++j) {
        xTensor->at({0, i, j}) = x[j];
      }

      // Set mapped PDG code input
      int pdg = p.getPDG();
      int pdgMapped = c_pdgMapping.at(pdg);
      if (!pdgMapped) {
        B2WARNING("Encountered particle with unknown pdg: " << pdg << ", assigning out-of-distribution value 0 as mapped pdg input.");
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
  size_t skimIndex = c_skimcodesMapping.at(m_skimCode);
  float prediction = activation(outputTensor->at({0, skimIndex}), c_aMapping.at(m_skimCode), c_bMapping.at(m_skimCode));

  // Save weight to extra info
  if (!m_debugMode) {
    eventExtraInfo->addExtraInfo("SmartBKG_Weight", 1 / prediction);
  } else {
    eventExtraInfo->addExtraInfo("SmartBKG_Prediction", prediction);
  }

  // Importance sampling
  if (!m_debugMode) {
    double randomNum = gRandom->Uniform(0, 1);
    B2DEBUG(1, "prediction " << prediction << " random " << randomNum << " weight " << 1 / prediction);
    bool returnValue = randomNum < prediction;
    this->setReturnValue(returnValue);
  } else {
    this->setReturnValue(false);
  }

}