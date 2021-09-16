/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marcel Hohmann (marcel.hohmann@unimelb.edu.au)           *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <math.h>
#include <algorithm>
#include "TMath.h"

#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <framework/geometry/B2Vector3.h>
#include <mva/interface/Interface.h>
#include <mva/methods/TMVA.h>
#include <iostream>
#include <limits>


#include <ecl/modules/eclChargedPIDMVA/ECLChargedPIDMVAModule.h>

using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLChargedPIDMVA)

ECLChargedPIDMVAModule::ECLChargedPIDMVAModule() : Module(), m_variables(194, -999)
{
  setDescription("The module implements charged particle identification using ECL-related observables via a multiclass BDT. For each track matched with a suitable ECLShower, the relevant ECL variables (shower shape, PSD etc.) are fed to the BDT which is stored in a conditions database payload. The BDT output variables are then used to construct a liklihood from pdfs also stored in the payload. The liklihood is then stored in the ECLPidLikelihood object.");

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
  B2INFO("Run: " << m_eventMetaData->getRun() <<
         ". Load supported MVA interfaces for multi-class charged particle identification...");

  MVA::AbstractInterface::initSupportedInterfaces();
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();

  B2INFO("\tLoading weightfiles from the payload class.");

  auto serialized_weightfiles = (*m_mvaWeights.get())->getMVAWeightStrings();
  auto nfiles = serialized_weightfiles->size();

  B2INFO(" number of weightfiles found: " << nfiles);
  m_experts.resize(nfiles);
  B2INFO(" Rezised Vector");
  for (unsigned int idx(0); idx < nfiles; idx++) {

    B2INFO("\t\tweightfile[" << idx << "]");
    B2DEBUG(12, "\t\tweightfile[" << idx << "]");

    // De-serialize the string into an MVA::Weightfile object.
    std::stringstream ss(serialized_weightfiles->at(idx));
    auto weightfile = MVA::Weightfile::loadFromStream(ss);

    MVA::GeneralOptions general_options;
    weightfile.getOptions(general_options);

    // Store an MVA::Expert object.
    m_experts[idx] = supported_interfaces[general_options.m_method]->getExpert();
    m_experts.at(idx)->load(weightfile);

    B2INFO("\t\tweightfile  at " << idx << " successfully initialised.");
    B2DEBUG(12, "\t\tweightfile  at " << idx << " successfully initialised.");


    if (idx == 0) {
      // TODO - should this be moved to one dataset per region?
      // They all use the same variables and options
      m_dataset = std::make_unique<MVA::SingleDataset>(general_options, m_variables, 1.0, m_spectators);
    }
  }

  m_log_transform_offset = (*m_mvaWeights.get())->getLogTransformOffset();
}


void ECLChargedPIDMVAModule::beginRun()
{
  //(*m_mvaWeights.get()).addCallback([this]() { checkDBPayloads();});
  checkDBPayloads();
  //(*m_mvaWeights.get()).addCallback([this]() { initializeMVA(); });
  initializeMVA();
}

void ECLChargedPIDMVAModule::event()
{
  for (const auto& track : m_tracks) {

    // load the pion fit hypothesis or closest in mass.
    const TrackFitResult* fitRes = track.getTrackFitResultWithClosestMass(Const::pion);
    if (fitRes == nullptr) continue;
    const auto relShowers = track.getRelationsTo<ECLShower>();
    if (relShowers.size() == 0) continue;

    const double p     = fitRes->getMomentum().Mag();
//     const double theta = fitRes->getMomentum().Theta();
    const auto charge  = fitRes->getChargeSign();

    // get and fill the variables
    double shEnergy(0.0), maxEnergy(0.0);

    const ECLShower* mostEnergeticShower = nullptr;

    for (const auto& eclShower : relShowers) {

      if (eclShower.getHypothesisId() != ECLShower::c_nPhotons) continue;

      shEnergy = eclShower.getEnergy();
      if (shEnergy > maxEnergy) {
        maxEnergy = shEnergy;
        mostEnergeticShower = &eclShower;
      }

    }
    // Need at least 1 shower with a photon hypothesis.
    if (!mostEnergeticShower) continue;

    double showerTheta = mostEnergeticShower->getTheta();

    //check that we cover this region with our weightfiles.
    if (!((*m_mvaWeights.get())->isPhasespaceCovered(showerTheta, p, charge))) continue;


    // basic variables
    m_dataset->m_input[0] = mostEnergeticShower->getE1oE9();
    m_dataset->m_input[1] = mostEnergeticShower->getE9oE21();
    m_dataset->m_input[2] = maxEnergy;
    m_dataset->m_input[3] = maxEnergy / p;
    m_dataset->m_input[4] = mostEnergeticShower->getTrkDepth();
    m_dataset->m_input[5] = mostEnergeticShower->getLateralEnergy();


    //Zernike moments
    int offset = 6;
    for (unsigned int n = 0; n <= 6; n++) {
      for (unsigned int m = 0; m <= n; m++) {
        m_dataset->m_input[(n * (n + 1)) / 2 + m + offset] =  mostEnergeticShower->getAbsZernikeMoment(n, m);
      }
    }

    // PSD information
    offset += 28;

    ECLGeometryPar* geometry = ECLGeometryPar::Instance();

    std::vector<std::tuple<double, unsigned int>> EnergyToSort;

    RelationVector<ECLCalDigit> relatedDigits = mostEnergeticShower->getRelationsTo<ECLCalDigit>();

    //EnergyToSort vector is used for sorting digits by offline two component energy
    for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {

      const auto caldigit = relatedDigits.object(iRel);

      //exclude digits without waveforms
      const double digitChi2 = caldigit->getTwoComponentChi2();
      if (digitChi2 < 0)  continue;

      ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();

      //exclude digits digits with poor chi2
      if (digitFitType1 == ECLDsp::poorChi2) continue;

      //exclude digits with diode-crossing fits
      if (digitFitType1 == ECLDsp::photonDiodeCrossing) continue;

      EnergyToSort.emplace_back(caldigit->getTwoComponentTotalEnergy(), iRel);
    }

    //sorting by energy
    std::sort(EnergyToSort.begin(), EnergyToSort.end(), std::greater<>());

    //get cluster position information
    const double showerR = mostEnergeticShower->getR();
    //const double showerTheta = cluster->getTheta();
    const double showerPhi = mostEnergeticShower->getPhi();

    B2Vector3D showerPosition;
    showerPosition.SetMagThetaPhi(showerR, showerTheta, showerPhi);


    for (unsigned int digit = 0; digit < 20; ++digit) {
      if (digit < EnergyToSort.size()) {
        const auto [digitEnergy, next] = EnergyToSort[digit];
        const auto caldigit = relatedDigits.object(next);
        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();
        const int cellId = caldigit->getCellId();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
        TVector3 tempP = showerPosition - calDigitPosition;

        m_dataset->m_input[offset]      = caldigit->getTwoComponentHadronEnergy();
        m_dataset->m_input[offset + 1]  = caldigit->getEnergy();
        m_dataset->m_input[offset + 2]  = m_dataset->m_input[offset] / digitEnergy;
        m_dataset->m_input[offset + 3]  = relatedDigits.weight(next);
        m_dataset->m_input[offset + 4]  = digitFitType1;
        m_dataset->m_input[offset + 5]  = tempP.Mag();
        m_dataset->m_input[offset + 6]  = tempP.CosTheta();
        m_dataset->m_input[offset + 7]  = tempP.Phi();
      } else {
        m_dataset->m_input[offset]      = 0.0;
        m_dataset->m_input[offset + 1]  = 0.0;
        m_dataset->m_input[offset + 2]  = 0.0;
        m_dataset->m_input[offset + 3]  = 0.0;
        m_dataset->m_input[offset + 4]  = -1.0;
        m_dataset->m_input[offset + 5]  = 0.0;
        m_dataset->m_input[offset + 6]  = 0.0;
        m_dataset->m_input[offset + 7]  = 0.0;
      }
      offset += 8;
    }

    //get the MVA response values
    unsigned int linearBinIndex = (*m_mvaWeights.get())->getLinearisedBinIndex(showerTheta, p, charge);
    // We deal w/ a SingleDataset, so 0 is the only existing component by construction.
    std::vector<float> scores = m_experts.at(linearBinIndex)->applyMulticlass(*m_dataset)[0];
    B2INFO("Scores:");
    for (auto score : scores) {
      B2INFO(score);
    }
    // log transform the scores
    for (unsigned int iResponse = 0; iResponse < scores.size(); iResponse++) {
      scores[iResponse] = logTransformation(scores[iResponse]);
    }
    B2INFO("Log Transformed Scores:");
    for (auto score : scores) {
      B2INFO(score);
    }

    float logLikelihoods[Const::ChargedStable::c_SetSize];

    // Order of loop is defined in UnitConst.cc: e, mu, pi, K, p, d
    for (const auto& hypo : Const::chargedStableSet) {
      unsigned int hypo_idx = hypo.getIndex();
      auto absPdgId = abs(hypo.getPDGCode());

      // TODO - avoid this duplication if no further transformations are booked,
      std::vector<float> transformed_scores = scores;

      // perform extra transformations if they are booked
      if ((*(*m_mvaWeights.get())->getTransformMode(linearBinIndex) ==
           ECLChargedPIDMVAWeights::BDTResponseTransformMode::c_GaussianTransform)
          or
          (*(*m_mvaWeights.get())->getTransformMode(linearBinIndex) ==
           ECLChargedPIDMVAWeights::BDTResponseTransformMode::c_DecorrelationTransform)) {

        // gaussian transform
        for (unsigned int iResponse = 0; iResponse < scores.size(); iResponse++) {
          transformed_scores[iResponse] = gaussTransformation(scores[iResponse], (*m_mvaWeights.get())->getCDF(absPdgId,  iResponse,
                                                              linearBinIndex));
        }
        if (*(*m_mvaWeights.get())->getTransformMode(linearBinIndex) ==
            ECLChargedPIDMVAWeights::BDTResponseTransformMode::c_DecorrelationTransform) {
          transformed_scores = decorrTransformation(transformed_scores, (*m_mvaWeights.get())->getDecorrelationMatrix(absPdgId,
                                                    linearBinIndex));
        }

      }
      B2INFO("Further Transformed Scores:");
      for (auto score : scores) {
        B2INFO(score);
      }


      // get the pdf values for each response value
      float logL = 0.0;
      B2INFO("LogL");
      for (unsigned int iResponse = 0; iResponse < transformed_scores.size(); iResponse++) {
        double xmin, xmax;
        (*m_mvaWeights.get())->getPDF(absPdgId, iResponse, linearBinIndex)->GetRange(xmin, xmax);
        // kick the response back into the range of the PDF (alternatively consider logL = std::numeric_limits<float>::min() if outside range)
        if (transformed_scores[iResponse] < xmin) transformed_scores[iResponse] = xmin + 1e-5;
        if (transformed_scores[iResponse] > xmax) transformed_scores[iResponse] = xmax - 1e-5;
        float pdfval = (*m_mvaWeights.get())->getPDF(absPdgId, iResponse, linearBinIndex)->Eval(transformed_scores[iResponse]);
        // dont take a log of inf or 0
        B2INFO("Raw pdfval: " << pdfval << " Max, Response, Min: " << xmin << ", " << transformed_scores[iResponse] << ", " << xmax);
        pdfval = std::max(pdfval , std::numeric_limits<float>::min());
        logL += (std::isnormal(pdfval) && pdfval > 0) ? std::log(pdfval) : c_dummyLogL;
        B2INFO(logL << "  " << pdfval << "  " << absPdgId << "  " << iResponse);
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


