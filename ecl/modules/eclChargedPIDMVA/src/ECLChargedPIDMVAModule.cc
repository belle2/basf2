/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <math.h>
#include <algorithm>
#include "TMath.h"

#include <framework/geometry/B2Vector3.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>

#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/geometry/ECLNeighbours.h>

#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/variables/ECLVariables.h>
#include <analysis/dataobjects/Particle.h>

#include <mva/interface/Interface.h>
#include <mva/methods/TMVA.h>
#include <mdst/dataobjects/Track.h>
#include <limits>
#include <string>

#include <ecl/modules/eclChargedPIDMVA/ECLChargedPIDMVAModule.h>

using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLChargedPIDMVA)

ECLChargedPIDMVAModule::ECLChargedPIDMVAModule() : Module()
{
  setDescription("The module implements charged particle identification using ECL-related observables via a multiclass BDT. For each track matched with a suitable ECLShower, the relevant ECL variables (shower shape, PSD etc.) are fed to the BDT which is stored in a conditions database payload. The BDT output variables are then used to construct a likelihood from pdfs also stored in the payload. The likelihood is then stored in the ECLPidLikelihood object.");

  addParam("payloadName",
           m_payload_name,
           "The name of the database payload object with the MVA weights.",
           std::string("ECLChargedPidMVAWeights"));
}

ECLChargedPIDMVAModule::~ECLChargedPIDMVAModule() = default;

void ECLChargedPIDMVAModule::initialize()
{
  m_eventMetaData.isRequired();

  m_mvaWeights = std::make_unique<DBObjPtr<ECLChargedPIDMVAWeights>>(m_payload_name);

  m_eclPidLikelihoods.registerInDataStore();
  m_tracks.registerRelationTo(m_eclPidLikelihoods);
}

void ECLChargedPIDMVAModule::checkDBPayloads()
{
  if (!m_mvaWeights) {B2FATAL("No ECLChargedPidMVAWeights payload found in database!");}
}


void ECLChargedPIDMVAModule::initializeMVA()
{
  B2DEBUG(12, "Run: " << m_eventMetaData->getRun() <<
          ". Load supported MVA interfaces for multi-class charged particle identification...");

  m_log_transform_offset = (*m_mvaWeights.get())->getLogTransformOffset();

  MVA::AbstractInterface::initSupportedInterfaces();
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();

  auto nCategories = (*m_mvaWeights.get())->nCategories();

  B2DEBUG(12, " number of categories booked: " << nCategories);

  m_experts.resize(nCategories);
  m_variables.resize(nCategories);


  for (unsigned int idx(0); idx < nCategories; idx++) {

    B2DEBUG(12, "\t\tweightfile[" << idx << "]");

    // De-serialize the string into an MVA::Weightfile object.
    std::stringstream ss((*m_mvaWeights.get())->getPhasespaceCategory(idx)->getSerialisedWeight());
    auto weightfile = MVA::Weightfile::loadFromStream(ss);

    MVA::GeneralOptions general_options;
    weightfile.getOptions(general_options);

    // Store an MVA::Expert object.
    m_experts[idx] = supported_interfaces[general_options.m_method]->getExpert();
    m_experts.at(idx)->load(weightfile);

    B2DEBUG(12, "\t\tweightfile  at " << idx << " successfully initialised.");

    // Load the variable objects
    Variable::Manager& manager = Variable::Manager::Instance();

    // get the full version of the variable names.
    // These are stored in the xml as an additional vector.
    const std::string identifier = std::string("de_aliased_clf_vars");
    auto clf_vars = weightfile.getVector<std::string>(identifier);
    m_variables[idx] = manager.getVariables(clf_vars);

    std::vector<float> features(general_options.m_variables.size(), 0.0);
    std::vector<float> spectators;

    m_datasets[idx] = std::make_unique<MVA::SingleDataset>(general_options, features, 1.0, spectators);
  }
}


void ECLChargedPIDMVAModule::beginRun()
{
  (*m_mvaWeights.get()).addCallback([this]() { checkDBPayloads();});
  checkDBPayloads();
  (*m_mvaWeights.get()).addCallback([this]() { initializeMVA(); });
  initializeMVA();
}

void ECLChargedPIDMVAModule::event()
{
  for (const auto& track : m_tracks) {

    // load the pion fit hypothesis or closest in mass.
    const TrackFitResult* fitRes = track.getTrackFitResultWithClosestMass(Const::pion);
    if (fitRes == nullptr) continue;

    // we require a track to have at least one shower matched to it.
    const auto relShowers = track.getRelationsTo<ECLShower>();
    if (relShowers.size() == 0) continue;

    // create a particle object so that we can use the variable manager.
    const Particle particle = Particle(&track, Const::ChargedStable(211));

    const double p = fitRes->getMomentum().Mag();
    const int charge = fitRes->getChargeSign();
    const double showerTheta = Belle2::Variable::eclClusterTheta(&particle);

    //get the MVA region
    unsigned int linearBinIndex = (*m_mvaWeights.get())->getLinearisedBinIndex(showerTheta, p, charge);
    unsigned int nvars = m_variables.at(linearBinIndex).size();

    // get the phasespaceCategory
    const auto phasespaceCategory = (*m_mvaWeights.get())->getPhasespaceCategory(linearBinIndex);

    // fill the feature vectors
    for (unsigned int ivar(0); ivar < nvars; ++ivar) {
      auto varobj = m_variables.at(linearBinIndex).at(ivar);

      float val = std::numeric_limits<float>::quiet_NaN();

      if (std::holds_alternative<double>(varobj->function(&particle))) {
        val = (float)std::get<double>(varobj->function(&particle));
      } else if (std::holds_alternative<int>(varobj->function(&particle))) {
        val = (float)std::get<int>(varobj->function(&particle));
      } else if (std::holds_alternative<bool>(varobj->function(&particle))) {
        val = (float)std::get<bool>(varobj->function(&particle));
      } else {
        B2ERROR("Variable '" << varobj->name << "' has wrong data type! It must be one of double, integer, or bool.");
      }

      if (std::isnan(val)) {
        B2ERROR("Variable '" << varobj->name <<
                "' has NaN entries. Variable definitions should include 'ifNaNGiveX(X, DEFAULT)' with a proper default value.");
      }
      m_datasets.at(linearBinIndex)->m_input[ivar] = val;
    }

    // get the mva response and convert to likelihood
    B2DEBUG(12, "Bin index, theta, p, charge: " << linearBinIndex << "  " << showerTheta << "  " << p << "  " << charge << " \n");
    std::vector<float> scores = m_experts.at(linearBinIndex)->applyMulticlass(*m_datasets.at(linearBinIndex))[0];

    // log transform the scores
    for (unsigned int iResponse = 0; iResponse < scores.size(); iResponse++) {
      scores[iResponse] = logTransformation(scores[iResponse]);
    }

    float logLikelihoods[Const::ChargedStable::c_SetSize];

    // Order of loop is defined in UnitConst.cc: e, mu, pi, K, p, d
    for (const auto& hypo : Const::chargedStableSet) {
      unsigned int hypo_idx = hypo.getIndex();
      auto absPdgId = abs(hypo.getPDGCode());

      // copy the scores so they arent transformed in place
      std::vector<float> transformed_scores;
      transformed_scores = scores;

      // perform extra transformations if they are booked
      if ((phasespaceCategory->getTransformMode() ==
           ECLChargedPIDPhasespaceCategory::BDTResponseTransformMode::c_GaussianTransform)
          or
          (phasespaceCategory->getTransformMode() ==
           ECLChargedPIDPhasespaceCategory::BDTResponseTransformMode::c_DecorrelationTransform)) {

        // gaussian transform
        for (unsigned int iResponse = 0; iResponse < scores.size(); iResponse++) {
          transformed_scores[iResponse] = gaussTransformation(scores[iResponse], phasespaceCategory->getCDF(absPdgId,  iResponse));
        }
        if (phasespaceCategory->getTransformMode() ==
            ECLChargedPIDPhasespaceCategory::BDTResponseTransformMode::c_DecorrelationTransform) {
          transformed_scores = decorrTransformation(transformed_scores, phasespaceCategory->getDecorrelationMatrix(absPdgId));
        }
      }

      // get the pdf values for each response value
      float logL = 0.0;
      for (unsigned int iResponse = 0; iResponse < transformed_scores.size(); iResponse++) {

        if ((phasespaceCategory->getTransformMode() ==
             ECLChargedPIDPhasespaceCategory::BDTResponseTransformMode::c_LogTransformSingle) and (hypo_idx != iResponse)) {continue;}

        double xmin, xmax;
        phasespaceCategory->getPDF(absPdgId, iResponse)->GetRange(xmin, xmax);
        // kick the response back into the range of the PDF (alternatively consider logL = std::numeric_limits<float>::min() if outside range)
        float transformed_score_copy = transformed_scores[iResponse];
        if (transformed_score_copy < xmin) transformed_score_copy = xmin + 1e-5;
        if (transformed_score_copy > xmax) transformed_score_copy = xmax - 1e-5;

        float pdfval = phasespaceCategory->getPDF(absPdgId, iResponse)->Eval(transformed_score_copy);
        // dont take a log of inf or 0
        pdfval = std::max(pdfval, std::numeric_limits<float>::min());
        logL += (std::isnormal(pdfval) && pdfval > 0) ? std::log(pdfval) : c_dummyLogL;
      }
      logLikelihoods[hypo_idx] = logL;
    } // hypo loop

    const auto eclPidLikelihood = m_eclPidLikelihoods.appendNew(logLikelihoods);

    track.addRelationTo(eclPidLikelihood);
  } // tracks
}


float ECLChargedPIDMVAModule::logTransformation(const float value) const
{
  // BDT response is limited to be between 0 and 1 so can safely hardcode 1.0 as the max value.
  return std::log((value + m_log_transform_offset) / (1.0 + m_log_transform_offset - value));
}


float ECLChargedPIDMVAModule::gaussTransformation(const float value, const TH1F* cdf) const
{
  int binIndex = cdf->GetXaxis()->FindBin(value);

  double dx = cdf->GetBinWidth(binIndex);
  double cumulant = cdf->GetBinContent(binIndex);
  double dy;

  if (binIndex == cdf->GetNbinsX()) {
    dy = 1 - cdf->GetBinContent(binIndex);
  } else if (binIndex == 0) {
    dy = cdf->GetBinContent(binIndex);
  } else {
    dy = cdf->GetBinContent(binIndex + 1) - cdf->GetBinContent(binIndex);
  }

  cumulant = cumulant + (value - cdf->GetBinLowEdge(binIndex)) * dy / dx;

  cumulant = std::min(cumulant, 1.0 - 1e-9);
  cumulant = std::max(cumulant, 1e-9);

  double maxErfInvArgRange = 0.99999999;
  double arg = 2.0 * cumulant - 1.0;

  arg = std::min(maxErfInvArgRange, arg);
  arg = std::max(-maxErfInvArgRange, arg);

  return c_sqrt2 * TMath::ErfInverse(arg);
}


std::vector<float> ECLChargedPIDMVAModule::decorrTransformation(const std::vector<float> scores,
    const std::vector<float>* decorrelationMatrix) const
{
  unsigned int n_scores = scores.size();
  std::vector<float> decor_scores(n_scores, 0.0);

  for (unsigned int irow = 0; irow < n_scores; irow++) {
    unsigned int offset = irow * n_scores;
    for (unsigned int icol = 0; icol < n_scores; icol++) {
      decor_scores[irow] += scores[irow] * decorrelationMatrix->at(icol + offset);
    }
  }
  return decor_scores;
}