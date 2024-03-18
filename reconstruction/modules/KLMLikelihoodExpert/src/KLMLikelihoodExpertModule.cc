/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>

#include <reconstruction/modules/KLMLikelihoodExpert/KLMLikelihoodExpertModule.h>

#include <klm/dataobjects/KLMHit2d.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

#include <tracking/dataobjects/ExtHit.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/KLMNNLikelihood.h>

#include <mva/interface/Interface.h>

#include <boost/algorithm/string/predicate.hpp>

using namespace Belle2;

REG_MODULE(KLMLikelihoodExpert);

KLMLikelihoodExpertModule::KLMLikelihoodExpertModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Get information from KLMMuIDLikelihood)DOC");

  // Parameter definitions
  addParam("identifier", m_identifier,
           "Database identifier or file used to load the weights.",
           m_identifier);
}

KLMLikelihoodExpertModule::~KLMLikelihoodExpertModule()
{
}

void KLMLikelihoodExpertModule::initialize()
{
  m_tracks.isRequired();
  m_klmNNLikelihoods.registerInDataStore();
  m_tracks.registerRelationTo(m_klmNNLikelihoods);

  if (not(boost::ends_with(m_identifier, ".root") or boost::ends_with(m_identifier, ".xml"))) {
    m_weightfile_representation = std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>(new
                                  DBObjPtr<DatabaseRepresentationOfWeightfile>(m_identifier));
  }
  MVA::AbstractInterface::initSupportedInterfaces();
}

void KLMLikelihoodExpertModule::terminate()
{
  m_expert.reset();
  m_dataset.reset();
}

void KLMLikelihoodExpertModule::beginRun()
{
  if (m_weightfile_representation) {
    if (m_weightfile_representation->hasChanged()) {
      std::stringstream ss((*m_weightfile_representation)->m_data);
      auto weightfile = MVA::Weightfile::loadFromStream(ss);
      init_mva(weightfile);
    }
  } else {
    auto weightfile = MVA::Weightfile::loadFromFile(m_identifier);
    init_mva(weightfile);
  }
}

void KLMLikelihoodExpertModule::endRun()
{
}

void KLMLikelihoodExpertModule::init_mva(MVA::Weightfile& weightfile)
{
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);
  m_expert = supported_interfaces[general_options.m_method]->getExpert();
  m_expert->load(weightfile);
  std::vector<float> dummy;
  int nInputVariables = general_options.m_variables.size();
  dummy.resize(nInputVariables, 0);
  m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, std::move(dummy), 0));
}


void KLMLikelihoodExpertModule::event()
{
  for (Track& track : m_tracks) {

    KLMMuidLikelihood* klmll = track.getRelatedTo<KLMMuidLikelihood>();

    if (!klmll) continue;

    // initialize hit pattern arrays
    for (int layer = 0; layer < m_TotalKLMLayers; layer++) {
      m_hitpattern_steplength[layer] = -1;
      m_hitpattern_width[layer] = -1;
      m_hitpattern_chi2[layer] = -1;
      m_hitpattern_hasext[layer] = 0;
    }

    std::map<int, int> ExtHitMap;
    RelationVector<ExtHit> ExtHitrelation = track.getRelationsTo<ExtHit>();
    for (unsigned long int ii = 0; ii < ExtHitrelation.size(); ii++) {
      ExtHit* exthit = ExtHitrelation[ii];

      if (exthit->getDetectorID() != Const::EDetector::BKLM
          and exthit->getDetectorID() != Const::EDetector::EKLM) continue;

      int layer;
      bool inBKLM = (exthit->getDetectorID() == Const::EDetector::BKLM);
      int copyid = exthit->getCopyID();

      int section, sector, plane, strip;

      if (inBKLM) {
        BKLMElementNumbers::moduleNumberToElementNumbers(copyid, &section, &sector, &layer);
        m_hitpattern_hasext[layer - 1] = 1;
      } else {
        EKLMElementNumbers::Instance().stripNumberToElementNumbers(copyid, &section, &layer, &sector, &plane, &strip);
        m_hitpattern_hasext[15 + layer - 1] = 1;
      }

      int index = layer - 1 + 15 * (1 - inBKLM); // make sure BKLM hits is in front of EKLM hits
      ExtHitMap[index] = ii; // only keep the last ext hit in each layer
    }

    RelationVector<KLMHit2d> KLMHit2drelation = track.getRelationsTo<KLMHit2d>();

    // only apply NN muonID to tracks with at least one KLMHit2d or one ExtHit in KLM.
    if (not(ExtHitMap.size() || KLMHit2drelation.size())) continue;

    std::map<int, int> Hit2dMap; // arrange KLMHit2d order in layer
    for (long unsigned int ii = 0; ii < KLMHit2drelation.size(); ii++) {
      KLMHit2d* klmhit = KLMHit2drelation[ii];
      bool hit_inBKLM = (klmhit->getSubdetector() == KLMElementNumbers::c_BKLM);
      unsigned long int hit_layer = klmhit->getLayer();

      int index = hit_layer - 1 + 15 * (1 - hit_inBKLM); // BKLM hits are in front of EKLM hits
      Hit2dMap.insert(std::pair<int, int> {index, ii});
    }

    int nklmhits = 0;
    ROOT::Math::XYZVector previousPosition(0., 0., 0.);
    for (auto itermap = Hit2dMap.begin(); itermap != Hit2dMap.end(); itermap ++) {

      nklmhits += 1;

      KLMHit2d* klmhit = KLMHit2drelation[itermap->second];

      float KFchi2 = KLMHit2drelation.weight(itermap->second);
      float width = klmhit->getWidth();

      ROOT::Math::XYZVector hitPosition = klmhit->getPosition();
      float steplength = 0.;
      if (nklmhits > 1) {
        steplength = (hitPosition - previousPosition).R();
      }
      previousPosition = hitPosition;

      // hit pattern creation.
      int hitpatternindex = itermap->first;
      m_hitpattern_chi2[hitpatternindex] = KFchi2;
      m_hitpattern_steplength[hitpatternindex] = steplength;
      m_hitpattern_width[hitpatternindex] = width;
    } // loop of Hit2dMap

    ExtHitMap.clear();
    Hit2dMap.clear();

    double muprob_nn = getNNmuProbability(&track, klmll);
    KLMNNLikelihood* klmNNLikelihood = m_klmNNLikelihoods.appendNew();
    klmNNLikelihood->setKLMNNLikelihood(muprob_nn);
    track.addRelationTo(klmNNLikelihood);
  } // loop of tracks
}

float KLMLikelihoodExpertModule::getNNmuProbability(const Track* track, const KLMMuidLikelihood* klmll)
{
  m_dataset->m_input[0] = klmll->getChiSquared();
  m_dataset->m_input[1] = klmll->getDegreesOfFreedom();
  m_dataset->m_input[2] = klmll->getExtLayer() - klmll->getHitLayer();
  m_dataset->m_input[3] = klmll->getExtLayer();
  m_dataset->m_input[4] = track->getTrackFitResultWithClosestMass(Const::muon)->getTransverseMomentum();

  for (int layer = 0; layer < m_TotalKLMLayers; layer ++) {
    m_dataset->m_input[5 + 4 * layer + 0] = m_hitpattern_width[layer]; // width
    m_dataset->m_input[5 + 4 * layer + 1] = m_hitpattern_steplength[layer]; // steplength
    m_dataset->m_input[5 + 4 * layer + 2] = m_hitpattern_chi2[layer]; // chi2
    m_dataset->m_input[5 + 4 * layer + 3] = m_hitpattern_hasext[layer]; // hasext
  }

  float muprob_nn = m_expert->apply(*m_dataset)[0];
  return muprob_nn;
}


