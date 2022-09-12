/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclChargedPIDMVA/ECLChargedPIDMVAModule.h>

// FRAMEWORK
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ANALYSIS
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

// MVA
#include <mva/interface/Interface.h>
#include <mva/methods/TMVA.h>

// MDST
#include <mdst/dataobjects/Track.h>

// C++
#include <limits>
#include <string>
#include <math.h>
#include "TMath.h"

using namespace Belle2;
typedef ECLChargedPIDPhasespaceCategory::MVAResponseTransformMode transformModes;

REG_MODULE(ECLChargedPIDMVA);

ECLChargedPIDMVAModule::ECLChargedPIDMVAModule() : Module()
{
  setDescription("This module implements charged particle identification using ECL-related observables via a multiclass MVA. For each track a set of relevant ECL variables(shower shape, PSD etc.) are fed to the MVA method which is stored in a conditions database payload. The MVA output variables are then used to construct a likelihood from pdfs also stored in the payload. The likelihood is then stored in the ECLPidLikelihood object.");
  setPropertyFlags(c_ParallelProcessingCertified);
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
  B2DEBUG(22, "Run: " << m_eventMetaData->getRun() <<
          ". Loading supported MVA interfaces for multi-class charged particle identification.");

  Variable::Manager& manager = Variable::Manager::Instance();
  m_binningVariables = manager.getVariables((*m_mvaWeights.get())->getBinningVariables());

  MVA::AbstractInterface::initSupportedInterfaces();
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();

  for (const auto& iterator : * (*m_mvaWeights.get())->getPhasespaceCategories()) {

    B2DEBUG(22, "\t\tweightfile[" << iterator.first << "]");

    // De-serialize the string into an MVA::Weightfile object.
    std::stringstream ss(iterator.second.getSerialisedWeight());
    auto weightfile = MVA::Weightfile::loadFromStream(ss);

    MVA::GeneralOptions general_options;
    weightfile.getOptions(general_options);

    // Store an MVA::Expert object.
    m_experts[iterator.first] = supported_interfaces[general_options.m_method]->getExpert();
    m_experts.at(iterator.first)->load(weightfile);

    B2DEBUG(22, "\t\tweightfile  at " << iterator.first << " successfully initialised.");

    // Get the full version of the variable names.
    // These are stored in the xml as an additional vector.
    const std::string identifier = std::string("de_aliased_clf_vars");
    auto clf_vars = weightfile.getVector<std::string>(identifier);
    m_variables[iterator.first] = manager.getVariables(clf_vars);

    B2DEBUG(22, "\t\t Weightfile at " << iterator.first << " has " << m_variables[iterator.first].size() << " features.");

    std::vector<float> features(clf_vars.size(), 0.0);
    std::vector<float> spectators;

    m_datasets[iterator.first] = std::make_unique<MVA::SingleDataset>(general_options, features, 1.0, spectators);
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

    // Load the pion fit hypothesis or closest in mass.
    const TrackFitResult* fitRes = track.getTrackFitResultWithClosestMass(Const::pion);
    if (fitRes == nullptr) continue;

    // If a track omega value is very small trackFitResult might return a charge of 0.
    // The particle is then not able to be assigned a pdg code causing a crash.
    // Skip such tracks.
    const int charge = fitRes->getChargeSign();
    if (charge == 0) continue;

    // Create a particle object so that we can use the variable manager.
    const Particle particle = Particle(&track, Const::pion);

    // Get global bin index for track corresponding to N dimensional binning.
    std::vector<float> binningVariableValues(m_binningVariables.size());
    for (unsigned int ivar(0); ivar < binningVariableValues.size(); ivar++) {
      auto varobj = m_binningVariables.at(ivar);
      binningVariableValues.at(ivar) = evaluateVariable(varobj, &particle);
    }
    const int linearCategoryIndex = (*m_mvaWeights.get())->getLinearisedCategoryIndex(binningVariableValues);

    // Require we cover the phasespace.
    // Alternatively could take closest covered region but behaviour will not be well understood.
    // After this check the linearCategoryIndex is guaranteed to be positive.
    if (!(*m_mvaWeights.get())->isPhasespaceCovered(linearCategoryIndex)) continue;

    // Get the phasespaceCategory
    const auto phasespaceCategory = (*m_mvaWeights.get())->getPhasespaceCategory(linearCategoryIndex);
    const auto transformMode = phasespaceCategory->getTransformMode();

    // Fill the feature vectors
    unsigned int nvars = m_variables.at(linearCategoryIndex).size();
    for (unsigned int ivar(0); ivar < nvars; ++ivar) {
      auto varobj = m_variables.at(linearCategoryIndex).at(ivar);
      m_datasets.at(linearCategoryIndex)->m_input[ivar] = evaluateVariable(varobj, &particle);
    }

    // Get the mva response to be converted to a likelihood
    std::vector<float> scores = m_experts.at(linearCategoryIndex)->applyMulticlass(*m_datasets.at(linearCategoryIndex))[0];

    // Log transform the scores
    // This turns the MVA output which, if using the nominal TMVA MulticlassBDT,
    // is heavily peaked at 0 and 1 into a smooth curve.
    // We can then evaluate the likelihoods from this curve directly or further transform the responses.
    for (unsigned int iResponse = 0; iResponse < scores.size(); iResponse++) {
      if ((transformMode != transformModes::c_DirectMVAResponse) and
          (transformMode != transformModes::c_LogMVAResponse)) {
        scores[iResponse] = logTransformation(scores[iResponse],
                                              phasespaceCategory->getLogTransformOffset(),
                                              phasespaceCategory->getMaxPossibleResponseValue());
      }
    }

    B2DEBUG(22, "Scores: ");
    for (unsigned int iResponse = 0; iResponse < scores.size(); iResponse++) {
      B2DEBUG(22, "\t\t " << iResponse << " : " << scores[iResponse]);
    }

    float logLikelihoods[Const::ChargedStable::c_SetSize];

    // Order of loop is defined in UnitConst.cc: e, mu, pi, K, p, d
    for (const auto& hypo : Const::chargedStableSet) {
      unsigned int hypo_idx = hypo.getIndex();
      auto absPdgId = abs(hypo.getPDGCode());

      // Copy the scores so they aren't transformed in place
      std::vector<float> transformed_scores;
      transformed_scores = scores;

      // Perform extra transformations if they are booked
      if ((transformMode == transformModes::c_GaussianTransform) or
          (transformMode == transformModes::c_DecorrelationTransform)) {

        // Gaussian transform
        for (unsigned int iResponse = 0; iResponse < scores.size(); iResponse++) {
          transformed_scores[iResponse] = gaussTransformation(scores[iResponse], phasespaceCategory->getCDF(absPdgId,  iResponse));
        }
        if (transformMode == transformModes::c_DecorrelationTransform) {
          transformed_scores = decorrTransformation(transformed_scores, phasespaceCategory->getDecorrelationMatrix(absPdgId));
        }
      }

      // Get the pdf values for each response value
      float logL = 0.0;
      if ((transformMode == transformModes::c_DirectMVAResponse) or
          (transformMode == transformModes::c_LogMVAResponse)) {

        logLikelihoods[hypo_idx] = scores[phasespaceCategory->getMVAIndexForHypothesis(absPdgId)];
        if (transformMode == transformModes::c_LogMVAResponse) {
          logLikelihoods[hypo_idx] = (std::isnormal(logLikelihoods[hypo_idx])
                                      && logLikelihoods[hypo_idx] > 0) ? std::log(logLikelihoods[hypo_idx]) : c_dummyLogL;
        }
        logLikelihoods[hypo_idx] = logLikelihoods[hypo_idx] / phasespaceCategory->getTemperature();
        continue;
      }
      B2DEBUG(22, "MVA Index for hypo " << absPdgId << " : " << phasespaceCategory->getMVAIndexForHypothesis(absPdgId));

      for (unsigned int iResponse = 0; iResponse < transformed_scores.size(); iResponse++) {
        if ((transformMode == transformModes::c_LogTransformSingle) and
            (phasespaceCategory->getMVAIndexForHypothesis(absPdgId) != iResponse)) {continue;}

        double xmin, xmax;
        phasespaceCategory->getPDF(iResponse, absPdgId)->GetRange(xmin, xmax);
        // Kick the response back into the range of the PDF (alternatively consider logL = std::numeric_limits<float>::min() if outside range)
        float transformed_score_copy = transformed_scores[iResponse];
        if (transformed_score_copy < xmin) transformed_score_copy = xmin + 1e-5;
        if (transformed_score_copy > xmax) transformed_score_copy = xmax - 1e-5;

        float pdfval = phasespaceCategory->getPDF(iResponse, absPdgId)->Eval(transformed_score_copy);
        // Don't take a log of inf or 0
        logL += (std::isnormal(pdfval) && pdfval > 0) ? std::log(pdfval) : c_dummyLogL;
        B2DEBUG(22, "MVA response index, MVA score, logL: " << iResponse << "  " << transformed_score_copy << "  " << logL << " \n");
      }
      logLikelihoods[hypo_idx] = logL;
    } // hypo loop

    B2DEBUG(22, "logL: ");
    for (unsigned int iLogL = 0; iLogL < Const::ChargedStable::c_SetSize; iLogL++) {
      B2DEBUG(22, "\t\t " << iLogL << " : " << logLikelihoods[iLogL]);
    }

    const auto eclPidLikelihood = m_eclPidLikelihoods.appendNew(logLikelihoods);

    track.addRelationTo(eclPidLikelihood);
  } // tracks
}

float ECLChargedPIDMVAModule::logTransformation(const float value, const float offset, const float max) const
{
  return std::log((value + offset) / (max + offset - value));
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

float ECLChargedPIDMVAModule::evaluateVariable(const Variable::Manager::Var* varobj, const Particle* particle)
{
  float val = std::numeric_limits<float>::quiet_NaN();
  if (std::holds_alternative<double>(varobj->function(particle))) {
    val = (float)std::get<double>(varobj->function(particle));
  } else if (std::holds_alternative<int>(varobj->function(particle))) {
    val = (float)std::get<int>(varobj->function(particle));
  } else if (std::holds_alternative<bool>(varobj->function(particle))) {
    val = (float)std::get<bool>(varobj->function(particle));
  } else {
    B2ERROR("Variable '" << varobj->name << "' has wrong data type! It must be one of double, integer, or bool.");
  }
  if (std::isnan(val)) {
    //NaNs cause crashed if using TMVA MulticlassBDT.
    //In case other MVA methods are used, NaNs likely should be masked properly also.
    B2FATAL("Variable '" << varobj->name <<
            "' has NaN entries. Variable definitions in the MVA payload should include 'ifNaNGiveX(X, DEFAULT)'" <<
            "with a proper default value if it is possible for the variable to evaluate as NaN.");
  }

  return val;
}
