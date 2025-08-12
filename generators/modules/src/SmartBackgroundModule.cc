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

  // Session options
  m_sessionOptions.SetIntraOpNumThreads(1);
  m_sessionOptions.SetInterOpNumThreads(1);
  m_sessionOptions.SetExecutionMode(ORT_SEQUENTIAL);

  // initialize ONNX session
  m_session = std::make_unique<Ort::Session>(m_env, filename.c_str(), m_sessionOptions);

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

  // Create vector of input tensors for the five inputs of the SmartBKG model
  std::vector<Ort::Value> inputs;
  float xArray[100 * 8];
  int64_t xShape[] = {1, 100, 8};
  inputs.push_back(Ort::Value::CreateTensor(m_memoryInfo, xArray, 100 * 8, xShape, 3));
  int32_t pdgArray[100];
  int64_t pdgShape[] = {1, 100};
  inputs.push_back(Ort::Value::CreateTensor(m_memoryInfo, pdgArray, 100, pdgShape, 2));
  int32_t motherArray[100];
  int64_t motherShape[] = {1, 100};
  inputs.push_back(Ort::Value::CreateTensor(m_memoryInfo, motherArray, 100, motherShape, 2));
  uint8_t maskArray[100];
  int64_t maskShape[] = {1, 100};
  inputs.push_back(Ort::Value::CreateTensor(m_memoryInfo, maskArray, 100, maskShape, 2));
  int32_t cArray[1];
  int64_t cShape[] = {1};
  inputs.push_back(Ort::Value::CreateTensor(m_memoryInfo, cArray, 1, cShape, 1));

  // Create output tensor
  float outputArray[51];
  int64_t outputShape[] = {1, 51};
  auto outputTensor = Ort::Value::CreateTensor(m_memoryInfo, outputArray, 51, outputShape, 2);

  // Warning about 100 particle truncation
  if (numParticles > 100) {
    B2DEBUG(20, "More than 100 MC particles in event, cutting off excess particles");
  }

  // Set event type input
  cArray[0] = c_eventtypeMapping.at(m_eventType);

  // Loop over particles and set particle wise inputs
  for (unsigned i = 0; i < 100; ++i) {
    if (i < numParticles) {

      // If particle exists set mask to 0
      maskArray[i] = 0;

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
        xArray[i * 8 + j] = x[j];
      }

      // Set mapped PDG code input
      int pdg = p.getPDG();
      int pdgMapped = c_pdgMapping.at(pdg);
      if (!pdgMapped) {
        B2WARNING("Encountered particle with unknown pdg: " << pdg << ", assigning out-of-distribution value 0 as mapped pdg input.");
        pdgMapped = 0;
      }
      pdgArray[i] = pdgMapped;

      // Set mother index input
      MCParticle* mother = p.getMother();
      int motherindex = -1;
      if (mother) {
        motherindex = mother->getArrayIndex();
      }
      motherArray[i] = motherindex;

    } else {
      // If particle does not exist, set mask to 1 and all inputs to 0
      maskArray[i] = 1;
      for (unsigned j = 0; j < 8; ++j) {
        xArray[i * 8 + j] = 0;
      }
      pdgArray[i] = 0;
      motherArray[i] = 0;
    }
  }

  // Perform inference
  m_session->Run(m_runOptions, c_inputNames, inputs.data(), inputs.size(), c_outputNames, &outputTensor, 1);

  // Extract prediction for selected skim
  int skimIndex = c_skimcodesMapping.at(m_skimCode);
  float prediction = activation(outputArray[skimIndex], c_aMapping.at(m_skimCode), c_bMapping.at(m_skimCode));

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