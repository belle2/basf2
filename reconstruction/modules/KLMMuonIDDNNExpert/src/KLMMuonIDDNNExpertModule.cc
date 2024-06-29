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

#include <reconstruction/modules/KLMMuonIDDNNExpert/KLMMuonIDDNNExpertModule.h>

#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/bklm/geometry/Module.h>
#include <klm/eklm/geometry/TransformDataGlobalAligned.h>

#include <CLHEP/Units/SystemOfUnits.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

#include <tracking/dataobjects/ExtHit.h>

#include <mdst/dataobjects/PIDLikelihood.h>

#include <mva/interface/Interface.h>

#include <boost/algorithm/string/predicate.hpp>

using namespace Belle2;

REG_MODULE(KLMMuonIDDNNExpert);

KLMMuonIDDNNExpertModule::KLMMuonIDDNNExpertModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Get information from KLMMuIDLikelihood)DOC");
  setPropertyFlags(c_ParallelProcessingCertified);
}

KLMMuonIDDNNExpertModule::~KLMMuonIDDNNExpertModule()
{
}

void KLMMuonIDDNNExpertModule::initialize()
{
  m_tracks.isRequired();

  m_inputVariable.registerInDataStore();
  m_tracks.registerRelationTo(m_inputVariable);

  // setup KLM geometry
  bklm::GeometryPar* bklmGeometry = bklm::GeometryPar::instance();
  const EKLM::GeometryData& eklmGeometry = EKLM::GeometryData::Instance();

  m_EndcapScintWidth = eklmGeometry.getStripGeometry()->getWidth() / CLHEP::cm; // in G4e units (cm)

  for (int layer = 1; layer <= m_maxBKLMLayers; ++layer) {
    const bklm::Module* module =
      bklmGeometry->findModule(BKLMElementNumbers::c_ForwardSection, 1, layer);
    m_BarrelPhiStripWidth[layer - 1] = module->getPhiStripWidth(); // in G4e units (cm)
    m_BarrelZStripWidth[layer - 1] = module->getZStripWidth(); // in G4e units (cm)
  }

  MVA::AbstractInterface::initSupportedInterfaces();
}

void KLMMuonIDDNNExpertModule::terminate()
{
  m_expert.reset();
  m_dataset.reset();
}

void KLMMuonIDDNNExpertModule::beginRun()
{
  if (m_weightfile_representation.isValid()) {
    if (m_weightfile_representation.hasChanged()) {
      std::stringstream ss(m_weightfile_representation->m_data);
      auto weightfile = MVA::Weightfile::loadFromStream(ss);
      initializeMVA(weightfile);
    }
  } else {
    B2FATAL("Payload " << m_identifier << " is not valid!");
  }
}

void KLMMuonIDDNNExpertModule::endRun()
{
}

void KLMMuonIDDNNExpertModule::initializeMVA(MVA::Weightfile& weightfile)
{
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);
  m_expert = supported_interfaces[general_options.m_method]->getExpert();
  m_expert->load(weightfile);
  std::vector<float> dummy;
  int nInputVariables = general_options.m_variables.size();
  if (nInputVariables != 5 + 4 * (m_maxBKLMLayers + m_maxEKLMLayers)) {
    B2FATAL("Number of input variables mismatch. Required " << 5 + 4 * (m_maxBKLMLayers + m_maxEKLMLayers) << " but " << nInputVariables
            << " given. ");
  }
  dummy.resize(nInputVariables, 0);
  m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, std::move(dummy), 0));
}


void KLMMuonIDDNNExpertModule::event()
{
  for (Track& track : m_tracks) {

    KLMMuidLikelihood* klmll = track.getRelatedTo<KLMMuidLikelihood>();

    if (!klmll) continue;

    // initialize hit pattern arrays
    m_hitpattern_steplength.fill(-1.);
    m_hitpattern_width.fill(-1.);
    m_hitpattern_chi2.fill(-1.);
    m_hitpattern_hasext.fill(0);

    bool hasExtInKLM = false;
    for (const ExtHit& exthit : track.getRelationsTo<ExtHit>()) {

      if (exthit.getDetectorID() != Const::EDetector::BKLM
          and exthit.getDetectorID() != Const::EDetector::EKLM) continue;

      int layer;
      bool inBKLM = (exthit.getDetectorID() == Const::EDetector::BKLM);
      int copyid = exthit.getCopyID();

      int section, sector, plane, strip;

      if (inBKLM) {
        BKLMElementNumbers::moduleNumberToElementNumbers(copyid, &section, &sector, &layer);
        if (layer > m_maxBKLMLayers) continue;
        m_hitpattern_hasext.at(layer - 1) = 1;
      } else {
        EKLMElementNumbers::Instance().stripNumberToElementNumbers(copyid, &section, &layer, &sector, &plane, &strip);
        if (layer > m_maxEKLMLayers) continue;
        m_hitpattern_hasext.at(m_maxBKLMLayers + layer - 1) = 1;
      }

      hasExtInKLM = true;

    }

    RelationVector<KLMHit2d> KLMHit2drelation = track.getRelationsTo<KLMHit2d>();

    // only apply NN muonID to tracks with at least one KLMHit2d or one ExtHit in KLM.
    if (not(hasExtInKLM || KLMHit2drelation.size())) continue;

    std::map<int, int> Hit2dMap; // arrange KLMHit2d in the order of layer
    for (long unsigned int ii = 0; ii < KLMHit2drelation.size(); ii++) {
      KLMHit2d* klmhit = KLMHit2drelation[ii];
      bool hit_inBKLM = (klmhit->getSubdetector() == KLMElementNumbers::c_BKLM);
      unsigned long int hit_layer = klmhit->getLayer();

      int index = hit_layer - 1 + m_maxBKLMLayers * (1 - hit_inBKLM); // BKLM hits are in front of EKLM hits
      if (index > (m_maxBKLMLayers + m_maxEKLMLayers)) continue;
      Hit2dMap.insert(std::pair<int, int> {index, ii});
    }

    int nklmhits = 0;
    ROOT::Math::XYZVector previousPosition(0., 0., 0.);
    for (auto itermap = Hit2dMap.begin(); itermap != Hit2dMap.end(); itermap ++) {

      nklmhits += 1;

      KLMHit2d* klmhit = KLMHit2drelation[itermap->second];

      float KFchi2 = KLMHit2drelation.weight(itermap->second);
      float width = getHitWidth(klmhit);

      ROOT::Math::XYZVector hitPosition = klmhit->getPosition();
      float steplength = 0.;
      if (nklmhits > 1) {
        steplength = (hitPosition - previousPosition).R();
      }
      previousPosition = hitPosition;

      // hit pattern creation.
      int hitpatternindex = itermap->first;
      m_hitpattern_chi2.at(hitpatternindex) = KFchi2;
      m_hitpattern_steplength.at(hitpatternindex) = steplength;
      m_hitpattern_width.at(hitpatternindex) = width;
    } // loop of Hit2dMap

    Hit2dMap.clear();

    double muprob_nn = getNNmuProbability(&track, klmll);
    PIDLikelihood* pid = track.getRelated<PIDLikelihood>();
    pid->addPreOfficialLikelihood("klmMuonIDDNN", muprob_nn);
  } // loop of tracks
}

float KLMMuonIDDNNExpertModule::getNNmuProbability(const Track* track, const KLMMuidLikelihood* klmll)
{
  m_dataset->m_input[0] = klmll->getChiSquared();
  m_dataset->m_input[1] = klmll->getDegreesOfFreedom();
  m_dataset->m_input[2] = klmll->getExtLayer() - klmll->getHitLayer();
  m_dataset->m_input[3] = klmll->getExtLayer();
  m_dataset->m_input[4] = track->getTrackFitResultWithClosestMass(Const::muon)->getTransverseMomentum();

  for (int layer = 0; layer < (m_maxBKLMLayers + m_maxEKLMLayers); layer ++) {
    m_dataset->m_input[5 + 4 * layer + 0] = m_hitpattern_width.at(layer); // width
    m_dataset->m_input[5 + 4 * layer + 1] = m_hitpattern_steplength.at(layer); // steplength
    m_dataset->m_input[5 + 4 * layer + 2] = m_hitpattern_chi2.at(layer); // chi2
    m_dataset->m_input[5 + 4 * layer + 3] = m_hitpattern_hasext.at(layer); // hasext
  }

  KLMMuonIDDNNInputVariable* inputVariable = m_inputVariable.appendNew();
  inputVariable->setKLMMuonIDDNNInputVariable(m_dataset->m_input);
  track->addRelationTo(inputVariable);

  float muprob_nn = m_expert->apply(*m_dataset)[0];
  return muprob_nn;
}

float KLMMuonIDDNNExpertModule::getHitWidth(const KLMHit2d* klmhit)
{
  float stripwidth1 = 0; // strip width of phi or X direction
  float stripwidth2 = 0; // strip width of Z or Y direction
  float stripdiff1 = 0; // max minus min strip number in phi or X direction
  float stripdiff2 = 0; // max minus min strip number in Z or Y direction
  if (klmhit->getSubdetector() == KLMElementNumbers::c_BKLM) {
    stripwidth1 = m_BarrelPhiStripWidth[klmhit->getLayer() - 1];
    stripwidth2 = m_BarrelZStripWidth[klmhit->getLayer() - 1];
    stripdiff1 = (klmhit->getPhiStripMax() - klmhit->getPhiStripMin() + 1) * 0.5;
    stripdiff2 = (klmhit->getZStripMax() - klmhit->getZStripMin() + 1) * 0.5;
  } else {
    stripwidth1 = m_EndcapScintWidth;
    stripwidth2 = m_EndcapScintWidth;
    stripdiff1 = (klmhit->getXStripMax() - klmhit->getXStripMin() + 1) * 0.5;
    stripdiff2 = (klmhit->getYStripMax() - klmhit->getYStripMin() + 1) * 0.5;
  }
  float width1 = stripwidth1 * stripdiff1;
  float width2 = stripwidth2 * stripdiff2;
  return std::sqrt(width1 * width1 + width2 * width2);
}


